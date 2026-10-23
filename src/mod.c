#include "modding.h"

#include <ultra64.h>
#include <defines.h>
#include "common_structs.h"
#include "main.h"
#include "menus.h"
#include "code_800029B0.h"
#include "audio/external.h"

// N64 Z + R, which is LB + RB (or LT + RT) with the default Xbox-style mapping.
#define RESET_COMBO_HELD (Z_TRIG | R_TRIG)
#define RESET_COMBO_PRESS START_BUTTON

// Placeholder gamestate the game itself uses (see func_802A38B4) to force update_gamestate() on the next frame.
#define GAMESTATE_FORCE_RELOAD 255

static bool reset_combo_pressed(void) {
    s32 i;

    // Only physical controllers 1-4, so the combo has to be done on a single pad.
    for (i = 0; i < 4; i++) {
        struct Controller* controller = &gControllers[i];

        if (((controller->button & RESET_COMBO_HELD) == RESET_COMBO_HELD) &&
            (controller->buttonPressed & RESET_COMBO_PRESS)) {
            return true;
        }
    }
    return false;
}

// Soft reset back to the Nintendo logo, mirroring the game's own "quit to start menu" path (func_802A38B4)
// but targeting the logo intro like the demo timeout does (func_8028FC34).
static void soft_reset(void) {
    // Stop music and sound effects.
    func_800CA330(0x19);
    func_800CA008(0, 0);

    gIsGamePaused = 0;
    gIsInQuitToMenuTransition = 0;
    gQuitToMenuTransitionCounter = 0;
    gDemoMode = DEMO_MODE_INACTIVE;
    gDemoUseController = 0;

    gMenuSelection = LOGO_INTRO_MENU;
    gGamestateNext = START_MENU_FROM_QUIT;
    gGamestate = GAMESTATE_FORCE_RELOAD;
}

// Runs once per frame after read_controllers(). Setting gGamestate to the placeholder here makes this
// frame's game_state_handler() a no-op, so the Start press isn't also handled as pause/menu input.
RECOMP_HOOK("game_state_handler") void on_game_state_handler(void) {
    if (reset_combo_pressed()) {
        soft_reset();
    }
}
