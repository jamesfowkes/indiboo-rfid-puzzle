/* C/C++/Arduino Includes */
#include <Arduino.h>
#include <stdint.h>

/* Application Includes */

#include "game-state.h"

static const uint8_t STATE_PINS[] = {A5, A6};

void game_state_setup()
{
	pinMode(STATE_PINS[0], OUTPUT);
	pinMode(STATE_PINS[1], OUTPUT);
}

void game_state_set(eGameState state)
{
	digitalWrite(STATE_PINS[0], state & 0x02);
	digitalWrite(STATE_PINS[1], state & 0x01);
}
