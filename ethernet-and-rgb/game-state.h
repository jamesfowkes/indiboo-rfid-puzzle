#ifndef _GAME_STATE_H_
#define _GAME_STATE_H_

typedef enum
{
	eGameState_InProgress,
	eGameState_IncorrectCombination,
	eGameState_CorrectCombination,
} eGameState;

void game_state_setup();
void game_state_tick();
eGameState game_state_get();

#endif

