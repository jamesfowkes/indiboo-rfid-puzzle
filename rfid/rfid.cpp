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
    2, 3, 4, 5, 6, 7
};

static uint8_t RST_PIN = 9;

static bool * sp_rfid_update_flag;
static MFRC522 s_rfids[RFID_SLOT_COUNT] = {
    MFRC522(SS_PINS[0], RST_PIN),
    MFRC522(SS_PINS[1], RST_PIN),
    MFRC522(SS_PINS[2], RST_PIN),
    MFRC522(SS_PINS[3], RST_PIN),
    MFRC522(SS_PINS[4], RST_PIN),
    MFRC522(SS_PINS[5], RST_PIN)
};

static UID s_current_uids[RFID_SLOT_COUNT];

static uint8_t s_card_not_present_count[RFID_SLOT_COUNT];

static void get_uid(MFRC522& mfrc522, UID &uid, uint8_t& count, uint8_t slot)
{
    (void)slot;
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
    {
        count = 0;
        memcpy(uid.bytes, mfrc522.uid.uidByte, mfrc522.uid.size);
        uid.size = mfrc522.uid.size;
    }
    else
    {
        if (count >= 2)
        {
            uid_clear(uid);
        }
        else
        {
            count++;
        }
    }
}

static void get_uids(MFRC522 (&mfrc522)[RFID_SLOT_COUNT], UID (&uid)[RFID_SLOT_COUNT])
{
    for (uint8_t slot=0; slot<RFID_SLOT_COUNT; slot++)
    {
        get_uid(mfrc522[slot], uid[slot], s_card_not_present_count[slot], slot);
    }
}

static void rfid_task_fn(TaskAction* this_task)
{
    (void)this_task;
    get_uids(s_rfids, s_current_uids);
    *sp_rfid_update_flag = true;
}
static TaskAction s_rfid_task(rfid_task_fn, 100, INFINITE_TICKS);

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
        Serial.print("None");
    }
}

static void print_uids(UID * pUIDs, const uint8_t nUIDs)
{
    for (uint8_t i=0; i<nUIDs; i++)
    {
        if (uid_is_valid(pUIDs[i]))
        {
            print_uid(pUIDs[i]);
            Serial.print(", ");
        }
        else
        {
            Serial.print("None, ");
        }
    }
}

/*
 * Public Functions
 */

void rfid_setup(bool& rfid_update_flag)
{
    uint8_t slot;

    sp_rfid_update_flag = &rfid_update_flag;

    SPI.begin();

    rfid_nv_setup();

    for (slot=0;slot<RFID_SLOT_COUNT;slot++)
    {
        s_rfids[slot].PCD_Init();
        pinMode(SS_PINS[slot], OUTPUT);
        digitalWrite(SS_PINS[slot], HIGH);

        Serial.print("Checking for reader in slot ");
        Serial.print(slot);
        Serial.print("...");
        s_rfids[slot].PCD_DumpVersionToSerial();
    }
}

void rfid_tick()
{
    s_rfid_task.tick();
}

uint8_t rfid_get_matched(bool (&match)[RFID_SLOT_COUNT])
{
    uint8_t count = 0;
    for (uint8_t slot=0; slot<RFID_SLOT_COUNT; slot++)
    {
        if (uid_is_valid(s_current_uids[slot]))
        {
            match[slot] = rfid_store_match_in_slot(s_current_uids[slot], slot) != NO_MATCH;
            if (match[slot])
            {
                count++;
            }
        }
        else
        {
            match[slot] = false;
        }
    }
    return count;
}

void rfid_print_current_uids()
{
    for (uint8_t slot=0; slot<RFID_SLOT_COUNT; slot++)
    {
        Serial.print("Slot ");
        Serial.print(slot);
        Serial.print(" ");
        print_uid(s_current_uids[slot]);
        Serial.println("");
    }
}

void rfid_debug()
{   
    uint8_t match = NO_MATCH;
    UID uid;
    for (uint8_t slot=0; slot<RFID_SLOT_COUNT; slot++)
    {
        Serial.print("Slot ");
        Serial.print(slot);
        Serial.print(": ");
        print_uid(s_current_uids[slot]);
        match = rfid_store_match_in_slot(s_current_uids[slot], slot);
        if (match != NO_MATCH)
        {
            Serial.print(" (match ");
            Serial.print(match);
            Serial.print(")");
        }
        else
        {
            Serial.print(" (no match on ");
            for (uint8_t idx=0;idx<3;idx++)
            {
                rfid_store_get_uid(uid, slot, idx);
                print_uid(uid);
                Serial.print(",");
            }
            Serial.print(")");
        }
        Serial.println("");
    }
}

void rfid_print_current_uid(uint8_t slot)
{
    print_uid(s_current_uids[slot]);
}

void rfid_print_saved_uid(uint8_t slot)
{
    UID to_print[RFID_HISTORY_COUNT];
    rfid_store_get_uids(to_print, slot);
    print_uids(to_print, RFID_HISTORY_COUNT);
}

void rfid_save_all_current_uids()
{
    get_uids(s_rfids, s_current_uids);
    
    for (uint8_t slot=0; slot<RFID_SLOT_COUNT; slot++)
    {
        Serial.print("Slot ");
        Serial.print(slot);
        Serial.println(":");

        Serial.print("Saved UIDs: ");
        rfid_print_saved_uid(slot);
        Serial.println();

        if (uid_is_valid(s_current_uids[slot]))
        {
            if (!rfid_store_match_in_slot(s_current_uids[slot], slot) != NO_MATCH)
            {
                Serial.print("Saving new UID ");
                print_uid(s_current_uids[slot]);
                Serial.print(" to slot ");
                Serial.print(slot);
                Serial.print(" (index ");
                Serial.print((int)rfid_store_save_uid(s_current_uids[slot], slot));
                Serial.println(")");
            }
            else
            {
                Serial.print("UID ");
                print_uid(s_current_uids[slot]);
                Serial.print(" already in slot ");
                Serial.println(slot);
                   
            }
        }
        else
        {
            Serial.print("No/invalid card in slot ");
            Serial.println(slot);
        }
    }
}

void rfid_save_current_uid(uint8_t slot)
{
    rfid_store_save_uid(s_current_uids[slot], slot);
}
