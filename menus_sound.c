#include "everything.h"

//--page-split-- menus_sound

void menus_sound() {

  if (menu_process_event && KEY_PRESS_EVENT && KEY == GLFW_KEY_ESCAPE) menu_switch(menus_escape);
  if (gui_window(24, 8, 3)) menu_switch(menus_play);

  gui_text(-1, 0, "Sound Settings");

  gui_check(4, 2, &option_sound, 1, "Enable sound");

  static int set = 0;
  static char volume[13];
  if (!set++) {
    option_volume = nearbyintf(1000.0f * option_volume) / 1000.0f;
    snprintf(volume, 12, "%0.3f", option_volume); volume[12] = 0;
  };
  gui_text(1, 4, "Volume:");
  if (gui_input(10, 4, 12, 12, volume, NO_FLAGS)) {
    option_volume = strtod(volume, NULL);
    if (option_volume > 100.0f) option_volume = 100.0f;
    if (option_volume < 0.001f) option_volume = 0.001f;
    option_volume = nearbyintf(1000.0f * option_volume) / 1000.0f;
    snprintf(volume, 12, "%0.3f", option_volume); volume[12] = 0;
  };

  gui_text(1, 6, "Normal volume is 1.000");

};
