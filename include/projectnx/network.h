#ifndef PROJECTNX_NETWORK_H
#define PROJECTNX_NETWORK_H

#include <stdbool.h>

#define PNX_NETWORK_DETAIL_CAPACITY 192U

typedef struct {
    bool initialized;
    bool checked;
    bool online;
    long http_status;
    char detail[PNX_NETWORK_DETAIL_CAPACITY];
} PnxNetworkStatus;

void pnx_network_status_init(PnxNetworkStatus *status);
bool pnx_network_platform_init(PnxNetworkStatus *status);
bool pnx_network_probe(PnxNetworkStatus *status);
void pnx_network_platform_exit(PnxNetworkStatus *status);

#endif
