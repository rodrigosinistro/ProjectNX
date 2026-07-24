#include "projectnx/xbox.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_xsts_response_with_profile(void)
{
    const char *response =
        "{"
        "\"IssueInstant\":\"2026-07-23T12:00:00Z\","
        "\"NotAfter\":\"2026-07-24T12:00:00Z\","
        "\"Token\":\"opaque-xsts-token\","
        "\"DisplayClaims\":{\"xui\":[{"
        "\"uhs\":\"1234567890\","
        "\"ptx\":\"partner-user-9876543210\","
        "\"gtg\":\"RodrigoNX\""
        "}]}"
        "}";
    PnxXboxStatus status;

    pnx_xbox_init(&status);
    assert(pnx_xbox_parse_xsts_response(response, &status));
    assert(strcmp(status.xsts_token, "opaque-xsts-token") == 0);
    assert(strcmp(status.user_hash, "1234567890") == 0);
    assert(strcmp(status.profile_id, "partner-user-9876543210") == 0);
    assert(strcmp(status.gamertag, "RodrigoNX") == 0);
    assert(strcmp(status.not_after, "2026-07-24T12:00:00Z") == 0);
}

static void test_xsts_response_without_optional_claims(void)
{
    const char *response =
        "{\"Token\":\"opaque\",\"DisplayClaims\":{\"xui\":[{\"uhs\":\"1\"}]}}";
    PnxXboxStatus status;

    pnx_xbox_init(&status);
    assert(pnx_xbox_parse_xsts_response(response, &status));
    assert(strcmp(status.xsts_token, "opaque") == 0);
    assert(strcmp(status.user_hash, "1") == 0);
    assert(status.gamertag[0] == '\0');
    assert(status.profile_id[0] == '\0');
    assert(status.not_after[0] == '\0');
}

static void test_modern_gamertag_fallback(void)
{
    const char *response =
        "{"
        "\"Token\":\"opaque\","
        "\"DisplayClaims\":{\"xui\":[{"
        "\"uhs\":\"1\","
        "\"mgt\":\"Rodrigo\","
        "\"mgs\":\"4321\""
        "}]}"
        "}";
    PnxXboxStatus status;

    pnx_xbox_init(&status);
    assert(pnx_xbox_parse_xsts_response(response, &status));
    assert(strcmp(status.gamertag, "Rodrigo#4321") == 0);
}

static void test_incomplete_xsts_response_is_rejected(void)
{
    PnxXboxStatus status;

    pnx_xbox_init(&status);
    (void)snprintf(
        status.xsts_token,
        sizeof(status.xsts_token),
        "%s",
        "stale-token");
    assert(!pnx_xbox_parse_xsts_response(
        "{\"DisplayClaims\":{\"xui\":[{\"uhs\":\"1\"}]}}",
        &status));
    assert(status.xsts_token[0] == '\0');
}

static void test_stage_names(void)
{
    assert(strcmp(
        pnx_xbox_stage_name(PNX_XBOX_USER_CONNECTING),
        "USER_CONNECTING") == 0);
    assert(strcmp(
        pnx_xbox_stage_name(PNX_XBOX_XSTS_AUTHENTICATED),
        "XSTS_AUTHENTICATED") == 0);
    assert(strcmp(
        pnx_xbox_stage_name((PnxXboxStage)99),
        "INVALID") == 0);
}

int main(void)
{
    test_xsts_response_with_profile();
    test_xsts_response_without_optional_claims();
    test_modern_gamertag_fallback();
    test_incomplete_xsts_response_is_rejected();
    test_stage_names();
    puts("ProjectNX Xbox tests: OK");
    return 0;
}
