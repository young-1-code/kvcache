#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdint.h>

#define MAX_LISTENTER_NUM (1000U)

int create_server(const char* ip, uint16_t port);

#endif
