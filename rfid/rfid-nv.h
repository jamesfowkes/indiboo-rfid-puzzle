#ifndef _RFID_NV_H_
#define _RFID_NV_H_

void rfid_nv_setup();

void rfid_nv_set_uid(UID& uid, uint8_t direct_index);
void rfid_nv_get_uid(UID& uid, uint8_t direct_index);
void rfid_nv_clear_uid(uint8_t direct_index);

#ifdef RFID_NV_ACCESS_DEBUG
void rfid_nv_dump_all_data(Stream &s);
#endif

#endif
