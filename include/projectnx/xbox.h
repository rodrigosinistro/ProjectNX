#ifndef PROJECTNX_XBOX_H
#define PROJECTNX_XBOX_H

#include <stdbool.h>

#include "projectnx/auth.h"

#define PNX_XBOX_TOKEN_CAPACITY 16384U
#define PNX_XBOX_USER_HASH_CAPACITY 128U
#define PNX_XBOX_GAMERTAG_CAPACITY 128U
#define PNX_XBOX_PROFILE_ID_CAPACITY 128U
#define PNX_XBOX_TIMESTAMP_CAPACITY 64U
#define PNX_XBOX_DETAIL_CAPACITY 256U

typedef enum {
    PNX_XBOX_IDLE = 0,
    PNX_XBOX_USER_CONNECTING,
    PNX_XBOX_USER_AUTHENTICATED,
    PNX_XBOX_XSTS_CONNECTING,
    PNX_XBOX_XSTS_AUTHENTICATED,
    PNX_XBOX_FAILED
} PnxXboxStage;

typedef struct {
    PnxXboxStage stage;
    long http_status;
    long xbox_error;
    char user_token[PNX_XBOX_TOKEN_CAPACITY];
    char xsts_token[PNX_XBOX_TOKEN_CAPACITY];
    char user_hash[PNX_XBOX_USER_HASH_CAPACITY];
    char gamertag[PNX_XBOX_GAMERTAG_CAPACITY];
    char profile_id[PNX_XBOX_PROFILE_ID_CAPACITY];
    char not_after[PNX_XBOX_TIMESTAMP_CAPACITY];
    char detail[PNX_XBOX_DETAIL_CAPACITY];
} PnxXboxStatus;

void pnx_xbox_init(PnxXboxStatus *status);
void pnx_xbox_reset(PnxXboxStatus *status);
bool pnx_xbox_authenticate_user(
    const PnxAuthStatus *microsoft_auth,
    PnxXboxStatus *status);
bool pnx_xbox_parse_xsts_response(
    const char *response,
    PnxXboxStatus *status);
bool pnx_xbox_authorize_xsts(PnxXboxStatus *status);
const char *pnx_xbox_stage_name(PnxXboxStage stage);

#endif
