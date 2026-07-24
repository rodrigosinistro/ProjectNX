#include "projectnx/xbox.h"

#include <curl/curl.h>

#include <stdio.h>
#include <string.h>

#include "projectnx/json.h"

#ifndef PROJECTNX_VERSION
#define PROJECTNX_VERSION "dev"
#endif

#define PNX_XBOX_USER_AUTH_URL \
    "https://user.auth.xboxlive.com/user/authenticate"
#define PNX_XBOX_XSTS_AUTH_URL \
    "https://xsts.auth.xboxlive.com/xsts/authorize"
#define PNX_XBOX_XSTS_RELYING_PARTY "http://xboxlive.com"
#define PNX_XBOX_SANDBOX "RETAIL"
#define PNX_XBOX_RESPONSE_CAPACITY 32768U
#define PNX_XBOX_ESCAPED_TOKEN_CAPACITY 24576U
#define PNX_XBOX_REQUEST_CAPACITY 32768U

typedef struct {
    char data[PNX_XBOX_RESPONSE_CAPACITY];
    size_t length;
    bool overflow;
} PnxXboxResponse;

static void secure_clear(char *value, size_t capacity)
{
    volatile unsigned char *cursor = (volatile unsigned char *)value;

    while (capacity > 0U) {
        *cursor = 0U;
        cursor++;
        capacity--;
    }
}

static void clear_credentials(PnxXboxStatus *status)
{
    if (status == NULL) {
        return;
    }
    secure_clear(status->user_token, sizeof(status->user_token));
    secure_clear(status->xsts_token, sizeof(status->xsts_token));
    secure_clear(status->user_hash, sizeof(status->user_hash));
    secure_clear(status->gamertag, sizeof(status->gamertag));
    secure_clear(status->profile_id, sizeof(status->profile_id));
    secure_clear(status->not_after, sizeof(status->not_after));
}

static void clear_xsts_result(PnxXboxStatus *status)
{
    if (status == NULL) {
        return;
    }
    secure_clear(status->xsts_token, sizeof(status->xsts_token));
    secure_clear(status->gamertag, sizeof(status->gamertag));
    secure_clear(status->profile_id, sizeof(status->profile_id));
    secure_clear(status->not_after, sizeof(status->not_after));
}

static void set_detail(PnxXboxStatus *status, const char *detail)
{
    if (status == NULL) {
        return;
    }
    if (detail == NULL || detail[0] == '\0') {
        detail = "Falha Xbox sem detalhes.";
    }
    (void)snprintf(status->detail, sizeof(status->detail), "%s", detail);
}

static size_t collect_response(
    void *contents,
    size_t element_size,
    size_t element_count,
    void *user_data)
{
    PnxXboxResponse *response = (PnxXboxResponse *)user_data;
    const size_t incoming = element_size * element_count;
    const size_t available =
        sizeof(response->data) - response->length - 1U;

    if (incoming > available) {
        response->overflow = true;
        return 0U;
    }

    memcpy(response->data + response->length, contents, incoming);
    response->length += incoming;
    response->data[response->length] = '\0';
    return incoming;
}

static bool perform_xbox_post(
    const char *url,
    const char *request_body,
    PnxXboxResponse *response,
    long *http_status,
    char *error,
    size_t error_capacity)
{
    CURL *curl;
    CURLcode result;
    struct curl_slist *headers = NULL;
    struct curl_slist *next_headers;

    if (url == NULL || request_body == NULL || response == NULL ||
        http_status == NULL || error == NULL || error_capacity == 0U) {
        return false;
    }

    memset(response, 0, sizeof(*response));
    *http_status = 0L;
    error[0] = '\0';

    curl = curl_easy_init();
    if (curl == NULL) {
        (void)snprintf(
            error,
            error_capacity,
            "%s",
            "Nao foi possivel criar a requisicao Xbox.");
        return false;
    }

    next_headers = curl_slist_append(
        headers,
        "Content-Type: application/json");
    if (next_headers != NULL) {
        headers = next_headers;
        next_headers = curl_slist_append(headers, "Accept: application/json");
    }
    if (next_headers != NULL) {
        headers = next_headers;
        next_headers = curl_slist_append(
            headers,
            "x-xbl-contract-version: 1");
    }
    if (next_headers == NULL) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        (void)snprintf(
            error,
            error_capacity,
            "%s",
            "Nao foi possivel preparar os cabecalhos Xbox.");
        return false;
    }
    headers = next_headers;

    (void)curl_easy_setopt(curl, CURLOPT_URL, url);
    (void)curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    (void)curl_easy_setopt(curl, CURLOPT_POST, 1L);
    (void)curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body);
    (void)curl_easy_setopt(
        curl,
        CURLOPT_POSTFIELDSIZE,
        (long)strlen(request_body));
    (void)curl_easy_setopt(
        curl,
        CURLOPT_USERAGENT,
        "ProjectNX/" PROJECTNX_VERSION " (Nintendo Switch; libnx)");
    (void)curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, collect_response);
    (void)curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    (void)curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    (void)curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    (void)curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8L);
    (void)curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    (void)curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    (void)curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    (void)curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    result = curl_easy_perform(curl);
    if (result == CURLE_OK) {
        (void)curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, http_status);
    } else {
        (void)snprintf(
            error,
            error_capacity,
            "HTTPS do Xbox falhou: %s",
            curl_easy_strerror(result));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (response->overflow) {
        (void)snprintf(
            error,
            error_capacity,
            "%s",
            "Resposta Xbox maior que o limite seguro.");
        return false;
    }
    return result == CURLE_OK;
}

