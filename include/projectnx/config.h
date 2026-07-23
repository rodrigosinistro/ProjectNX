#ifndef PROJECTNX_CONFIG_H
#define PROJECTNX_CONFIG_H

#include <stdbool.h>
#include <stddef.h>

#define PNX_CLIENT_ID_CAPACITY 64U
#define PNX_TENANT_CAPACITY 40U

typedef struct {
    char client_id[PNX_CLIENT_ID_CAPACITY];
    char tenant[PNX_TENANT_CAPACITY];
    bool file_found;
    bool client_id_valid;
} PnxConfig;

void pnx_config_init(PnxConfig *config);
bool pnx_config_load(PnxConfig *config, const char *path);
bool pnx_config_parse_line(PnxConfig *config, const char *line);
bool pnx_config_validate_client_id(const char *client_id);

#endif
