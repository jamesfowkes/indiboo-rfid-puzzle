#include <Arduino.h>

#include <stdint.h>

#include "commands.h"

typedef enum
{
	eCommandState_Idle,
	eCommandState_WaitForAck	
} eCommandState;

static const uint8_t COMMAND_PINS[] = {A0, A1, A2};
static const uint8_t COMMAND_READY_PIN = A3;
static const uint8_t ACK_PIN = A4;

static eCommandState eState = eCommandState_Idle;

#include <TaskAction.h>

static void assert_command_ready(bool assert)
{
	digitalWrite(ACK_PIN, assert ? LOW : HIGH);
}

static eCommand read_command()
{
	return (eCommand)(
		(digitalRead(COMMAND_PINS[0]) == HIGH ? 0x04 : 0x00) |
		(digitalRead(COMMAND_PINS[1]) == HIGH ? 0x02 : 0x00) |
		(digitalRead(COMMAND_PINS[2]) == HIGH ? 0x01 : 0x00)
	);
}

static eCommandState handle_idle_state()
{
	eCommandState new_state = eCommandState_Idle;
	eCommand command;
	if (digitalRead(COMMAND_READY_PIN) == LOW)
	{
		command = read_command();
		app_handle_command(command);
		new_state = eCommandState_Ack;
		assert_command_ready(true);
	}
	return new_state;
}

static eCommandState handle_ack_state()
{
	eCommandState new_state = eCommandState_Ack;
	if (digitalRead(COMMAND_READY_PIN) == HIGH)
	{
		assert_command_ready(false);
		new_state = eCommandState_Idle;
	}
	return new_state;
}

static void command_task_fn(TaskAction* this_task)
{
	(void)this_task;
    if (eState == eCommandState_WaitForAck)
    {
    	if (digitalRead(ACK_PIN) == LOW)
    	{
			eState = eCommandState_Idle;
		}
    }
}
static TaskAction s_command_task(command_task_fn, 50, INFINITE_TICKS);

void command_setup()
{
	for(uint8_t i=0; i<sizeof(COMMAND_PINS); i++)
	{
		pinMode(COMMAND_PINS[i], OUTPUT);
	}
	pinMode(COMMAND_READY_PIN, OUTPUT);
	pinMode(ACK_PIN, INPUT_PULLUP);

	assert_command_ready(false);
}

void command_send(eCommand command);
{
	if (eState == eCommandState_Idle)
	{
		put_command(command);
		eState = eCommandState_WaitForAck;
	}
}

void command_tick()
{
	s_command_task.tick();
}