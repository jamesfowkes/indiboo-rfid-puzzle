#include <util/atomic.h>
#include <Adafruit_NeoPixel.h>
#include <TaskAction.h>

#include "check-and-clear.h"

extern "C"{
#include "button.h"
}

#include "fixed-length-accumulator.h"
#include "very-tiny-http.h"

#include "game-ethernet.h"
#include "ensb_eeprom.h"

#define CC(r,g,b) (r),(g),(b)
#define CA(r,g,b) (255-r),(255-g),(255-b)

static const uint8_t DEFAULT_MAC_ADDRESS[] = { 0xCA, 0xFE, 0xE5, 0xCA, 0xBE, 0x04 };
static const uint8_t DEFAULT_IP_ADDRESS[] = {192, 168, 0, 104};

static const uint8_t RELAY_PINS[2] = {2, 3};
static const uint8_t RGB_PINS[] = {5, 6, 9};

static void rgb_set(uint8_t r, uint8_t g, uint8_t b)
{
    analogWrite(RGB_PINS[0], r);
    analogWrite(RGB_PINS[1], g);
    analogWrite(RGB_PINS[2], b);
}

static void send_standard_erm_response()
{
    http_server_set_response_code("200 OK");
    http_server_set_header("Access-Control-Allow-Origin", "*");
    http_server_finish_headers();
}

static void debug_task_fn(TaskAction*task)
{
    (void)task;
}
static TaskAction s_debug_task(debug_task_fn, 500, INFINITE_TICKS);

static void print_ip(uint8_t * ip, bool nl=false)
{
    Serial.print((int)ip[0]); Serial.print('.');
    Serial.print((int)ip[1]); Serial.print('.');
    Serial.print((int)ip[2]); Serial.print('.');
    Serial.print((int)ip[3]);
    if (nl) { Serial.println(""); }
}

static void print_mac(uint8_t * mac, bool nl=false)
{
    Serial.print((int)mac[0], HEX); Serial.print(':');
    Serial.print((int)mac[1], HEX); Serial.print(':');
    Serial.print((int)mac[2], HEX); Serial.print(':');
    Serial.print((int)mac[3], HEX); Serial.print(':');
    Serial.print((int)mac[4], HEX); Serial.print(':');
    Serial.print((int)mac[5], HEX);
    if (nl) { Serial.println(""); }
}

static void set_ip_req_handler(char const * const url)
{
    (void)url;
    Serial.println(F("Handling /setting/ip"));
    char const * pSetting = NULL;
    if ((pSetting = strstr(url, "/setting/ip/")))
    {
        uint8_t ip[4];

        Serial.print(F("Setting IP address: "));
        sscanf(pSetting+12, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]);
        print_ip(ip, true);
        eeprom_set(eIP_ADDRESS, ip);
    }
    send_standard_erm_response();
}

static void set_mac_req_handler(char const * const url)
{
    Serial.println(F("Handling /setting/mac"));
    char const * pSetting = NULL;
    if ((pSetting = strstr(url, "/setting/mac/")))
    {
        uint8_t mac[6];

        Serial.print(F("Setting mac address: "));
        sscanf(pSetting+13, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
        print_mac(mac, true);
        eeprom_set(eMAC_ADDRESS, mac);
    }
    send_standard_erm_response();
}

static void win_game(char const * const url)
{
    (void)url;
    digitalWrite(RELAY_PINS[0], LOW);
    digitalWrite(RELAY_PINS[1], LOW);
    rgb_set(CC(0,0,64));
    send_standard_erm_response();
}

static void reset_game(char const * const url)
{
    (void)url;
    digitalWrite(RELAY_PINS[0], HIGH);
    digitalWrite(RELAY_PINS[1], HIGH);
    rgb_set(CC(64,0,0));
    send_standard_erm_response();
}

static void reset_req_handler(char const * const url)
{
    eeprom_set(eIP_ADDRESS, DEFAULT_IP_ADDRESS);
    eeprom_set(eMAC_ADDRESS, DEFAULT_MAC_ADDRESS);
    send_standard_erm_response();
}

static http_get_handler s_handlers[] = 
{
    {"/setting/reset", reset_req_handler},
    {"/setting/ip", set_ip_req_handler},
    {"/setting/mac", set_mac_req_handler},
    {"/game/win", win_game},
    {"/game/reset", reset_game},
    {"", NULL}
};

void setup()
{
    uint8_t ip[4];
    uint8_t mac[6];

    eeprom_setup();

    ethernet_setup(s_handlers);
    ethernet_get_ip(ip);
    ethernet_get_mac(mac);

    pinMode(RELAY_PINS[0], OUTPUT);
    pinMode(RELAY_PINS[1], OUTPUT);

    pinMode(RGB_PINS[0], OUTPUT);
    pinMode(RGB_PINS[1], OUTPUT);
    pinMode(RGB_PINS[2], OUTPUT);

    Serial.begin(115200);

    Serial.println("Cave Escape Ethernet Button");
    Serial.print("IP: ");
    print_ip(ip, true);
    Serial.print("MAC: ");
    print_mac(mac, true);

    reset_game("");

    delay(200);
}

void loop()
{
    s_debug_task.tick();
    ethernet_tick();    
}

static void handle_serial_cmd(char const * const cmd)
{
    if (cmd[0] == '/')
    {
        http_get_handler * pHandler = http_server_match_handler_url(cmd, s_handlers);
        if (pHandler)
        {
            pHandler->fn(cmd);
            Serial.println("***Response***");
            Serial.print(http_server_get_response());
            Serial.println("***Response End***");
        }
        else
        {
            Serial.print("URL '");
            Serial.print(cmd);
            Serial.println("' unknown");           
        }
    }
    else
    {
        Serial.print("Command '");
        Serial.print(cmd);
        Serial.println("' unknown");
    }
}

static char s_serial_buffer[64];
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
