#include <stdint.h>

#include <Arduino.h>

#include "commands.h"

typedef enum
{
	eCommandState_Idle,
	eCommandState_Ack	
} eCommandState;

static const uint8_t COMMAND_PINS[] = {A0, A1, A2};
static const uint8_t COMMAND_READY_PIN = A3;
static const uint8_t ACK_PIN = A4;

static eCommandState s_eState = eCommandState_Idle;

#include <TaskAction.h>

static void assert_ack(bool assert)
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
		assert_ack(true);
	}
	return new_state;
}

static eCommandState handle_ack_state()
{
	eCommandState new_state = eCommandState_Ack;
	if (digitalRead(COMMAND_READY_PIN) == HIGH)
	{
		assert_ack(false);
		new_state = eCommandState_Idle;
	}
	return new_state;
}

static void command_task_fn(TaskAction* this_task)
{
	(void)this_task;
    switch(s_eState)
    {
	case eCommandState_Idle:
		s_eState = handle_idle_state();
		break;
	case eCommandState_Ack:
		s_eState = handle_ack_state();
		break;
    }
}
static TaskAction s_command_task(command_task_fn, 50, INFINITE_TICKS);

void command_setup()
{
	for(uint8_t i=0; i<sizeof(COMMAND_PINS); i++)
	{
		pinMode(COMMAND_PINS[i], INPUT_PULLUP);
	}
	pinMode(COMMAND_READY_PIN, INPUT_PULLUP);
	pinMode(ACK_PIN, OUTPUT);

	assert_ack(false);
}

void command_tick()
{
	s_command_task.tick();
}

bool command_is_idle()
{
	return (s_eState == eCommandState_Idle);
}