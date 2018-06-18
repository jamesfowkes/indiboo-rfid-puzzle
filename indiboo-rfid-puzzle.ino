#include <util/atomic.h>
#include <TaskAction.h>
#include <Adafruit_NeoPixel.h>

#include "uid.h"
#include "rfid.h"

#define RFID_NV_ACCESS_DEBUG
#include "rfid-nv.h"

static bool s_rfid_update_flag = false;

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

static void finish_game()
{

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

void setup()
{
	Serial.begin(115200);
	Serial.println("RFID Puzzle");

	s_pixels.begin();
	s_pixels.show();

	rfid_setup(s_rfid_update_flag);

	rfid_nv_dump_all_data(Serial);

	rfid_save_all_current_uids();

	Serial.println("Starting puzzle...");
}

void loop()
{
	bool matched[RFID_SLOT_COUNT];

	uint8_t match_count;
	rfid_tick();
	s_debug.tick();
	if (check_and_clear(s_rfid_update_flag))
	{
		match_count = rfid_get_matched(matched);
		
		show_pixels(s_pixels, matched);

		if (match_count == RFID_SLOT_COUNT)
		{
			finish_game();
		}
	}
}
