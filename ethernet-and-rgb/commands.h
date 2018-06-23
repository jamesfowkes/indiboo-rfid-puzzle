#ifndef _COMMANDS_H_
#define _COMMANDS_H_

typedef enum
{
	eCommand_Reset,
	eCommand_WipeMemory,
	eCommand_StoreMemory,
	eCommand_SetWonGameState,
} eCommand;

void command_setup();
void command_send(eCommand command);
void command_tick();

#endif
