/**
 * Marlin 3D Printer Firmware - Farm Mode Extension
 *
 * farm_confirm.cpp
 *
 * See farm_confirm.h for the rationale. Implementation notes:
 *
 * - Reuses Marlin's native Yes/No confirmation dialog (`MenuItem_confirm`,
 *   the same one behind "Stop print?"/"Shutdown?" in menu_main.cpp) so the
 *   operator sees a proper centered screen with two real options, instead
 *   of a one-line status message that only asked for an acknowledgment.
 * - `select_screen()` is not blocking by itself — it's meant to be called
 *   once per redraw from the active screen function. `ui.goto_screen()`
 *   points the UI at `pantalla_confirmacion()` so `idle()` keeps calling
 *   it on every loop tick, which draws the dialog and reacts to the
 *   encoder; a local `resultado` flag is what actually blocks this
 *   function until one of the two options is clicked.
 * - The wait loop has no timeout: the machine stays in this state
 *   indefinitely until a human acts, which is the whole point — we
 *   never want the farm host to assume a printer is free just because
 *   time passed. idle() keeps running while we wait, so heaters, thermal
 *   protection and the watchdog all keep working normally.
 */

#include "../../inc/MarlinConfig.h"

#if ENABLED(FARM_MODE_CONFIRM_ON_FINISH)

#include "farm_confirm.h"
#include "../../MarlinCore.h"
#include "../../lcd/marlinui.h"
#include "../../lcd/menu/menu_item.h"

#if ENABLED(HOST_ACTION_COMMANDS)
  #include "../host_actions.h"
#endif

FarmConfirm farmConfirm;

namespace {

  enum class Resultado : uint8_t { PENDIENTE, OK, FALLO };
  Resultado resultado_confirmacion;

  void marcar_ok()    { resultado_confirmacion = Resultado::OK; }
  void marcar_fallo() { resultado_confirmacion = Resultado::FALLO; }

  // Función de pantalla: idle() la llama en cada vuelta del loop mientras
  // dure la espera (ver ui.goto_screen más abajo). Dibuja el diálogo
  // Sí/No nativo de Marlin y despacha el click al callback correspondiente.
  void pantalla_confirmacion() {
    MenuItem_confirm::select_screen(
      GET_TEXT_F(MSG_YES), GET_TEXT_F(MSG_NO),
      marcar_ok, marcar_fallo,
      F("Impresion OK?")
    );
  }

} // namespace

void FarmConfirm::notify_and_wait(const bool was_ok) {

  resultado_confirmacion = Resultado::PENDIENTE;

  // Deja pre-seleccionada la opción más probable según cómo terminó el
  // trabajo (fin normal de SD vs abort), pero el operario puede cambiarla
  // libremente con el encoder antes de hacer click — nunca decide por él.
  ui.set_selection(was_ok);
  ui.goto_screen(pantalla_confirmacion);

  #if ENABLED(HOST_PROMPT_SUPPORT)
    hostui.prompt_do(
      PROMPT_USER_CONTINUE,
      was_ok ? F("Print finished") : F("Print failed"),
      FPSTR(CONTINUE_STR)
    );
  #endif

  // Bloquea (sin colgar el firmware) hasta que el operario elija una
  // opción en el menú. Sin timeout: este estado nunca se limpia solo.
  while (resultado_confirmacion == Resultado::PENDIENTE) marlin.idle();

  // Recién ahora, tras la elección física del operario, se informa al
  // host — según lo que el operario eligió, no según `was_ok`.
  #if ENABLED(HOST_ACTION_COMMANDS)
    hostui.action(resultado_confirmacion == Resultado::OK
      ? F("print_ok_confirmed")
      : F("print_fail_confirmed")
    );
  #endif

  ui.return_to_status();
}

#endif // FARM_MODE_CONFIRM_ON_FINISH
