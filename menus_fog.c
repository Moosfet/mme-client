#include "everything.h"

//--page-split-- menus_fog

void menus_fog(void) {

  if (menu_process_event && KEY_PRESS_EVENT && KEY == GLFW_KEY_ESCAPE) menu_switch(menus_escape);
  if (gui_window(46, 15, 3)) menu_switch(menus_play);

  gui_text(-1, 0, "Fog Settings");

  int color = option_fog_type & 1;
  int type = option_fog_type & 6;

  gui_text(4, 2, "Day / Night");
  if (
    gui_radio(4, 4, &color, 1, "Day Mode") |
    gui_radio(4, 5, &color, 0, "Night Mode")
  ) {
    if (option_lighting) {
      map_invalidate_chunks();
    };
  };

  gui_text(4, 7, "Fog Proximity");
  gui_radio(4, 9, &type, 2, "Nearby Fog");
  gui_radio(4, 10, &type, 6, "Distant Fog");
  gui_radio(4, 11, &type, 0, "Invisible Fog");

  option_fog_type = type | color;

  if (gui_link(2, 13, "FPS Settings")) menu_switch(menus_fps);

  gui_text(28, 2, "Max Fog Distance");
  //gui_radio(28, 4, &option_fog_distance, 0, "automatic (     )");
  //if (gui_link(39, 4, "setup")) menu_switch(menus_autofog);
  gui_radio(28, 5, &option_fog_distance, 100, "100 block radius");
  gui_radio(28, 6, &option_fog_distance, 160, "160 block radius");
  gui_radio(28, 7, &option_fog_distance, 256, "256 block radius");
  gui_radio(28, 8, &option_fog_distance, 400, "400 block radius");
  gui_radio(28, 9, &option_fog_distance, 640, "640 block radius");
  gui_radio(28, 10, &option_fog_distance, 1024, "1024 block radius");
  gui_radio(28, 11, &option_fog_distance, 1600, "1600 block radius");
  gui_radio(28, 12, &option_fog_distance, 2560, "2560 block radius");
  gui_radio(28, 13, &option_fog_distance, 4096, "4096 block radius");

};
