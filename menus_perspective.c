#include "everything.h"

//--page-split-- menus_perspective

void menus_perspective() {

  if (menu_process_event && KEY_PRESS_EVENT && KEY == GLFW_KEY_ESCAPE) menu_switch(menus_escape);
  if (gui_window(74, 16, 3)) menu_switch(menus_play);

  gui_text(-1, 0, "Perspective Options");

  static int angle = 0;
  if (angle == 0) {
    if (option_anaglyph_enable) {
      angle = -1;
    } else {
      angle = option_perspective_angle;
    };
  };

  int line = 2;
  char buffer[20];
  for (int i = 30; i <= 120; i += 10) {
    snprintf(buffer, 20, "%d degrees", i);
    gui_radio(4, line++, &angle, i, buffer);
  };
  line++;
  gui_radio(4, line, &angle, -1, "Controlled by 3D Anaglyph Mode");
  option_anaglyph_enable = angle == -1;

  if (angle != -1) option_perspective_angle = angle;

};
