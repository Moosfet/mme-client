#include "everything.h"

//--page-split-- menus_autofog

void menus_autofog(void) {

  if (menu_process_event && KEY_PRESS_EVENT && KEY == GLFW_KEY_ESCAPE) menu_switch(menus_escape);
  if (gui_window(58, 14, 3)) menu_switch(menus_play);

  gui_text(-1, 0, "Automatic Fog Settings");

  gui_text(1, 2, "Automatic fog adjusts your fog distance to acheive a desired FPS rate.");

  gui_radio(4, 3, &option_fps_goal, 10, "10 FPS");
  gui_radio(4, 4, &option_fps_goal, 12, "12 FPS");
  gui_radio(4, 5, &option_fps_goal, 15, "15 FPS");
  gui_radio(4, 6, &option_fps_goal, 20, "20 FPS");
  gui_radio(24, 3, &option_fps_goal, 24, "24 FPS");
  gui_radio(24, 4, &option_fps_goal, 30, "30 FPS");
  gui_radio(24, 5, &option_fps_goal, 48, "48 FPS");
  gui_radio(24, 6, &option_fps_goal, 60, "60 FPS");

  if (option_fps_limit < option_fps_goal) {
    option_fps_limit = option_fps_goal;
  };

  #if 0
  static char fps_text[16] = {27};
  if (fps_text[0] == 27) {
    if (option_custom_fps == 0) {
      fps_text[0] = 0;
    } else {
      snprintf(fps_text, 5, "%d", option_custom_fps);
      fps_text[3] = 0;
    };
  };
  gui_radio(44, 3, &option_fps_goal, 1, "custom");
  gui_input(45, 5, 4, 4, fps_text, FLAGS);
  option_custom_fps = atoi(fps_text);
  #endif

};
