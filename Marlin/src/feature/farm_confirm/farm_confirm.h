/**
 * Marlin 3D Printer Firmware - Farm Mode Extension
 *
 * farm_confirm.h
 *
 * Adds a mandatory physical confirmation step after a print job ends
 * (either normally or aborted/failed), so the printer is only reported
 * back to the farm-management host as available once an operator has
 * physically inspected the part and chosen the outcome on the LCD.
 *
 * Unlike a plain acknowledgment click, the operator is always presented
 * with a real Yes/No menu (Marlin's native `MenuItem_confirm` dialog) and
 * picks the actual result themselves — the firmware never assumes a
 * normal SD-print finish means the part came out fine, nor that an abort
 * necessarily means it's scrap. The host then receives one of two
 * distinct action strings over serial:
 *
 *   //action:print_ok_confirmed
 *   //action:print_fail_confirmed
 *
 * This does not decide whether the printer returns to the queue or is
 * taken out of service — that decision belongs to the host software.
 *
 * Enable with FARM_MODE_CONFIRM_ON_FINISH in Configuration_adv.h.
 */
#pragma once

#include "../../inc/MarlinConfigPre.h"

#if ENABLED(FARM_MODE_CONFIRM_ON_FINISH)

class FarmConfirm {
  public:
    // Call when a print job ends. Blocks (non-busy, keeps calling idle())
    // until the operator picks an outcome on the LCD Yes/No menu, then
    // reports that choice to the host. `was_ok` distinguishes a normal
    // finish (true) from an abort/error condition (false) — it is only
    // used to pre-select the likely answer on the menu, never to decide
    // the outcome by itself.
    static void notify_and_wait(const bool was_ok);
};

extern FarmConfirm farmConfirm;

#endif // FARM_MODE_CONFIRM_ON_FINISH
