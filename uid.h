#ifndef _UID_H_
#define _UID_H_

typedef struct {
	uint8_t size;
	uint8_t bytes[10];
} UID;

bool uid_is_valid(const UID& uid);
void uid_copy(UID& dst, const UID& src);
void uid_clear(UID& to_clear);
bool uid_match(const UID& uid1, const UID& uid2);

#endif