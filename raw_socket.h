#ifndef RAW_SOCKET_H
#define RAW_SOCKET_H

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>

int cria_raw_socket(char*);

#endif