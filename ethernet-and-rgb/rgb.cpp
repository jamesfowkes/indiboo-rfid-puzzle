#include <stdint.h>
#include <ctype.h>

#include <Arduino.h>
#include "EEPROMex.h"

#include "rgb.h"

static const uint8_t RGB_PINS[] = {5, 6, 9};
static unsigned long s_counter_expiry = 0;

static uint8_t s_rgb[3];
static int EEPROM_ADDRESS_RGB[2];

static void rgb_eeprom_setup()
{
    EEPROM_ADDRESS_RGB[0] = EEPROM.getAddress(sizeof(uint8_t) * 3);
    EEPROM_ADDRESS_RGB[1] = EEPROM.getAddress(sizeof(uint8_t) * 3);
}

static void rgb_save_eeprom(uint8_t(&rgb)[3], int address)
{
    EEPROM.updateByte(address, rgb[0]);
    EEPROM.updateByte(address+1, rgb[1]);
    EEPROM.updateByte(address+2, rgb[2]);
}

static void rgb_get_eeprom(uint8_t(&rgb)[3], int address)
{
    rgb[0] = EEPROM.readByte(address);
    rgb[1] = EEPROM.readByte(address+1);
    rgb[2] = EEPROM.readByte(address+2);
}

static uint8_t hex_to_u8(char hex)
{
	uint8_t val = 0;
	if ((hex >= '0') && (hex <= '9'))
	{
		val = hex - '0';
	}
	else if ((hex >= 'A') && (hex <= 'F'))
	{
		val = hex - 'A' + 10;
	}

	return val;
}

static uint8_t hex_to_u8(char const * const pHex)
{
	return (hex_to_u8(pHex[0]) << 4) + hex_to_u8(pHex[1]);
}

void rgb_setup()
{
	pinMode(RGB_PINS[0], OUTPUT);
	pinMode(RGB_PINS[1], OUTPUT);
	pinMode(RGB_PINS[2], OUTPUT);

	rgb_eeprom_setup();

	Serial.println(F("RGB Settings:"));
    rgb_get_eeprom(s_rgb, EEPROM_ADDRESS_RGB[0]);
    Serial.print("Correct: ");
    rgb_print(s_rgb, true);

    rgb_get_eeprom(s_rgb, EEPROM_ADDRESS_RGB[1]);
    Serial.print(F("Incorrect: "));
    rgb_print(s_rgb, true);
}

void rgb_intro()
{
	for(uint8_t pwm=0; pwm<50; pwm++)
	{
		rgb_set(pwm,0,0);
		delay(10);
	}

	for(uint8_t pwm=0; pwm<50; pwm++)
	{
		rgb_set(0,pwm, 0);
		delay(10);
	}

	for(uint8_t pwm=0; pwm<50; pwm++)
	{
		rgb_set(0,0,pwm);
		delay(10);
	}
	rgb_set(0,0,0);
}

void rgb_tick()
{
	if (s_counter_expiry)
	{
		if (millis() == s_counter_expiry)
		{
			s_counter_expiry = 0;
			rgb_set(0,0,0);
		}
	}
}

void rgb_set(uint8_t(&rgb)[3])
{
	rgb_set(rgb[0], rgb[1], rgb[2]);
}

void rgb_set(uint8_t r, uint8_t g, uint8_t b)
{
	analogWrite(RGB_PINS[0], r);
	analogWrite(RGB_PINS[1], g);
	analogWrite(RGB_PINS[2], b);
}

bool rgb_parse_hex(char const * const pRGB, uint8_t(&rgb)[3])
{
	bool valid_string = true;
	for (uint8_t i=0; i<6;i++)
	{
		valid_string &= (bool)isxdigit(pRGB[i]);
	}

	if (valid_string)
	{
		rgb[0] = hex_to_u8(&pRGB[0]);
		rgb[1] = hex_to_u8(&pRGB[2]);
		rgb[2] = hex_to_u8(&pRGB[4]);
	}

	return valid_string;
}

void rgb_print(uint8_t(&rgb)[3], bool nl)
{
	rgb_print(rgb[0], rgb[1], rgb[2], nl);
}

void rgb_print(uint8_t r, uint8_t g, uint8_t b, bool nl)
{
	Serial.print(r);Serial.print(", ");
	Serial.print(g);Serial.print(", ");
	Serial.print(b);
	if (nl)
	{
		Serial.println("");
	}
}

void rgb_flash(uint8_t(&rgb)[3], uint16_t ms_count)
{
	rgb_set(rgb);
	s_counter_expiry = millis() + ms_count;
	if (s_counter_expiry == 0) { s_counter_expiry = 1; }
}

void rgb_save(uint8_t(&rgb)[3], eRGBSetting setting)
{
	rgb_save_eeprom(rgb, EEPROM_ADDRESS_RGB[setting]);
}
