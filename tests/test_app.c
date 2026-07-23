#include "projectnx/app.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_happy_path(void)
{
    PnxApp app;
    pnx_app_init(&app, false);
    assert(app.state == PNX_STATE_WELCOME);
    assert(app.transition_count == 1U);

    pnx_app_dispatch(&app, PNX_ACTION_CONFIRM);
    assert(app.state == PNX_STATE_AUTH_REQUIRED);

    pnx_app_dispatch(&app, PNX_ACTION_CONFIRM);
    assert(app.state == PNX_STATE_AUTH_WAITING);

    pnx_app_dispatch(&app, PNX_ACTION_CONFIRM);
    assert(app.state == PNX_STATE_CATALOG);

    pnx_app_dispatch(&app, PNX_ACTION_CONFIRM);
    assert(app.state == PNX_STATE_STREAM_CONNECTING);

    pnx_app_dispatch(&app, PNX_ACTION_CONFIRM);
    assert(app.state == PNX_STATE_STREAMING);

    pnx_app_dispatch(&app, PNX_ACTION_CONFIRM);
    assert(app.state == PNX_STATE_CATALOG);
}

static void test_navigation_and_debug(void)
{
    PnxApp app;
    pnx_app_init(&app, true);
    assert(app.docked);
    assert(!app.debug_visible);

    pnx_app_dispatch(&app, PNX_ACTION_TOGGLE_DEBUG);
    assert(app.debug_visible);

    pnx_app_dispatch(&app, PNX_ACTION_CONFIRM);
    pnx_app_dispatch(&app, PNX_ACTION_BACK);
    assert(app.state == PNX_STATE_WELCOME);

    pnx_app_dispatch(&app, PNX_ACTION_EXIT);
    assert(app.state == PNX_STATE_EXITING);
}

static void test_error_recovery(void)
{
    PnxApp app;
    pnx_app_init(&app, false);
    pnx_app_dispatch(&app, PNX_ACTION_CONFIRM);
    assert(app.state == PNX_STATE_AUTH_REQUIRED);

    pnx_app_set_error(&app, "Falha simulada");
    assert(app.state == PNX_STATE_ERROR);
    assert(strcmp(app.error_message, "Falha simulada") == 0);

    pnx_app_dispatch(&app, PNX_ACTION_CONFIRM);
    assert(app.state == PNX_STATE_AUTH_REQUIRED);
}

static void test_transition_guard(void)
{
    assert(pnx_app_can_transition(PNX_STATE_BOOT, PNX_STATE_WELCOME));
    assert(!pnx_app_can_transition(PNX_STATE_BOOT, PNX_STATE_STREAMING));
    assert(!pnx_app_can_transition(PNX_STATE_EXITING, PNX_STATE_WELCOME));
    assert(strcmp(pnx_state_name(PNX_STATE_COUNT), "INVALID") == 0);
}

int main(void)
{
    test_happy_path();
    test_navigation_and_debug();
    test_error_recovery();
    test_transition_guard();
    puts("ProjectNX core tests: OK");
    return 0;
}

