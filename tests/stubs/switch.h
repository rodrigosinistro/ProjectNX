#ifndef PROJECTNX_TEST_SWITCH_H
#define PROJECTNX_TEST_SWITCH_H

#include <stdbool.h>
#include <stdint.h>

typedef uint64_t u64;

typedef struct {
    uint32_t reserved;
} PadState;

typedef enum {
    AppletOperationMode_Handheld = 0,
    AppletOperationMode_Console = 1
} AppletOperationMode;

enum {
    HidNpadStyleSet_NpadStandard = 1U
};

enum {
    HidNpadButton_A = 1U << 0,
    HidNpadButton_B = 1U << 1,
    HidNpadButton_X = 1U << 2,
    HidNpadButton_Plus = 1U << 3
};

void consoleInit(void *console);
void consoleExit(void *console);
void consoleUpdate(void *console);
void padConfigureInput(uint32_t max_players, uint32_t style_set);
void padInitializeDefault(PadState *pad);
void padUpdate(PadState *pad);
u64 padGetButtonsDown(const PadState *pad);
bool appletMainLoop(void);
AppletOperationMode appletGetOperationMode(void);
void svcSleepThread(int64_t nanoseconds);

#endif
