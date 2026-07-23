#include <switch.h>

#include <stdbool.h>
#include <stdio.h>

#include "projectnx/app.h"

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

static void draw(const PnxApp *app)
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

    printf("\n------------------------------------------------------------\n");
    printf("[A] Avancar   [B] Voltar   [X] Diagnostico   [+] Sair\n");

    if (app->debug_visible) {
        printf("\n\x1b[33;1mDiagnostico\x1b[0m\n");
        printf("Estado: %s\n", pnx_state_name(app->state));
        printf("Modo: %s\n", app->docked ? "Dock" : "Portatil");
        printf("Transicoes: %lu\n", (unsigned long)app->transition_count);
        printf("Memoria: title mode recomendado\n");
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
    pnx_app_init(&app, docked);

    while (appletMainLoop() && app.state != PNX_STATE_EXITING) {
        padUpdate(&pad);
        pnx_app_dispatch(&app, read_action(&pad));
        draw(&app);
        consoleUpdate(NULL);
        svcSleepThread(50000000L);
    }

    consoleExit(NULL);
    return 0;
}
