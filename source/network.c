#include "projectnx/network.h"

#include <curl/curl.h>
#include <switch.h>

#include <stdio.h>
#include <string.h>

#ifndef PROJECTNX_VERSION
#define PROJECTNX_VERSION "dev"
#endif

#define PNX_IDENTITY_DISCOVERY_URL \
    "https://login.microsoftonline.com/consumers/v2.0/.well-known/openid-configuration"

static size_t discard_response(
    void *contents,
    size_t element_size,
    size_t element_count,
    void *user_data)
{
    (void)contents;
    (void)user_data;
    return element_size * element_count;
}

void pnx_network_status_init(PnxNetworkStatus *status)
{
    if (status == NULL) {
        return;
    }

    memset(status, 0, sizeof(*status));
    (void)snprintf(
        status->detail,
        sizeof(status->detail),
        "%s",
        "Rede ainda nao verificada.");
}

bool pnx_network_platform_init(PnxNetworkStatus *status)
{
    Result socket_result;
    CURLcode curl_result;

    if (status == NULL) {
        return false;
    }

    socket_result = socketInitializeDefault();
    if (R_FAILED(socket_result)) {
        (void)snprintf(
            status->detail,
            sizeof(status->detail),
            "Falha ao iniciar sockets: 0x%08X",
            socket_result);
        return false;
    }

    curl_result = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (curl_result != CURLE_OK) {
        (void)snprintf(
            status->detail,
            sizeof(status->detail),
            "Falha ao iniciar TLS: %s",
            curl_easy_strerror(curl_result));
        socketExit();
        return false;
    }

    status->initialized = true;
    (void)snprintf(
        status->detail,
        sizeof(status->detail),
        "%s",
        "Sockets e TLS inicializados.");
    return true;
}

bool pnx_network_probe(PnxNetworkStatus *status)
{
    CURL *curl;
    CURLcode result;
    long http_status = 0L;

    if (status == NULL || !status->initialized) {
        return false;
    }

    status->checked = false;
    status->online = false;
    status->http_status = 0L;

    curl = curl_easy_init();
    if (curl == NULL) {
        (void)snprintf(
            status->detail,
            sizeof(status->detail),
            "%s",
            "Nao foi possivel criar a requisicao HTTPS.");
        status->checked = true;
        return false;
    }

    (void)curl_easy_setopt(curl, CURLOPT_URL, PNX_IDENTITY_DISCOVERY_URL);
    (void)curl_easy_setopt(
        curl,
        CURLOPT_USERAGENT,
        "ProjectNX/" PROJECTNX_VERSION " (Nintendo Switch; libnx)");
    (void)curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_response);
    (void)curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    (void)curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3L);
    (void)curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8L);
    (void)curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    (void)curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    (void)curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    (void)curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    result = curl_easy_perform(curl);
    if (result == CURLE_OK) {
        (void)curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status);
    }
    curl_easy_cleanup(curl);

    status->checked = true;
    status->http_status = http_status;
    status->online =
        result == CURLE_OK && http_status >= 200L && http_status < 300L;

    if (result != CURLE_OK) {
        (void)snprintf(
            status->detail,
            sizeof(status->detail),
            "HTTPS falhou: %s",
            curl_easy_strerror(result));
    } else if (!status->online) {
        (void)snprintf(
            status->detail,
            sizeof(status->detail),
            "Microsoft respondeu HTTP %ld.",
            http_status);
    } else {
        (void)snprintf(
            status->detail,
            sizeof(status->detail),
            "Microsoft Identity online (HTTP %ld).",
            http_status);
    }

    return status->online;
}

void pnx_network_platform_exit(PnxNetworkStatus *status)
{
    if (status == NULL || !status->initialized) {
        return;
    }

    curl_global_cleanup();
    socketExit();
    status->initialized = false;
}
