#include "projectnx/auth.h"

#include <curl/curl.h>

#include <stdio.h>
#include <string.h>

#include "projectnx/json.h"

#ifndef PROJECTNX_VERSION
#define PROJECTNX_VERSION "dev"
#endif

#define PNX_AUTH_RESPONSE_CAPACITY 32768U
#define PNX_AUTH_URL_CAPACITY 256U
#define PNX_AUTH_POST_CAPACITY 4096U

typedef struct {
    char data[PNX_AUTH_RESPONSE_CAPACITY];
    size_t length;
    bool overflow;
} PnxAuthResponse;

static void secure_clear(char *value, size_t capacity)
{
    volatile unsigned char *cursor = (volatile unsigned char *)value;

    while (capacity > 0U) {
        *cursor = 0U;
        cursor++;
        capacity--;
    }
}

static void set_detail(PnxAuthStatus *status, const char *detail)
{
    if (status == NULL) {
        return;
    }
    if (detail == NULL || detail[0] == '\0') {
        detail = "Falha de autenticacao sem detalhes.";
    }
    (void)snprintf(status->detail, sizeof(status->detail), "%s", detail);
}

static size_t collect_response(
    void *contents,
    size_t element_size,
    size_t element_count,
    void *user_data)
{
    PnxAuthResponse *response = (PnxAuthResponse *)user_data;
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

static bool perform_post(
    const char *url,
    const char *post_fields,
    PnxAuthResponse *response,
    long *http_status,
    char *error,
    size_t error_capacity)
{
    CURL *curl;
    CURLcode result;
    struct curl_slist *headers = NULL;

    if (url == NULL || post_fields == NULL || response == NULL ||
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
            "Nao foi possivel criar a requisicao de login.");
        return false;
    }

    headers = curl_slist_append(
        headers,
        "Content-Type: application/x-www-form-urlencoded");
    if (headers == NULL) {
        curl_easy_cleanup(curl);
        (void)snprintf(
            error,
            error_capacity,
            "%s",
            "Nao foi possivel preparar a requisicao de login.");
        return false;
    }

    (void)curl_easy_setopt(curl, CURLOPT_URL, url);
    (void)curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    (void)curl_easy_setopt(curl, CURLOPT_POST, 1L);
    (void)curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields);
    (void)curl_easy_setopt(
        curl,
        CURLOPT_POSTFIELDSIZE,
        (long)strlen(post_fields));
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
            "HTTPS do login falhou: %s",
            curl_easy_strerror(result));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (response->overflow) {
        (void)snprintf(
            error,
            error_capacity,
            "%s",
            "Resposta de login maior que o limite seguro.");
        return false;
    }
    return result == CURLE_OK;
}

static void set_service_error(
    PnxAuthStatus *status,
    const char *response,
    const char *fallback)
{
    char description[PNX_AUTH_DETAIL_CAPACITY];
    char code[96];

    if (pnx_json_get_string(
            response,
            "error_description",
            description,
            sizeof(description))) {
        set_detail(status, description);
        return;
    }
    if (pnx_json_get_string(response, "error", code, sizeof(code))) {
        set_detail(status, code);
        return;
    }
    set_detail(status, fallback);
}

void pnx_auth_init(PnxAuthStatus *status)
{
    if (status == NULL) {
        return;
    }
    memset(status, 0, sizeof(*status));
    status->stage = PNX_AUTH_IDLE;
    status->poll_interval_seconds = 5L;
    set_detail(status, "Login ainda nao iniciado.");
}

void pnx_auth_clear_tokens(PnxAuthStatus *status)
{
    if (status == NULL) {
        return;
    }
    secure_clear(status->access_token, sizeof(status->access_token));
    secure_clear(status->refresh_token, sizeof(status->refresh_token));
    secure_clear(status->device_code, sizeof(status->device_code));
}

void pnx_auth_reset(PnxAuthStatus *status)
{
    if (status == NULL) {
        return;
    }
    pnx_auth_clear_tokens(status);
    pnx_auth_init(status);
}

