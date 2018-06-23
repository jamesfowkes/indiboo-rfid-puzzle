/* Arduino Library Includes */

#include "TaskAction.h"

/* Application Includes */

#include "game-state.h"

static const uint8_t STATE_PINS[] = {A5, A6};

/* Private Variables */
static eGameState s_state = eGameState_InProgress;

static eGameState game_state_read()
{
	return (eGameState)(
		(digitalRead(STATE_PINS[0]) ? 0x02 : 0x00) + 
		(digitalRead(STATE_PINS[1]) ? 0x01 : 0x00)
	);
}

static void set_game_state(eGameState new_state)
{
	if (s_state != new_state)
	{
		Serial.print("New state: ");
		Serial.println(new_state);
		s_state = new_state;
	}
}
static void game_state_task_fn(TaskAction* this_task)
{
	(void)this_task;

    static uint8_t read_idx = 0;
	static eGameState state_debouncers[2];

	state_debouncers[read_idx] = game_state_read();
	read_idx = 1-read_idx;

	if (state_debouncers[0] == state_debouncers[1])
	{
		set_game_state(state_debouncers[0]);
	}
}
static TaskAction s_game_state_task(game_state_task_fn, 10, INFINITE_TICKS);

void game_state_setup()
{
	pinMode(STATE_PINS[0], INPUT);
	pinMode(STATE_PINS[1], INPUT);
}

void game_state_tick()
{
	s_game_state_task.tick();
}

eGameState game_state_get()
{
	return s_state;
}