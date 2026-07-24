#include <switch.h>

#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "projectnx/app.h"
#include "projectnx/auth.h"
#include "projectnx/config.h"
#include "projectnx/network.h"

#ifndef PROJECTNX_VERSION
#define PROJECTNX_VERSION "dev"
#endif

static PnxAction read_action(PadState *pad)
{
    const u64 down = padGetButtonsDown(pad);

    if ((down & HidNpadButton_Plus) != 0U) {
        return PNX_ACTION_EXIT;
    }
    if ((down & HidNpadButton_A) != 0U) {
        return PNX_ACTION_CONFIRM;
    }
    if ((down & HidNpadButton_B) != 0U) {
        return PNX_ACTION_BACK;
    }
    if ((down & HidNpadButton_X) != 0U) {
        return PNX_ACTION_TOGGLE_DEBUG;
    }
    return PNX_ACTION_NONE;
}

static void draw(
    const PnxApp *app,
    const PnxNetworkStatus *network,
    const PnxConfig *config,
    const PnxAuthStatus *auth)
{
    printf("\x1b[2J\x1b[H");
    printf("\x1b[36;1mProjectNX\x1b[0m  v%s-preview\n", PROJECTNX_VERSION);
    printf("Cliente nativo de jogos por nuvem\n");
    printf("============================================================\n\n");
    printf("\x1b[37;1m%s\x1b[0m\n\n", pnx_state_title(app->state));

    if (app->state == PNX_STATE_ERROR && app->error_message[0] != '\0') {
        printf("\x1b[31m%s\x1b[0m\n", app->error_message);
    } else {
        printf("%s\n", pnx_state_description(app->state));
    }

    if (app->state == PNX_STATE_AUTH_REQUIRED && network->online) {
        printf("\n\x1b[32;1mRede segura: OK\x1b[0m\n");
        printf("%s\n", network->detail);
        if (config->client_id_valid) {
            printf("Aplicativo Microsoft: configurado\n");
        } else {
            printf("Aplicativo Microsoft: client_id ainda nao configurado\n");
        }
    }

    if (app->state == PNX_STATE_AUTH_WAITING &&
        (auth->stage == PNX_AUTH_CODE_READY ||
         auth->stage == PNX_AUTH_PENDING)) {
        printf("\nAcesse no celular ou computador:\n");
        printf("\x1b[36;1m%s\x1b[0m\n", auth->verification_uri);
        printf("\nDigite o codigo:\n");
        printf("\x1b[32;1m%s\x1b[0m\n", auth->user_code);
        printf("\n%s\n", auth->detail);
    }

    if (app->state == PNX_STATE_CATALOG &&
        auth->stage == PNX_AUTH_AUTHENTICATED) {
        printf("\n\x1b[32;1mConta Microsoft conectada.\x1b[0m\n");
        printf("Nenhum token foi gravado no cartao SD.\n");
    }

    printf("\n------------------------------------------------------------\n");
    if (app->state == PNX_STATE_AUTH_WAITING) {
        printf("[B] Cancelar   [X] Diagnostico   [+] Sair\n");
    } else {
        printf("[A] Avancar   [B] Voltar   [X] Diagnostico   [+] Sair\n");
    }

    if (app->debug_visible) {
        printf("\n\x1b[33;1mDiagnostico\x1b[0m\n");
        printf("Estado: %s\n", pnx_state_name(app->state));
        printf("Modo: %s\n", app->docked ? "Dock" : "Portatil");
        printf("Transicoes: %lu\n", (unsigned long)app->transition_count);
        printf("Memoria: title mode recomendado\n");
        printf(
            "Rede: %s",
            network->online ? "online" :
            (network->checked ? "offline" : "nao verificada"));
        if (network->http_status > 0L) {
            printf(" (HTTP %ld)", network->http_status);
        }
        printf("\n");
        printf(
            "Config: %s | client_id: %s\n",
            config->file_found ? "carregada" : "padrao",
            config->client_id_valid ? "valido" : "pendente");
        printf(
            "Login: %s | HTTP: %ld\n",
            pnx_auth_stage_name(auth->stage),
            auth->http_status);
    }
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    consoleInit(NULL);
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeDefault(&pad);

    const bool docked = appletGetOperationMode() == AppletOperationMode_Console;
    PnxApp app;
    PnxAuthStatus auth;
    PnxConfig config;
    PnxNetworkStatus network;

    pnx_app_init(&app, docked);
    pnx_auth_init(&auth);
    pnx_config_init(&config);
    (void)pnx_config_load(
        &config,
        "sdmc:/switch/projectnx/config.ini");
    pnx_network_status_init(&network);
    (void)pnx_network_platform_init(&network);

    while (appletMainLoop() && app.state != PNX_STATE_EXITING) {
        PnxAction action;

        padUpdate(&pad);
        action = read_action(&pad);

        if (action == PNX_ACTION_CONFIRM &&
            app.state == PNX_STATE_AUTH_REQUIRED) {
            draw(&app, &network, &config, &auth);
            consoleUpdate(NULL);
            if (pnx_auth_request_device_code(&config, &auth)) {
                pnx_app_dispatch(&app, PNX_ACTION_CONFIRM);
            } else {
                pnx_app_set_error(&app, auth.detail);
            }
        } else {
            if (action == PNX_ACTION_BACK &&
                (app.state == PNX_STATE_AUTH_WAITING ||
                 app.state == PNX_STATE_CATALOG)) {
                pnx_auth_reset(&auth);
            }
            pnx_app_dispatch(&app, action);
        }

        if (app.state == PNX_STATE_NETWORK_CHECK) {
            draw(&app, &network, &config, &auth);
            consoleUpdate(NULL);

            if (pnx_network_probe(&network)) {
                pnx_app_dispatch(&app, PNX_ACTION_NETWORK_READY);
            } else {
                pnx_app_set_error(&app, network.detail);
            }
        }

        if (app.state == PNX_STATE_AUTH_WAITING &&
            pnx_auth_poll_due(&auth, time(NULL))) {
            const PnxAuthPollResult poll_result =
                pnx_auth_poll_token(&config, &auth, time(NULL));

            if (poll_result == PNX_AUTH_POLL_COMPLETE) {
                pnx_app_dispatch(&app, PNX_ACTION_AUTH_COMPLETE);
            } else if (poll_result == PNX_AUTH_POLL_FATAL) {
                char error[PNX_ERROR_MESSAGE_CAPACITY];
                (void)snprintf(
                    error,
                    sizeof(error),
                    "%.*s",
                    (int)sizeof(error) - 1,
                    auth.detail);
                pnx_auth_reset(&auth);
                pnx_app_dispatch(&app, PNX_ACTION_BACK);
                pnx_app_set_error(&app, error);
            }
        }

        draw(&app, &network, &config, &auth);
        consoleUpdate(NULL);
        svcSleepThread(50000000L);
    }

    pnx_auth_reset(&auth);
    pnx_network_platform_exit(&network);
    consoleExit(NULL);
    return 0;
}
