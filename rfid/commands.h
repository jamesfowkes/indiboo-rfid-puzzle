#ifndef _COMMANDS_H_
#define _COMMANDS_H_

typedef enum
{
	eCommand_Reset,
	eCommand_WipeMemory,
	eCommand_StoreMemory,
	eCommand_SetWonGameState,
} eCommand;

void app_handle_command(eCommand command);

void command_setup();
void command_tick();
bool command_is_idle();

#endif