bool pnx_auth_request_device_code(
    const PnxConfig *config,
    PnxAuthStatus *status)
{
    char url[PNX_AUTH_URL_CAPACITY];
    char post_fields[PNX_AUTH_POST_CAPACITY];
    char transport_error[PNX_AUTH_DETAIL_CAPACITY];
    PnxAuthResponse response;
    long expires_in = 900L;
    long interval = 5L;
    int written;

    if (config == NULL || status == NULL || !config->client_id_valid) {
        if (status != NULL) {
            status->stage = PNX_AUTH_FAILED;
            set_detail(status, "client_id do ProjectNX nao esta configurado.");
        }
        return false;
    }

    pnx_auth_reset(status);
    set_detail(status, "Solicitando codigo de dispositivo...");

    written = snprintf(
        url,
        sizeof(url),
        "https://login.microsoftonline.com/%s/oauth2/v2.0/devicecode?mkt=pt-BR",
        config->tenant);
    if (written < 0 || (size_t)written >= sizeof(url)) {
        status->stage = PNX_AUTH_FAILED;
        set_detail(status, "Endereco do Microsoft Identity invalido.");
        return false;
    }

    written = snprintf(
        post_fields,
        sizeof(post_fields),
        "client_id=%s&scope=XboxLive.signin%%20XboxLive.offline_access",
        config->client_id);
    if (written < 0 || (size_t)written >= sizeof(post_fields)) {
        status->stage = PNX_AUTH_FAILED;
        set_detail(status, "Configuracao de login maior que o limite.");
        return false;
    }

    if (!perform_post(
            url,
            post_fields,
            &response,
            &status->http_status,
            transport_error,
            sizeof(transport_error))) {
        status->stage = PNX_AUTH_FAILED;
        set_detail(status, transport_error);
        return false;
    }

    if (status->http_status != 200L) {
        status->stage = PNX_AUTH_FAILED;
        set_service_error(
            status,
            response.data,
            "Microsoft recusou a solicitacao do codigo.");
        return false;
    }

    if (!pnx_json_get_string(
            response.data,
            "device_code",
            status->device_code,
            sizeof(status->device_code)) ||
        !pnx_json_get_string(
            response.data,
            "user_code",
            status->user_code,
            sizeof(status->user_code)) ||
        !pnx_json_get_string(
            response.data,
            "verification_uri",
            status->verification_uri,
            sizeof(status->verification_uri))) {
        status->stage = PNX_AUTH_FAILED;
        set_detail(status, "Resposta de login incompleta.");
        return false;
    }

    (void)pnx_json_get_long(response.data, "expires_in", &expires_in);
    (void)pnx_json_get_long(response.data, "interval", &interval);
    if (expires_in < 60L || expires_in > 3600L) {
        expires_in = 900L;
    }
    if (interval < 5L || interval > 60L) {
        interval = 5L;
    }

    status->expires_in_seconds = expires_in;
    status->poll_interval_seconds = interval;
    status->issued_at = time(NULL);
    status->next_poll_at = status->issued_at + interval;
    status->stage = PNX_AUTH_CODE_READY;
    set_detail(status, "Codigo criado. Conclua o login no celular ou PC.");
    return true;
}

bool pnx_auth_poll_due(const PnxAuthStatus *status, time_t now)
{
    if (status == NULL ||
        (status->stage != PNX_AUTH_CODE_READY &&
         status->stage != PNX_AUTH_PENDING)) {
        return false;
    }
    return now >= status->next_poll_at;
}

