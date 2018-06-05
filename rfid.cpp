/* Arduino Library Includes */

#include "TaskAction.h"
#include "MFRC522.h"
#include "EEPROMex.h"

/* Application Includes */

#include "uid.h"
#include "rfid.h"
#include "rfid-store.h"
#include "rfid-nv.h"

/* Private Variables */

static const uint8_t SS_PINS[RFID_SLOT_COUNT] = {
	3, 4, 5, 6, 7, 8, 9, 10
};

static uint8_t SS_PIN = 10;
static uint8_t RST_PIN = 9;

static bool * sp_rfid_update_flag;
static MFRC522 s_rfid = MFRC522(SS_PIN, RST_PIN);

static UID s_current_uids[RFID_SLOT_COUNT];

static bool get_uid(MFRC522& mfrc522, UID &uid)
{
	if (!mfrc522.PICC_IsNewCardPresent()) { return false; }
	if (!mfrc522.PICC_ReadCardSerial()) { return false; }
	if (mfrc522.uid.size == 0) { return false; }

	memcpy(uid.bytes, mfrc522.uid.uidByte, mfrc522.uid.size);
	uid.size = mfrc522.uid.size;

	return true;
}

static bool get_uids(MFRC522 &mfrc522, UID (&uid)[RFID_SLOT_COUNT])
{
	for (uint8_t i=0; i<RFID_SLOT_COUNT; i++)
	{
		mfrc522.SetCSPin(i);
		get_uid(mfrc522, uid[i]);
	}
}

static void rfid_task_fn(TaskAction* this_task)
{
	(void)this_task;
	*sp_rfid_update_flag = get_uids(s_rfid, s_current_uids);
}
static TaskAction s_rfid_task(rfid_task_fn, 50, INFINITE_TICKS);

static void print_uid(UID& uid)
{
	if (uid_is_valid(uid))
	{
		for(uint8_t i=0; i<uid.size; i++)
		{
			Serial.print(uid.bytes[i], 16);
		}
	}
	else
	{
		Serial.print(" Invalid UID? Size=");
		Serial.print(uid.size);
	}
}

static void print_uids(UID * pUIDs, const uint8_t nUIDs)
{
	for (uint8_t i=0; i<nUIDs; i++)
	{
		print_uid(pUIDs[i]);
	}
}

/*
 * Public Functions
 */

void rfid_setup(bool& rfid_update_flag)
{
	sp_rfid_update_flag = &rfid_update_flag;

	SPI.begin();
	s_rfid.PCD_Init();

	rfid_nv_setup();
}

void rfid_tick()
{
	s_rfid_task.tick();
}

void rfid_print_current_uid(uint8_t index)
{
	print_uid(s_current_uids[index]);
}

void rfid_print_saved_uid(uint8_t index)
{
	UID to_print[RFID_HISTORY_COUNT];
	rfid_store_get_uids(to_print, index);
	print_uids(to_print, RFID_HISTORY_COUNT);
}

void rfid_save_all_current_uids()
{
	for (uint8_t i=0; i<RFID_SLOT_COUNT; i++)
	{
		rfid_save_current_uid(i);
	}
}

void rfid_save_current_uid(uint8_t slot)
{
	rfid_store_save_uid(s_current_uids[slot], slot);
}
