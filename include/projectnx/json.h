#ifndef PROJECTNX_JSON_H
#define PROJECTNX_JSON_H

#include <stdbool.h>
#include <stddef.h>

bool pnx_json_get_string(
    const char *json,
    const char *key,
    char *output,
    size_t output_capacity);
bool pnx_json_get_long(const char *json, const char *key, long *output);
bool pnx_json_escape_string(
    const char *input,
    char *output,
    size_t output_capacity);

#endif
