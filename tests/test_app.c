#include "projectnx/app.h"
#include "projectnx/config.h"

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
    assert(app.state == PNX_STATE_NETWORK_CHECK);

    pnx_app_dispatch(&app, PNX_ACTION_NETWORK_READY);
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
    pnx_app_dispatch(&app, PNX_ACTION_NETWORK_READY);
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
    assert(pnx_app_can_transition(PNX_STATE_WELCOME, PNX_STATE_NETWORK_CHECK));
    assert(!pnx_app_can_transition(PNX_STATE_BOOT, PNX_STATE_STREAMING));
    assert(!pnx_app_can_transition(PNX_STATE_EXITING, PNX_STATE_WELCOME));
    assert(strcmp(pnx_state_name(PNX_STATE_COUNT), "INVALID") == 0);
}

static void test_config_parser(void)
{
    PnxConfig config;

    pnx_config_init(&config);
    assert(strcmp(config.tenant, "consumers") == 0);
    assert(!config.client_id_valid);

    assert(pnx_config_parse_line(
        &config,
        "client_id=00001111-aaaa-2222-bbbb-3333cccc4444"));
    assert(config.client_id_valid);

    assert(pnx_config_parse_line(&config, "tenant=consumers"));
    assert(strcmp(config.tenant, "consumers") == 0);
    assert(pnx_config_parse_line(&config, "# comentario"));
    assert(!pnx_config_parse_line(&config, "tenant=../../invalido"));
    assert(!pnx_config_validate_client_id("nao-e-um-guid"));
}

int main(void)
{
    test_happy_path();
    test_navigation_and_debug();
    test_error_recovery();
    test_transition_guard();
    test_config_parser();
    puts("ProjectNX core tests: OK");
    return 0;
}
