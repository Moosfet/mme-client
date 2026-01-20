#include "everything.h"

//--page-split-- menus_misc

void menus_misc(void) {

  if (menu_process_event && KEY_PRESS_EVENT && KEY == GLFW_KEY_ESCAPE) menu_switch(menus_escape);
  if (gui_window(40, 12, 3)) menu_switch(menus_play);

  gui_text(-1, 0, "Miscellaneous Options");

  gui_text(1, 2, "How to position window at startup:");
  gui_radio(4, 3, &option_window_location, 0, "Let the OS position it.");
  gui_radio(4, 4, &option_window_location, 1, "Center window on monitor.");
  gui_radio(4, 5, &option_window_location, 2, "Restore saved position.");
  gui_check(4, 6, &option_remember_size, 1, "Remember the game's window size.");

  gui_check(4, 8, &option_sound, 1, "Enable sound");
  gui_check(4, 9, &option_hud_display, 1, "Show (crappy) HUD");

  gui_check(4, 10, &option_superhuman, 1, "Super-Human Abilities");

};
