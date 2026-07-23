#ifndef PROJECTNX_APP_H
#define PROJECTNX_APP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PNX_ERROR_MESSAGE_CAPACITY 160U

typedef enum {
    PNX_STATE_BOOT = 0,
    PNX_STATE_WELCOME,
    PNX_STATE_AUTH_REQUIRED,
    PNX_STATE_AUTH_WAITING,
    PNX_STATE_CATALOG,
    PNX_STATE_STREAM_CONNECTING,
    PNX_STATE_STREAMING,
    PNX_STATE_ERROR,
    PNX_STATE_EXITING,
    PNX_STATE_COUNT
} PnxState;

typedef enum {
    PNX_ACTION_NONE = 0,
    PNX_ACTION_CONFIRM,
    PNX_ACTION_BACK,
    PNX_ACTION_TOGGLE_DEBUG,
    PNX_ACTION_EXIT
} PnxAction;

typedef struct {
    PnxState state;
    PnxState previous_state;
    bool debug_visible;
    bool docked;
    uint32_t transition_count;
    char error_message[PNX_ERROR_MESSAGE_CAPACITY];
} PnxApp;

void pnx_app_init(PnxApp *app, bool docked);
void pnx_app_dispatch(PnxApp *app, PnxAction action);
void pnx_app_set_error(PnxApp *app, const char *message);
bool pnx_app_can_transition(PnxState from, PnxState to);
const char *pnx_state_name(PnxState state);
const char *pnx_state_title(PnxState state);
const char *pnx_state_description(PnxState state);

#endif

