#include <EtherCard.h>

#include "ensb_eeprom.h"

#include "fixed-length-accumulator.h"
#include "very-tiny-http.h"

static const uint16_t BUF_SIZE = 256;
static uint8_t MAC_ADDRESS[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
static uint8_t IP_ADDRESS[] = {192, 168, 0, 99};
static const uint8_t GATEWAY[] = {255, 255, 255, 0};

byte Ethernet::buffer[BUF_SIZE];
static BufferFiller bfill;

static http_get_handler * s_pHandlers = NULL;

static word sendEthernet(char * to_send)
{
    bfill = ether.tcpOffset();
    bfill.emit_raw(to_send, strlen(to_send));
    return bfill.position();
}

void ethernet_setup(http_get_handler * pHandlers)
{
    eeprom_get(eIP_ADDRESS, IP_ADDRESS);
    eeprom_get(eMAC_ADDRESS, MAC_ADDRESS);

	if (ether.begin(sizeof Ethernet::buffer, MAC_ADDRESS) == 0) 
    {
        Serial.println(F("Failed to access Ethernet controller"));
    }

    s_pHandlers = pHandlers;
	ether.staticSetup(IP_ADDRESS, GATEWAY);
}

void ethernet_tick()
{
	word len = ether.packetReceive();
    word pos = ether.packetLoop(len);

    if (pos)
    {
        Serial.println(F("Handling request:"));
        Serial.println((char *) Ethernet::buffer + pos);
        http_server_handle_req(s_pHandlers, (char *) Ethernet::buffer + pos);
        ether.httpServerReply(sendEthernet(http_server_get_response()));
    }
}

void ethernet_get_ip(uint8_t * ip)
{
    memcpy(ip, IP_ADDRESS, 4);
}

void ethernet_get_mac(uint8_t * mac)
{
    memcpy(mac, MAC_ADDRESS, 6);
}
