#include "projectnx/xbox.h"

#include <stdio.h>
#include <string.h>

#include "projectnx/json.h"

static void secure_clear(char *value, size_t capacity)
{
    volatile unsigned char *cursor = (volatile unsigned char *)value;

    while (capacity > 0U) {
        *cursor = 0U;
        cursor++;
        capacity--;
    }
}

static void clear_parsed_result(PnxXboxStatus *status)
{
    secure_clear(status->xsts_token, sizeof(status->xsts_token));
    secure_clear(status->gamertag, sizeof(status->gamertag));
    secure_clear(status->profile_id, sizeof(status->profile_id));
    secure_clear(status->not_after, sizeof(status->not_after));
}

bool pnx_xbox_parse_xsts_response(
    const char *response,
    PnxXboxStatus *status)
{
    char token[PNX_XBOX_TOKEN_CAPACITY];
    char user_hash[PNX_XBOX_USER_HASH_CAPACITY];
    char gamertag[PNX_XBOX_GAMERTAG_CAPACITY];
    char modern_suffix[PNX_XBOX_GAMERTAG_CAPACITY];
    char profile_id[PNX_XBOX_PROFILE_ID_CAPACITY];
    char not_after[PNX_XBOX_TIMESTAMP_CAPACITY];
    bool parsed;

    if (response == NULL || status == NULL) {
        return false;
    }

    memset(token, 0, sizeof(token));
    memset(user_hash, 0, sizeof(user_hash));
    memset(gamertag, 0, sizeof(gamertag));
    memset(modern_suffix, 0, sizeof(modern_suffix));
    memset(profile_id, 0, sizeof(profile_id));
    memset(not_after, 0, sizeof(not_after));

    parsed =
        pnx_json_get_string(response, "Token", token, sizeof(token)) &&
        pnx_json_get_string(response, "uhs", user_hash, sizeof(user_hash));
    if (!parsed) {
        secure_clear(token, sizeof(token));
        secure_clear(user_hash, sizeof(user_hash));
        clear_parsed_result(status);
        return false;
    }

    (void)pnx_json_get_string(
        response,
        "gtg",
        gamertag,
        sizeof(gamertag));
    if (gamertag[0] == '\0' &&
        pnx_json_get_string(
            response,
            "mgt",
            gamertag,
            sizeof(gamertag)) &&
        pnx_json_get_string(
            response,
            "mgs",
            modern_suffix,
            sizeof(modern_suffix)) &&
        modern_suffix[0] != '\0') {
        const size_t gamertag_length = strlen(gamertag);
        const size_t suffix_length = strlen(modern_suffix);

        if (gamertag_length + suffix_length + 2U <= sizeof(gamertag)) {
            gamertag[gamertag_length] = '#';
            memcpy(
                gamertag + gamertag_length + 1U,
                modern_suffix,
                suffix_length + 1U);
        }
    }
    if (!pnx_json_get_string(
            response,
            "ptx",
            profile_id,
            sizeof(profile_id)) &&
        !pnx_json_get_string(
            response,
            "upi",
            profile_id,
            sizeof(profile_id))) {
        (void)pnx_json_get_string(
            response,
            "xid",
            profile_id,
            sizeof(profile_id));
    }
    (void)pnx_json_get_string(
        response,
        "NotAfter",
        not_after,
        sizeof(not_after));

    clear_parsed_result(status);
    (void)snprintf(
        status->xsts_token,
        sizeof(status->xsts_token),
        "%s",
        token);
    (void)snprintf(
        status->user_hash,
        sizeof(status->user_hash),
        "%s",
        user_hash);
    (void)snprintf(
        status->gamertag,
        sizeof(status->gamertag),
        "%s",
        gamertag);
    (void)snprintf(
        status->profile_id,
        sizeof(status->profile_id),
        "%s",
        profile_id);
    (void)snprintf(
        status->not_after,
        sizeof(status->not_after),
        "%s",
        not_after);

    secure_clear(token, sizeof(token));
    secure_clear(user_hash, sizeof(user_hash));
    secure_clear(gamertag, sizeof(gamertag));
    secure_clear(modern_suffix, sizeof(modern_suffix));
    secure_clear(profile_id, sizeof(profile_id));
    secure_clear(not_after, sizeof(not_after));
    return true;
}
