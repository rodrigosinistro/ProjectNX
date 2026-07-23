#include "projectnx/app.h"

#include <stdio.h>
#include <string.h>

static void pnx_transition(PnxApp *app, PnxState next)
{
    if (app == NULL || !pnx_app_can_transition(app->state, next)) {
        return;
    }

    app->previous_state = app->state;
    app->state = next;
    app->transition_count++;
}

void pnx_app_init(PnxApp *app, bool docked)
{
    if (app == NULL) {
        return;
    }

    memset(app, 0, sizeof(*app));
    app->state = PNX_STATE_BOOT;
    app->previous_state = PNX_STATE_BOOT;
    app->docked = docked;
    pnx_transition(app, PNX_STATE_WELCOME);
}

void pnx_app_dispatch(PnxApp *app, PnxAction action)
{
    if (app == NULL || action == PNX_ACTION_NONE) {
        return;
    }

    if (action == PNX_ACTION_EXIT) {
        pnx_transition(app, PNX_STATE_EXITING);
        return;
    }

    if (action == PNX_ACTION_TOGGLE_DEBUG) {
        app->debug_visible = !app->debug_visible;
        return;
    }

    if (action == PNX_ACTION_BACK) {
        switch (app->state) {
        case PNX_STATE_AUTH_REQUIRED:
            pnx_transition(app, PNX_STATE_WELCOME);
            break;
        case PNX_STATE_AUTH_WAITING:
            pnx_transition(app, PNX_STATE_AUTH_REQUIRED);
            break;
        case PNX_STATE_CATALOG:
            pnx_transition(app, PNX_STATE_AUTH_REQUIRED);
            break;
        case PNX_STATE_STREAM_CONNECTING:
            pnx_transition(app, PNX_STATE_CATALOG);
            break;
        case PNX_STATE_STREAMING:
            pnx_transition(app, PNX_STATE_CATALOG);
            break;
        case PNX_STATE_ERROR:
            pnx_transition(app, app->previous_state);
            break;
        default:
            break;
        }
        return;
    }

    if (action != PNX_ACTION_CONFIRM) {
        return;
    }

    switch (app->state) {
    case PNX_STATE_WELCOME:
        pnx_transition(app, PNX_STATE_AUTH_REQUIRED);
        break;
    case PNX_STATE_AUTH_REQUIRED:
        pnx_transition(app, PNX_STATE_AUTH_WAITING);
        break;
    case PNX_STATE_AUTH_WAITING:
        pnx_transition(app, PNX_STATE_CATALOG);
        break;
    case PNX_STATE_CATALOG:
        pnx_transition(app, PNX_STATE_STREAM_CONNECTING);
        break;
    case PNX_STATE_STREAM_CONNECTING:
        pnx_transition(app, PNX_STATE_STREAMING);
        break;
    case PNX_STATE_STREAMING:
        pnx_transition(app, PNX_STATE_CATALOG);
        break;
    case PNX_STATE_ERROR:
        pnx_transition(app, app->previous_state);
        break;
    default:
        break;
    }
}

void pnx_app_set_error(PnxApp *app, const char *message)
{
    if (app == NULL) {
        return;
    }

    if (message == NULL || message[0] == '\0') {
        message = "Erro desconhecido";
    }

    (void)snprintf(app->error_message, sizeof(app->error_message), "%s", message);
    pnx_transition(app, PNX_STATE_ERROR);
}

bool pnx_app_can_transition(PnxState from, PnxState to)
{
    if (from < PNX_STATE_BOOT || from >= PNX_STATE_COUNT ||
        to < PNX_STATE_BOOT || to >= PNX_STATE_COUNT || from == to) {
        return false;
    }

    if (to == PNX_STATE_EXITING) {
        return from != PNX_STATE_EXITING;
    }

    switch (from) {
    case PNX_STATE_BOOT:
        return to == PNX_STATE_WELCOME || to == PNX_STATE_ERROR;
    case PNX_STATE_WELCOME:
        return to == PNX_STATE_AUTH_REQUIRED || to == PNX_STATE_ERROR;
    case PNX_STATE_AUTH_REQUIRED:
        return to == PNX_STATE_WELCOME || to == PNX_STATE_AUTH_WAITING ||
               to == PNX_STATE_ERROR;
    case PNX_STATE_AUTH_WAITING:
        return to == PNX_STATE_AUTH_REQUIRED || to == PNX_STATE_CATALOG ||
               to == PNX_STATE_ERROR;
    case PNX_STATE_CATALOG:
        return to == PNX_STATE_AUTH_REQUIRED ||
               to == PNX_STATE_STREAM_CONNECTING || to == PNX_STATE_ERROR;
    case PNX_STATE_STREAM_CONNECTING:
        return to == PNX_STATE_CATALOG || to == PNX_STATE_STREAMING ||
               to == PNX_STATE_ERROR;
    case PNX_STATE_STREAMING:
        return to == PNX_STATE_CATALOG || to == PNX_STATE_ERROR;
    case PNX_STATE_ERROR:
        return to != PNX_STATE_BOOT && to != PNX_STATE_ERROR;
    case PNX_STATE_EXITING:
    case PNX_STATE_COUNT:
        return false;
    }

    return false;
}

const char *pnx_state_name(PnxState state)
{
    static const char *const names[PNX_STATE_COUNT] = {
        "BOOT",
        "WELCOME",
        "AUTH_REQUIRED",
        "AUTH_WAITING",
        "CATALOG",
        "STREAM_CONNECTING",
        "STREAMING",
        "ERROR",
        "EXITING"
    };

    if (state < PNX_STATE_BOOT || state >= PNX_STATE_COUNT) {
        return "INVALID";
    }
    return names[state];
}

const char *pnx_state_title(PnxState state)
{
    static const char *const titles[PNX_STATE_COUNT] = {
        "Inicializando",
        "Bem-vindo ao ProjectNX",
        "Conectar conta Microsoft",
        "Aguardando autorizacao",
        "Catalogo de jogos",
        "Preparando transmissao",
        "Transmissao ativa",
        "Ocorreu um erro",
        "Encerrando"
    };

    if (state < PNX_STATE_BOOT || state >= PNX_STATE_COUNT) {
        return "Estado invalido";
    }
    return titles[state];
}

const char *pnx_state_description(PnxState state)
{
    static const char *const descriptions[PNX_STATE_COUNT] = {
        "Carregando os servicos essenciais.",
        "Preview tecnico da interface nativa.",
        "A versao futura usara login por codigo de dispositivo.",
        "Simulacao: confirme para concluir o login.",
        "Simulacao: um jogo demonstrativo esta selecionado.",
        "Simulacao: negociando a sessao WebRTC.",
        "Simulacao: video, audio e controles conectados.",
        "Use A ou B para retornar ao estado anterior.",
        "Salvando configuracoes e liberando recursos."
    };

    if (state < PNX_STATE_BOOT || state >= PNX_STATE_COUNT) {
        return "Estado fora do intervalo.";
    }
    return descriptions[state];
}