PnxAuthPollResult pnx_auth_poll_token(
    const PnxConfig *config,
    PnxAuthStatus *status,
    time_t now)
{
    char url[PNX_AUTH_URL_CAPACITY];
    char post_fields[PNX_AUTH_POST_CAPACITY];
    char transport_error[PNX_AUTH_DETAIL_CAPACITY];
    char error_code[96];
    char *escaped_device_code;
    PnxAuthResponse response;
    CURL *encoder;
    bool has_access_token;
    int written;

    if (!pnx_auth_poll_due(status, now)) {
        return PNX_AUTH_POLL_NOT_DUE;
    }
    if (config == NULL || !config->client_id_valid) {
        status->stage = PNX_AUTH_FAILED;
        set_detail(status, "client_id ficou indisponivel durante o login.");
        return PNX_AUTH_POLL_FATAL;
    }
    if (now >= status->issued_at + status->expires_in_seconds) {
        status->stage = PNX_AUTH_EXPIRED;
        set_detail(status, "O codigo expirou. Inicie o login novamente.");
        return PNX_AUTH_POLL_FATAL;
    }

    encoder = curl_easy_init();
    if (encoder == NULL) {
        status->stage = PNX_AUTH_FAILED;
        set_detail(status, "Nao foi possivel preparar o codigo do dispositivo.");
        return PNX_AUTH_POLL_FATAL;
    }
    escaped_device_code =
        curl_easy_escape(encoder, status->device_code, 0);
    if (escaped_device_code == NULL) {
        curl_easy_cleanup(encoder);
        status->stage = PNX_AUTH_FAILED;
        set_detail(status, "Nao foi possivel codificar o codigo do dispositivo.");
        return PNX_AUTH_POLL_FATAL;
    }

    written = snprintf(
        url,
        sizeof(url),
        "https://login.microsoftonline.com/%s/oauth2/v2.0/token",
        config->tenant);
    if (written < 0 || (size_t)written >= sizeof(url)) {
        curl_free(escaped_device_code);
        curl_easy_cleanup(encoder);
        status->stage = PNX_AUTH_FAILED;
        set_detail(status, "Endereco de confirmacao invalido.");
        return PNX_AUTH_POLL_FATAL;
    }

    written = snprintf(
        post_fields,
        sizeof(post_fields),
        "grant_type=urn%%3Aietf%%3Aparams%%3Aoauth%%3Agrant-type%%3Adevice_code"
        "&client_id=%s&device_code=%s",
        config->client_id,
        escaped_device_code);
    curl_free(escaped_device_code);
    curl_easy_cleanup(encoder);

    if (written < 0 || (size_t)written >= sizeof(post_fields)) {
        status->stage = PNX_AUTH_FAILED;
        set_detail(status, "Requisicao de confirmacao maior que o limite.");
        return PNX_AUTH_POLL_FATAL;
    }

    if (!perform_post(
            url,
            post_fields,
            &response,
            &status->http_status,
            transport_error,
            sizeof(transport_error))) {
        status->stage = PNX_AUTH_FAILED;
        set_detail(status, transport_error);
        return PNX_AUTH_POLL_FATAL;
    }

    if (status->http_status == 200L) {
        has_access_token = pnx_json_get_string(
            response.data,
            "access_token",
            status->access_token,
            sizeof(status->access_token));
        (void)pnx_json_get_string(
            response.data,
            "refresh_token",
            status->refresh_token,
            sizeof(status->refresh_token));

        if (!has_access_token) {
            secure_clear(response.data, sizeof(response.data));
            status->stage = PNX_AUTH_FAILED;
            set_detail(
                status,
                "Microsoft confirmou sem retornar o token de acesso Xbox.");
            return PNX_AUTH_POLL_FATAL;
        }

        secure_clear(response.data, sizeof(response.data));
        secure_clear(status->device_code, sizeof(status->device_code));
        status->stage = PNX_AUTH_AUTHENTICATED;
        set_detail(status, "Login Microsoft concluido.");
        return PNX_AUTH_POLL_COMPLETE;
    }

    if (!pnx_json_get_string(
            response.data,
            "error",
            error_code,
            sizeof(error_code))) {
        status->stage = PNX_AUTH_FAILED;
        set_service_error(
            status,
            response.data,
            "Resposta inesperada durante a confirmacao.");
        return PNX_AUTH_POLL_FATAL;
    }

    if (strcmp(error_code, "authorization_pending") == 0) {
        status->stage = PNX_AUTH_PENDING;
        status->next_poll_at = now + status->poll_interval_seconds;
        set_detail(status, "Aguardando a confirmacao da conta...");
        return PNX_AUTH_POLL_PENDING;
    }
    if (strcmp(error_code, "slow_down") == 0) {
        status->stage = PNX_AUTH_PENDING;
        status->poll_interval_seconds += 5L;
        if (status->poll_interval_seconds > 60L) {
            status->poll_interval_seconds = 60L;
        }
        status->next_poll_at = now + status->poll_interval_seconds;
        set_detail(status, "Microsoft pediu para aguardar um pouco mais...");
        return PNX_AUTH_POLL_PENDING;
    }
    if (strcmp(error_code, "authorization_declined") == 0) {
        status->stage = PNX_AUTH_DECLINED;
        set_detail(status, "O login foi cancelado no navegador.");
        return PNX_AUTH_POLL_FATAL;
    }
    if (strcmp(error_code, "expired_token") == 0) {
        status->stage = PNX_AUTH_EXPIRED;
        set_detail(status, "O codigo expirou. Inicie o login novamente.");
        return PNX_AUTH_POLL_FATAL;
    }

    status->stage = PNX_AUTH_FAILED;
    set_service_error(
        status,
        response.data,
        "Microsoft recusou a confirmacao do login.");
    return PNX_AUTH_POLL_FATAL;
}

const char *pnx_auth_stage_name(PnxAuthStage stage)
{
    static const char *const names[] = {
        "IDLE",
        "CODE_READY",
        "PENDING",
        "AUTHENTICATED",
        "DECLINED",
        "EXPIRED",
        "FAILED"
    };

    if (stage < PNX_AUTH_IDLE || stage > PNX_AUTH_FAILED) {
        return "INVALID";
    }
    return names[stage];
}
