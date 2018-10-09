#ifndef _GAME_ETHERNET_H_
#define _GAME_ETHERNET_H_

void ethernet_setup(http_get_handler * pHandlers);
void ethernet_tick();

void ethernet_get_ip(uint8_t * ip);
void ethernet_get_mac(uint8_t * mac);
#endif
