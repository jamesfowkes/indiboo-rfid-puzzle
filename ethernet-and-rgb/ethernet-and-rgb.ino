#include "EEPROMex.h"

#include "fixed-length-accumulator.h"
#include "very-tiny-http.h"

#include "commands.h"
#include "rgb.h"
#include "game-state.h"
#include "game-ethernet.h"

static const uint8_t RELAY_PINS[2] = {2, 3};

static void send_standard_erm_response()
{
    http_server_set_response_code("200 OK");
    http_server_set_header("Access-Control-Allow-Origin", "*");
    http_server_finish_headers();
}

static bool parse_and_set_rgb(char const * const pRGB, uint8_t(&rgb)[3], eRGBSetting setting)
{
    bool valid_value = rgb_parse_hex(pRGB, rgb);
    if (valid_value)
    {
        rgb_save(rgb, setting);
    }
    return valid_value;
}

static void set_rgb_handler(char const * const url)
{
    uint8_t rgb[3];

    Serial.println(F("Handling /rgb"));

    char const * pSetting = NULL;
    bool valid_value = true;
    if ((pSetting = strstr(url, "/correct/")))
    {
        Serial.print(F("Setting correct RGB values:"));
        valid_value = parse_and_set_rgb(pSetting + 9, rgb, eRGBSetting_Correct);
    }
    else if ((pSetting = strstr(url, "/incorrect/")))
    {
        Serial.print(F("Setting incorrect RGB values: "));
        valid_value = parse_and_set_rgb(pSetting + 11, rgb, eRGBSetting_Incorrect);
    }
    else
    {
        Serial.print("Invalid URL");
    }

    if (valid_value && pSetting)
    {
        rgb_print(rgb, true);
        rgb_flash(rgb, 2000);
    }
    else
    {
        Serial.println(F("Could not parse RGB value"));
    }
    send_standard_erm_response();
}

static void reset_handler(char const * const url)
{
    (void)url;
    Serial.println(F("Handling /reset"));
    send_standard_erm_response();
    command_send(eCommand_Reset);
}

static void wipe_handler(char const * const url)
{
    (void)url;
    Serial.println(F("Handling /wipe"));
    send_standard_erm_response();
    command_send(eCommand_WipeMemory);
}

static void store_handler(char const * const url)
{
    (void)url;
    Serial.println(F("Handling /store"));
    send_standard_erm_response();
    command_send(eCommand_StoreMemory);
}

static void win_handler(char const * const url)
{
    (void)url;
    Serial.println(F("Handling /win"));
    send_standard_erm_response();
    command_send(eCommand_SetWonGameState);
}

static http_get_handler s_handlers[] = 
{
    {"/reset", reset_handler},
    {"/wipe", wipe_handler},
    {"/store", store_handler},
    {"/win", win_handler},
    {"/rgb/correct", set_rgb_handler},
    {"/rgb/incorrect", set_rgb_handler},
    {"", NULL}
};

void setup()
{
    Serial.begin(115200);

    Serial.println(F("Cave Escape Moondial Ethernet/LED"));

    ethernet_setup(s_handlers);

    game_state_setup();

    command_setup();
    
    rgb_setup();
    rgb_intro();

    pinMode(RELAY_PINS[0], OUTPUT);
    pinMode(RELAY_PINS[1], OUTPUT);

    Serial.println(F("Starting..."));
}

void loop()
{
    command_tick();
    rgb_tick();
    game_state_tick();
    ethernet_tick();

    switch(game_state_get())
    {
    case eGameState_InProgress:
    case eGameState_IncorrectCombination:
        digitalWrite(RELAY_PINS[0], HIGH);
        digitalWrite(RELAY_PINS[1], HIGH);
        break;
    case eGameState_CorrectCombination:
        digitalWrite(RELAY_PINS[0], LOW);
        digitalWrite(RELAY_PINS[1], LOW);
        break;
    }
}
