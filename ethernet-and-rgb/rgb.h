#ifndef _RGB_H_
#define _RGB_H_

typedef enum
{
	eRGBSetting_Correct,
	eRGBSetting_Incorrect
} eRGBSetting;

void rgb_setup();
void rgb_intro();
void rgb_tick();

void rgb_save(uint8_t(&rgb)[3], eRGBSetting setting);

void rgb_set(eRGBSetting setting);
void rgb_set(uint8_t r, uint8_t g, uint8_t b);
void rgb_set(uint8_t(&rgb)[3]);

bool rgb_parse_hex(char const * const pRGB, uint8_t(&rgb)[3]);

void rgb_print(uint8_t r, uint8_t g, uint8_t b, bool nl);
void rgb_print(uint8_t(&rgb)[3], bool nl);

void rgb_flash(uint8_t(&rgb)[3], uint16_t ms_count);

#endif
