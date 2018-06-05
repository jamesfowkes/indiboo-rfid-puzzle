/* C/C++ Includes */

#include <stdint.h>
#include <string.h>

/* Application Includes */

#include "uid.h"

/* Public Functions */

bool uid_is_valid(const UID& uid)
{
	return ((uid.size == 4) || (uid.size == 7) || (uid.size == 10));
}

void uid_copy(UID& dst, const UID& src)
{
	if (uid_is_valid(src))
	{
		dst.size = src.size;
		for (uint8_t i = 0; i<src.size; i++)
		{
			dst.bytes[i] = src.bytes[i];
		}
	}
}

void uid_clear(UID& to_clear)
{
	to_clear.size = 0;
	memset(&to_clear.bytes, 0, 10);
}

bool uid_match(const UID& uid1, const UID& uid2)
{
	if (uid1.size != uid2.size) { return false; }
	return memcmp(uid1.bytes, uid2.bytes, uid1.size) == 0;
}
