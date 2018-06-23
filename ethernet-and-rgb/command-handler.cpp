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
	digitalWrite(COMMAND_READY_PIN, assert ? LOW : HIGH);
}

static void put_command(eCommand command)
{
	digitalWrite(COMMAND_PINS[0], ((uint8_t)command & 0x04) ? HIGH : LOW);
	digitalWrite(COMMAND_PINS[1], ((uint8_t)command & 0x02) ? HIGH : LOW);
	digitalWrite(COMMAND_PINS[2], ((uint8_t)command & 0x01) ? HIGH : LOW);
	assert_command_ready(true);
}

static void command_task_fn(TaskAction* this_task)
{
	(void)this_task;
    if (eState == eCommandState_WaitForAck)
    {
    	if (digitalRead(ACK_PIN) == LOW)
    	{
    		assert_command_ready(false);
    		Serial.println(F("Entering idle state"));
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

void command_send(eCommand command)
{
	if (eState == eCommandState_Idle)
	{
		put_command(command);
		eState = eCommandState_WaitForAck;
	}
	else
	{
		Serial.print(F("Not in idle, cannot handle "));
		Serial.println(command);
	}
}

void command_tick()
{
	s_command_task.tick();
}
