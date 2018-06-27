#include <util/atomic.h>
#include <avr/wdt.h>

#include <TaskAction.h>
#include <Adafruit_NeoPixel.h>

#include "uid.h"
#include "rfid.h"
#include "rfid-store.h"
#include "commands.h"
#include "game-state.h"

#define RFID_NV_ACCESS_DEBUG
#include "rfid-nv.h"

static bool s_rfid_update_flag = false;
static bool s_command_flags[4] = {false, false, false, false};
static bool s_state_override = false;

static Adafruit_NeoPixel s_pixels = Adafruit_NeoPixel(RFID_SLOT_COUNT, 8, NEO_GRB + NEO_KHZ800);

static bool check_and_clear(bool &flag)
{
	bool value;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		value = flag;
		flag = false;
	}
	return value;
}

static void show_pixels(Adafruit_NeoPixel& pixels, bool (&show)[RFID_SLOT_COUNT])
{
	pixels.clear();
	for(uint8_t i = 0; i < RFID_SLOT_COUNT; i++)
	{
		if (show[i])
		{
			pixels.setPixelColor(i, 0, 32,0);
		}
	}
	pixels.show();
}

static void debug_fn(TaskAction* this_task)
{
	(void)this_task;
    rfid_debug();
}
static TaskAction s_debug(debug_fn, 1000, INFINITE_TICKS);

static void update_game_state_on_rfid()
{
	bool matched[RFID_SLOT_COUNT];

	uint8_t match_count = rfid_get_matched(matched);
	uint8_t tag_count= rfid_get_uid_count();

	show_pixels(s_pixels, matched);

	if (!s_state_override)
	{
		if (tag_count == RFID_SLOT_COUNT)
		{
			if (match_count == RFID_SLOT_COUNT)
			{
				game_state_set(eGameState_CorrectCombination);
			}
			else
			{
				game_state_set(eGameState_IncorrectCombination);
			}
		}
		else
		{
			game_state_set(eGameState_InProgress);
		}
	}
}

void app_handle_command(eCommand command)
{
	Serial.print("Got command ");
	Serial.println((int)command);
	
	s_command_flags[command] = true;
}

void setup()
{
	wdt_disable();

	Serial.begin(115200);
	Serial.println("RFID Puzzle");

	s_pixels.begin();
	s_pixels.show();

	command_setup();
	game_state_setup();

	rfid_setup(s_rfid_update_flag);

	rfid_nv_dump_all_data(Serial);

	rfid_save_all_current_uids();

	wdt_enable(WDTO_2S);

	Serial.println("Starting puzzle...");
}

void loop()
{
	wdt_reset();

	rfid_tick();
	command_tick();
	s_debug.tick();

	if (check_and_clear(s_rfid_update_flag))
	{
		update_game_state_on_rfid();
	}

	if (command_is_idle() && check_and_clear(s_command_flags[eCommand_Reset]))
	{
		Serial.println("Resetting via WDT...");
		while(true) {}
	}

	if (check_and_clear(s_command_flags[eCommand_WipeMemory]))
	{
		Serial.println("Clearing EEPROM");
		rfid_store_clear_all();
	}

	if (check_and_clear(s_command_flags[eCommand_StoreMemory]))
	{
		Serial.println("Saving current tags to EEPROM");
		rfid_save_all_current_uids();
	}

	if (check_and_clear(s_command_flags[eCommand_SetWonGameState]))
	{
		Serial.println("Overrding game state to win");
		s_state_override = true;
		game_state_set(eGameState_CorrectCombination);
	}
}

static void handle_serial_cmd(char const * const cmd)
{
	if (strcmp(cmd, "RESET") == 0)
	{
		s_command_flags[eCommand_Reset] = true;
	}
	else if (strcmp(cmd, "WIPE") == 0)
	{
		s_command_flags[eCommand_WipeMemory] = true;
	}
	else if (strcmp(cmd, "SAVE") == 0)
	{
		s_command_flags[eCommand_StoreMemory] = true;
	}
	else if (strcmp(cmd, "WIN") == 0)
	{
		s_command_flags[eCommand_SetWonGameState] = true;
	}
	else
	{
		Serial.println("Command unknown");
	}
}

static char s_serial_buffer[16];
static uint8_t s_bufidx = 0;

void serialEvent()
{
	while (Serial.available())
	{
		char c  = Serial.read();
		if (c == '\n')
		{
			handle_serial_cmd(s_serial_buffer);
			s_bufidx = 0;
			s_serial_buffer[0] = '\0';
		}
		else
		{
			s_serial_buffer[s_bufidx++] = c;
			s_serial_buffer[s_bufidx] = '\0';
		}
	}
}