static void set_service_error(
    PnxXboxStatus *status,
    const char *response)
{
    char message[PNX_XBOX_DETAIL_CAPACITY];
    long xbox_error = 0L;

    if (pnx_json_get_long(response, "XErr", &xbox_error)) {
        status->xbox_error = xbox_error;
    }
    if (pnx_json_get_string(
            response,
            "Message",
            message,
            sizeof(message)) &&
        message[0] != '\0') {
        set_detail(status, message);
        return;
    }
    if (xbox_error != 0L) {
        (void)snprintf(
            status->detail,
            sizeof(status->detail),
            "Xbox recusou a conta (XErr %ld).",
            xbox_error);
        return;
    }
    set_detail(status, "Xbox recusou a autenticacao da conta.");
}

void pnx_xbox_init(PnxXboxStatus *status)
{
    if (status == NULL) {
        return;
    }
    memset(status, 0, sizeof(*status));
    status->stage = PNX_XBOX_IDLE;
    set_detail(status, "Xbox ainda nao conectado.");
}

void pnx_xbox_reset(PnxXboxStatus *status)
{
    if (status == NULL) {
        return;
    }
    clear_credentials(status);
    pnx_xbox_init(status);
}

bool pnx_xbox_authenticate_user(
    const PnxAuthStatus *microsoft_auth,
    PnxXboxStatus *status)
{
    char escaped_access_token[PNX_XBOX_ESCAPED_TOKEN_CAPACITY];
    char request_body[PNX_XBOX_REQUEST_CAPACITY];
    char transport_error[PNX_XBOX_DETAIL_CAPACITY];
    PnxXboxResponse response;
    int written;

    if (microsoft_auth == NULL || status == NULL ||
        microsoft_auth->stage != PNX_AUTH_AUTHENTICATED ||
        microsoft_auth->access_token[0] == '\0') {
        if (status != NULL) {
            status->stage = PNX_XBOX_FAILED;
            set_detail(status, "Token Microsoft para Xbox nao esta disponivel.");
        }
        return false;
    }

    pnx_xbox_reset(status);
    status->stage = PNX_XBOX_USER_CONNECTING;
    set_detail(status, "Trocando autorizacao Microsoft por Xbox User Token...");

    if (!pnx_json_escape_string(
            microsoft_auth->access_token,
            escaped_access_token,
            sizeof(escaped_access_token))) {
        secure_clear(escaped_access_token, sizeof(escaped_access_token));
        status->stage = PNX_XBOX_FAILED;
        set_detail(status, "Token Microsoft maior que o limite Xbox.");
        return false;
    }

    written = snprintf(
        request_body,
        sizeof(request_body),
        "{"
        "\"RelyingParty\":\"http://auth.xboxlive.com\","
        "\"TokenType\":\"JWT\","
        "\"Properties\":{"
        "\"AuthMethod\":\"RPS\","
        "\"SiteName\":\"user.auth.xboxlive.com\","
        "\"RpsTicket\":\"d=%s\""
        "}"
        "}",
        escaped_access_token);
    secure_clear(escaped_access_token, sizeof(escaped_access_token));

    if (written < 0 || (size_t)written >= sizeof(request_body)) {
        secure_clear(request_body, sizeof(request_body));
        status->stage = PNX_XBOX_FAILED;
        set_detail(status, "Requisicao Xbox maior que o limite.");
        return false;
    }

    if (!perform_xbox_post(
            PNX_XBOX_USER_AUTH_URL,
            request_body,
            &response,
            &status->http_status,
            transport_error,
            sizeof(transport_error))) {
        secure_clear(request_body, sizeof(request_body));
        secure_clear(response.data, sizeof(response.data));
        clear_credentials(status);
        status->stage = PNX_XBOX_FAILED;
        set_detail(status, transport_error);
        return false;
    }
    secure_clear(request_body, sizeof(request_body));

    if (status->http_status != 200L) {
        clear_credentials(status);
        status->stage = PNX_XBOX_FAILED;
        set_service_error(status, response.data);
        secure_clear(response.data, sizeof(response.data));
        return false;
    }

    if (!pnx_json_get_string(
            response.data,
            "Token",
            status->user_token,
            sizeof(status->user_token)) ||
        !pnx_json_get_string(
            response.data,
            "uhs",
            status->user_hash,
            sizeof(status->user_hash))) {
        secure_clear(response.data, sizeof(response.data));
        clear_credentials(status);
        status->stage = PNX_XBOX_FAILED;
        set_detail(status, "Resposta Xbox incompleta.");
        return false;
    }

    secure_clear(response.data, sizeof(response.data));
    status->stage = PNX_XBOX_USER_AUTHENTICATED;
    set_detail(status, "Xbox User Token obtido.");
    return true;
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
        clear_xsts_result(status);
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

    clear_xsts_result(status);
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

bool pnx_xbox_authorize_xsts(PnxXboxStatus *status)
{
    char escaped_user_token[PNX_XBOX_ESCAPED_TOKEN_CAPACITY];
    char request_body[PNX_XBOX_REQUEST_CAPACITY];
    char transport_error[PNX_XBOX_DETAIL_CAPACITY];
    PnxXboxResponse response;
    int written;

    if (status == NULL ||
        status->stage != PNX_XBOX_USER_AUTHENTICATED ||
        status->user_token[0] == '\0') {
        if (status != NULL) {
            status->stage = PNX_XBOX_FAILED;
            set_detail(status, "Xbox User Token para XSTS nao esta disponivel.");
        }
        return false;
    }

    clear_xsts_result(status);
    status->http_status = 0L;
    status->xbox_error = 0L;
    status->stage = PNX_XBOX_XSTS_CONNECTING;
    set_detail(status, "Solicitando autorizacao XSTS no ambiente RETAIL...");

    if (!pnx_json_escape_string(
            status->user_token,
            escaped_user_token,
            sizeof(escaped_user_token))) {
        secure_clear(escaped_user_token, sizeof(escaped_user_token));
        clear_credentials(status);
        status->stage = PNX_XBOX_FAILED;
        set_detail(status, "Xbox User Token maior que o limite XSTS.");
        return false;
    }

    written = snprintf(
        request_body,
        sizeof(request_body),
        "{"
        "\"Properties\":{"
        "\"SandboxId\":\"" PNX_XBOX_SANDBOX "\","
        "\"UserTokens\":[\"%s\"]"
        "},"
        "\"RelyingParty\":\"" PNX_XBOX_XSTS_RELYING_PARTY "\","
        "\"TokenType\":\"JWT\""
        "}",
        escaped_user_token);
    secure_clear(escaped_user_token, sizeof(escaped_user_token));

    if (written < 0 || (size_t)written >= sizeof(request_body)) {
        secure_clear(request_body, sizeof(request_body));
        clear_credentials(status);
        status->stage = PNX_XBOX_FAILED;
        set_detail(status, "Requisicao XSTS maior que o limite.");
        return false;
    }

    if (!perform_xbox_post(
            PNX_XBOX_XSTS_AUTH_URL,
            request_body,
            &response,
            &status->http_status,
            transport_error,
            sizeof(transport_error))) {
        secure_clear(request_body, sizeof(request_body));
        secure_clear(response.data, sizeof(response.data));
        clear_credentials(status);
        status->stage = PNX_XBOX_FAILED;
        set_detail(status, transport_error);
        return false;
    }
    secure_clear(request_body, sizeof(request_body));

    if (status->http_status != 200L) {
        secure_clear(status->user_token, sizeof(status->user_token));
        clear_xsts_result(status);
        status->stage = PNX_XBOX_FAILED;
        set_service_error(status, response.data);
        secure_clear(response.data, sizeof(response.data));
        return false;
    }

    if (!pnx_xbox_parse_xsts_response(response.data, status)) {
        secure_clear(response.data, sizeof(response.data));
        clear_credentials(status);
        status->stage = PNX_XBOX_FAILED;
        set_detail(status, "Resposta XSTS incompleta.");
        return false;
    }

    secure_clear(response.data, sizeof(response.data));
    secure_clear(status->user_token, sizeof(status->user_token));
    status->stage = PNX_XBOX_XSTS_AUTHENTICATED;
    if (status->gamertag[0] != '\0') {
        (void)snprintf(
            status->detail,
            sizeof(status->detail),
            "XSTS confirmou o perfil %s.",
            status->gamertag);
    } else {
        set_detail(status, "XSTS confirmou a identidade Xbox.");
    }
    return true;
}

const char *pnx_xbox_stage_name(PnxXboxStage stage)
{
    static const char *const names[] = {
        "IDLE",
        "USER_CONNECTING",
        "USER_AUTHENTICATED",
        "XSTS_CONNECTING",
        "XSTS_AUTHENTICATED",
        "FAILED"
    };

    if (stage < PNX_XBOX_IDLE || stage > PNX_XBOX_FAILED) {
        return "INVALID";
    }
    return names[stage];
}
