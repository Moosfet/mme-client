#include "everything.h"

//--page-split-- menus_misc

void menus_misc() {

  if (menu_process_event && KEY_PRESS_EVENT && KEY == GLFW_KEY_ESCAPE) menu_switch(menus_escape);
  if (gui_window(40, 10, 3)) menu_switch(menus_play);

  gui_text(-1, 0, "Miscellaneous Options");

  gui_check(4, 2, &option_remember_size, 1, "Remember the game's window size.");
  gui_check(4, 3, &option_center_window, 1, "Center the window when opening it.");

  gui_check(4, 5, &option_sound, 1, "Enable sound");
  gui_check(4, 6, &option_hud_display, 1, "Show (crappy) HUD");

  gui_check(4, 8, &option_superhuman, 1, "Super-Human Abilities");

};
