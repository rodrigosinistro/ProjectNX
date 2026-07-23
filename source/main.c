#include <switch.h>

#include <stdbool.h>
#include <stdio.h>

#include "projectnx/app.h"
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
    const PnxConfig *config)
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

    printf("\n------------------------------------------------------------\n");
    printf("[A] Avancar   [B] Voltar   [X] Diagnostico   [+] Sair\n");

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
    PnxConfig config;
    PnxNetworkStatus network;

    pnx_app_init(&app, docked);
    pnx_config_init(&config);
    (void)pnx_config_load(
        &config,
        "sdmc:/switch/projectnx/config.ini");
    pnx_network_status_init(&network);
    (void)pnx_network_platform_init(&network);

    while (appletMainLoop() && app.state != PNX_STATE_EXITING) {
        padUpdate(&pad);
        pnx_app_dispatch(&app, read_action(&pad));

        if (app.state == PNX_STATE_NETWORK_CHECK) {
            draw(&app, &network, &config);
            consoleUpdate(NULL);

            if (pnx_network_probe(&network)) {
                pnx_app_dispatch(&app, PNX_ACTION_NETWORK_READY);
            } else {
                pnx_app_set_error(&app, network.detail);
            }
        }

        draw(&app, &network, &config);
        consoleUpdate(NULL);
        svcSleepThread(50000000L);
    }

    pnx_network_platform_exit(&network);
    consoleExit(NULL);
    return 0;
}
