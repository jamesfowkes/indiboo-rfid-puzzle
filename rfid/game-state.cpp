/* C/C++/Arduino Includes */
#include <Arduino.h>
#include <stdint.h>

/* Application Includes */

#include "game-state.h"

static const uint8_t STATE_PINS[] = {A5, 10};
static eGameState s_eOldState = -1;

void game_state_setup()
{
	pinMode(STATE_PINS[0], OUTPUT);
	pinMode(STATE_PINS[1], OUTPUT);
}

void game_state_set(eGameState state)
{
	if (s_eOldState != state)
	{
		Serial.print("Setting game state ");
		Serial.println(state);
		s_eOldState = state;
	}

	digitalWrite(STATE_PINS[0], state & 0x02 ? HIGH : LOW);
	digitalWrite(STATE_PINS[1], state & 0x01 ? HIGH : LOW);
}
