#include "projectnx/json.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static const char *skip_whitespace(const char *cursor)
{
    while (cursor != NULL && *cursor != '\0' &&
           isspace((unsigned char)*cursor)) {
        cursor++;
    }
    return cursor;
}

static int hex_value(char value)
{
    if (value >= '0' && value <= '9') {
        return value - '0';
    }
    if (value >= 'a' && value <= 'f') {
        return value - 'a' + 10;
    }
    if (value >= 'A' && value <= 'F') {
        return value - 'A' + 10;
    }
    return -1;
}

static bool append_byte(
    char *output,
    size_t output_capacity,
    size_t *output_length,
    unsigned char value)
{
    if (*output_length + 1U >= output_capacity) {
        return false;
    }
    output[*output_length] = (char)value;
    (*output_length)++;
    return true;
}

static bool append_codepoint(
    char *output,
    size_t output_capacity,
    size_t *output_length,
    unsigned int codepoint)
{
    if (codepoint <= 0x7FU) {
        return append_byte(
            output,
            output_capacity,
            output_length,
            (unsigned char)codepoint);
    }
    if (codepoint <= 0x7FFU) {
        return append_byte(
                   output,
                   output_capacity,
                   output_length,
                   (unsigned char)(0xC0U | (codepoint >> 6U))) &&
               append_byte(
                   output,
                   output_capacity,
                   output_length,
                   (unsigned char)(0x80U | (codepoint & 0x3FU)));
    }
    if (codepoint >= 0xD800U && codepoint <= 0xDFFFU) {
        return append_byte(output, output_capacity, output_length, '?');
    }
    return append_byte(
               output,
               output_capacity,
               output_length,
               (unsigned char)(0xE0U | (codepoint >> 12U))) &&
           append_byte(
               output,
               output_capacity,
               output_length,
               (unsigned char)(0x80U | ((codepoint >> 6U) & 0x3FU))) &&
           append_byte(
               output,
               output_capacity,
               output_length,
               (unsigned char)(0x80U | (codepoint & 0x3FU)));
}

static bool parse_string(
    const char *cursor,
    char *output,
    size_t output_capacity,
    const char **end)
{
    size_t output_length = 0U;

    if (cursor == NULL || output == NULL || output_capacity == 0U ||
        cursor[0] != '"') {
        return false;
    }

    cursor++;
    while (*cursor != '\0' && *cursor != '"') {
        unsigned int codepoint;
        unsigned char value = (unsigned char)*cursor;

        if (value < 0x20U) {
            return false;
        }

        if (*cursor != '\\') {
            if (!append_byte(
                    output,
                    output_capacity,
                    &output_length,
                    value)) {
                return false;
            }
            cursor++;
            continue;
        }

        cursor++;
        switch (*cursor) {
        case '"':
        case '\\':
        case '/':
            value = (unsigned char)*cursor;
            cursor++;
            break;
        case 'b':
            value = '\b';
            cursor++;
            break;
        case 'f':
            value = '\f';
            cursor++;
            break;
        case 'n':
            value = '\n';
            cursor++;
            break;
        case 'r':
            value = '\r';
            cursor++;
            break;
        case 't':
            value = '\t';
            cursor++;
            break;
        case 'u': {
            size_t index;
            codepoint = 0U;
            cursor++;
            for (index = 0U; index < 4U; index++) {
                const int digit = hex_value(cursor[index]);
                if (digit < 0) {
                    return false;
                }
                codepoint = (codepoint << 4U) | (unsigned int)digit;
            }
            cursor += 4;
            if (!append_codepoint(
                    output,
                    output_capacity,
                    &output_length,
                    codepoint)) {
                return false;
            }
            continue;
        }
        default:
            return false;
        }

        if (!append_byte(
                output,
                output_capacity,
                &output_length,
                value)) {
            return false;
        }
    }

    if (*cursor != '"') {
        return false;
    }

    output[output_length] = '\0';
    if (end != NULL) {
        *end = cursor + 1;
    }
    return true;
}

static bool skip_string(const char *cursor, const char **end)
{
    if (cursor == NULL || cursor[0] != '"') {
        return false;
    }

    cursor++;
    while (*cursor != '\0') {
        if ((unsigned char)*cursor < 0x20U) {
            return false;
        }
        if (*cursor == '"') {
            if (end != NULL) {
                *end = cursor + 1;
            }
            return true;
        }
        if (*cursor != '\\') {
            cursor++;
            continue;
        }

        cursor++;
        if (*cursor == 'u') {
            size_t index;
            cursor++;
            for (index = 0U; index < 4U; index++) {
                if (hex_value(cursor[index]) < 0) {
                    return false;
                }
            }
            cursor += 4;
        } else if (*cursor == '"' || *cursor == '\\' || *cursor == '/' ||
                   *cursor == 'b' || *cursor == 'f' || *cursor == 'n' ||
                   *cursor == 'r' || *cursor == 't') {
            cursor++;
        } else {
            return false;
        }
    }

    return false;
}

static const char *find_value(const char *json, const char *key)
{
    const char *cursor;
    char parsed_key[96];

    if (json == NULL || key == NULL || key[0] == '\0') {
        return NULL;
    }

    cursor = json;
    while (*cursor != '\0') {
        const char *after_string;
        const char *after_key;

        if (*cursor != '"') {
            cursor++;
            continue;
        }

        if (!skip_string(cursor, &after_string)) {
            return NULL;
        }

        after_key = skip_whitespace(after_string);
        if (*after_key == ':') {
            if (!parse_string(
                    cursor,
                    parsed_key,
                    sizeof(parsed_key),
                    NULL)) {
                return NULL;
            }
            if (strcmp(parsed_key, key) == 0) {
                return skip_whitespace(after_key + 1);
            }
        }
        cursor = after_string;
    }

    return NULL;
}

bool pnx_json_get_string(
    const char *json,
    const char *key,
    char *output,
    size_t output_capacity)
{
    const char *value = find_value(json, key);

    if (value == NULL || *value != '"') {
        return false;
    }
    return parse_string(value, output, output_capacity, NULL);
}

bool pnx_json_get_long(const char *json, const char *key, long *output)
{
    const char *value = find_value(json, key);
    char *end;
    long parsed;

    if (value == NULL || output == NULL) {
        return false;
    }

    errno = 0;
    parsed = strtol(value, &end, 10);
    if (value == end || errno == ERANGE ||
        parsed == LONG_MIN || parsed == LONG_MAX) {
        return false;
    }

    end = (char *)skip_whitespace(end);
    if (*end != ',' && *end != '}' && *end != '\0') {
        return false;
    }

    *output = parsed;
    return true;
}
