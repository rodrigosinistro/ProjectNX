#ifndef PROJECTNX_AUTH_H
#define PROJECTNX_AUTH_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#include "projectnx/config.h"

#define PNX_AUTH_DEVICE_CODE_CAPACITY 4096U
#define PNX_AUTH_USER_CODE_CAPACITY 32U
#define PNX_AUTH_URI_CAPACITY 192U
#define PNX_AUTH_DETAIL_CAPACITY 256U
#define PNX_AUTH_TOKEN_CAPACITY 8192U

typedef enum {
    PNX_AUTH_IDLE = 0,
    PNX_AUTH_CODE_READY,
    PNX_AUTH_PENDING,
    PNX_AUTH_AUTHENTICATED,
    PNX_AUTH_DECLINED,
    PNX_AUTH_EXPIRED,
    PNX_AUTH_FAILED
} PnxAuthStage;

typedef enum {
    PNX_AUTH_POLL_NOT_DUE = 0,
    PNX_AUTH_POLL_PENDING,
    PNX_AUTH_POLL_COMPLETE,
    PNX_AUTH_POLL_FATAL
} PnxAuthPollResult;

typedef struct {
    PnxAuthStage stage;
    long http_status;
    long expires_in_seconds;
    long poll_interval_seconds;
    time_t issued_at;
    time_t next_poll_at;
    char device_code[PNX_AUTH_DEVICE_CODE_CAPACITY];
    char user_code[PNX_AUTH_USER_CODE_CAPACITY];
    char verification_uri[PNX_AUTH_URI_CAPACITY];
    char access_token[PNX_AUTH_TOKEN_CAPACITY];
    char refresh_token[PNX_AUTH_TOKEN_CAPACITY];
    char detail[PNX_AUTH_DETAIL_CAPACITY];
} PnxAuthStatus;

void pnx_auth_init(PnxAuthStatus *status);
void pnx_auth_reset(PnxAuthStatus *status);
bool pnx_auth_request_device_code(
    const PnxConfig *config,
    PnxAuthStatus *status);
bool pnx_auth_poll_due(const PnxAuthStatus *status, time_t now);
PnxAuthPollResult pnx_auth_poll_token(
    const PnxConfig *config,
    PnxAuthStatus *status,
    time_t now);
const char *pnx_auth_stage_name(PnxAuthStage stage);

#endif
