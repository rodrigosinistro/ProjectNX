#include "projectnx/config.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static char *trim(char *value)
{
    char *end;

    while (*value != '\0' && isspace((unsigned char)*value)) {
        value++;
    }

    if (*value == '\0') {
        return value;
    }

    end = value + strlen(value) - 1U;
    while (end > value && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    return value;
}

static bool tenant_is_safe(const char *tenant)
{
    size_t index;
    const size_t length = strlen(tenant);

    if (length == 0U || length >= PNX_TENANT_CAPACITY) {
        return false;
    }

    for (index = 0U; index < length; index++) {
        const unsigned char current = (unsigned char)tenant[index];
        if (!isalnum(current) && current != '-' && current != '.') {
            return false;
        }
    }

    return true;
}

void pnx_config_init(PnxConfig *config)
{
    if (config == NULL) {
        return;
    }

    memset(config, 0, sizeof(*config));
    (void)snprintf(config->tenant, sizeof(config->tenant), "%s", "consumers");
}

bool pnx_config_validate_client_id(const char *client_id)
{
    static const size_t hyphens[] = {8U, 13U, 18U, 23U};
    size_t index;
    size_t hyphen_index = 0U;

    if (client_id == NULL || strlen(client_id) != 36U) {
        return false;
    }

    for (index = 0U; index < 36U; index++) {
        if (hyphen_index < sizeof(hyphens) / sizeof(hyphens[0]) &&
            index == hyphens[hyphen_index]) {
            if (client_id[index] != '-') {
                return false;
            }
            hyphen_index++;
        } else if (!isxdigit((unsigned char)client_id[index])) {
            return false;
        }
    }

    return true;
}

bool pnx_config_parse_line(PnxConfig *config, const char *line)
{
    char buffer[256];
    char *separator;
    char *key;
    char *value;

    if (config == NULL || line == NULL || strlen(line) >= sizeof(buffer)) {
        return false;
    }

    (void)snprintf(buffer, sizeof(buffer), "%s", line);
    key = trim(buffer);
    if (*key == '\0' || *key == '#' || *key == ';') {
        return true;
    }

    separator = strchr(key, '=');
    if (separator == NULL) {
        return false;
    }

    *separator = '\0';
    value = trim(separator + 1);
    key = trim(key);

    if (strcmp(key, "client_id") == 0) {
        if (strlen(value) >= sizeof(config->client_id)) {
            return false;
        }
        (void)snprintf(config->client_id, sizeof(config->client_id), "%s", value);
        config->client_id_valid =
            pnx_config_validate_client_id(config->client_id);
        return *value == '\0' || config->client_id_valid;
    }

    if (strcmp(key, "tenant") == 0) {
        if (!tenant_is_safe(value)) {
            return false;
        }
        (void)snprintf(config->tenant, sizeof(config->tenant), "%s", value);
        return true;
    }

    return true;
}

bool pnx_config_load(PnxConfig *config, const char *path)
{
    FILE *file;
    char line[256];
    bool valid = true;

    if (config == NULL || path == NULL) {
        return false;
    }

    pnx_config_init(config);
    file = fopen(path, "r");
    if (file == NULL) {
        return false;
    }

    config->file_found = true;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (!pnx_config_parse_line(config, line)) {
            valid = false;
        }
    }

    (void)fclose(file);
    return valid;
}
