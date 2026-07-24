#include "projectnx/app.h"
#include "projectnx/config.h"
#include "projectnx/json.h"

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

    pnx_app_dispatch(&app, PNX_ACTION_AUTH_COMPLETE);
    assert(app.state == PNX_STATE_XBOX_AUTH);

    pnx_app_dispatch(&app, PNX_ACTION_XBOX_COMPLETE);
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

static void test_auth_wait_requires_completion(void)
{
    PnxApp app;

    pnx_app_init(&app, false);
    pnx_app_dispatch(&app, PNX_ACTION_CONFIRM);
    pnx_app_dispatch(&app, PNX_ACTION_NETWORK_READY);
    pnx_app_dispatch(&app, PNX_ACTION_CONFIRM);
    assert(app.state == PNX_STATE_AUTH_WAITING);

    pnx_app_dispatch(&app, PNX_ACTION_CONFIRM);
    assert(app.state == PNX_STATE_AUTH_WAITING);

    pnx_app_dispatch(&app, PNX_ACTION_AUTH_COMPLETE);
    assert(app.state == PNX_STATE_XBOX_AUTH);

    pnx_app_dispatch(&app, PNX_ACTION_XBOX_COMPLETE);
    assert(app.state == PNX_STATE_CATALOG);
}

static void test_json_parser(void)
{
    const char *json =
        "{\"user_code\":\"ABCD-EFGH\",\"expires_in\":900,"
        "\"message\":\"Linha 1\\nLinha 2\",\"escaped\":\"Ol\\u00e1\"}";
    char value[64];
    long number = 0L;

    assert(pnx_json_get_string(json, "user_code", value, sizeof(value)));
    assert(strcmp(value, "ABCD-EFGH") == 0);
    assert(pnx_json_get_long(json, "expires_in", &number));
    assert(number == 900L);
    assert(pnx_json_get_string(json, "message", value, sizeof(value)));
    assert(strcmp(value, "Linha 1\nLinha 2") == 0);
    assert(pnx_json_get_string(json, "escaped", value, sizeof(value)));
    assert(strcmp(value, "Olá") == 0);
    assert(!pnx_json_get_string(json, "ausente", value, sizeof(value)));
}

static void test_json_escaping(void)
{
    char escaped[64];

    assert(pnx_json_escape_string(
        "token\"com\\escape\n",
        escaped,
        sizeof(escaped)));
    assert(strcmp(escaped, "token\\\"com\\\\escape\\n") == 0);
    assert(!pnx_json_escape_string("grande", escaped, 4U));
}

static void test_xbox_user_token_response(void)
{
    const char *json =
        "{"
        "\"IssueInstant\":\"2026-07-23T12:00:00Z\","
        "\"Token\":\"opaque-xbox-user-token\","
        "\"DisplayClaims\":{\"xui\":[{\"uhs\":\"1234567890\"}]}"
        "}";
    char token[64];
    char user_hash[32];

    assert(pnx_json_get_string(json, "Token", token, sizeof(token)));
    assert(strcmp(token, "opaque-xbox-user-token") == 0);
    assert(pnx_json_get_string(json, "uhs", user_hash, sizeof(user_hash)));
    assert(strcmp(user_hash, "1234567890") == 0);
}

static void test_large_json_string(void)
{
    char json[1200];
    char value[1030];
    const char *prefix = "{\"device_code\":\"";
    const char *suffix =
        "\",\"verification_uri\":\"https://microsoft.com/devicelogin\"}";
    const size_t prefix_length = strlen(prefix);
    const size_t suffix_length = strlen(suffix);
    size_t index;

    (void)snprintf(json, sizeof(json), "%s", prefix);
    for (index = 0U; index < 1024U; index++) {
        json[prefix_length + index] = 'A';
    }
    memcpy(
        json + prefix_length + 1024U,
        suffix,
        suffix_length + 1U);

    assert(pnx_json_get_string(
        json,
        "device_code",
        value,
        sizeof(value)));
    assert(strlen(value) == 1024U);
    assert(pnx_json_get_string(
        json,
        "verification_uri",
        value,
        sizeof(value)));
    assert(strcmp(value, "https://microsoft.com/devicelogin") == 0);
}

int main(void)
{
    test_happy_path();
    test_navigation_and_debug();
    test_error_recovery();
    test_transition_guard();
    test_config_parser();
    test_auth_wait_requires_completion();
    test_json_parser();
    test_json_escaping();
    test_xbox_user_token_response();
    test_large_json_string();
    puts("ProjectNX core tests: OK");
    return 0;
}
