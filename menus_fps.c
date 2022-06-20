#include "everything.h"

//--page-split-- menus_fps

void menus_fps() {

  if (menu_process_event && KEY_PRESS_EVENT && KEY == GLFW_KEY_ESCAPE) menu_switch(menus_escape);
  if (gui_window(58, 14, 3)) menu_switch(menus_play);

  gui_text(-1, 0, "FPS Settings");

  if (gui_check(4, 11, &option_request_vertical_sync, 1, "Ask the graphics driver to enable vertical sync.")) main_restart();

  if (option_request_vertical_sync) {
    if (statistics_vsync_rate == 0) {
      gui_text(1, 12, "Note: Your graphics driver has ignored this request.");
    };
  } else {
    if (statistics_vsync_rate != 0) {
      gui_text(1, 12, "Note: Your graphics driver forces vertical sync.");
    };
  };

  gui_check(4, 9, &option_fps_display, 1, "Display the current FPS in the corner of the screen.");

  gui_text(1, 2, "Desired FPS Limit");
  gui_radio(4, 3, &option_fps_limit, 10, "10 FPS");
  gui_radio(4, 4, &option_fps_limit, 12, "12 FPS");
  gui_radio(4, 5, &option_fps_limit, 15, "15 FPS");
  gui_radio(4, 6, &option_fps_limit, 20, "20 FPS");
  gui_radio(24, 3, &option_fps_limit, 24, "24 FPS");
  gui_radio(24, 4, &option_fps_limit, 30, "30 FPS");
  gui_radio(24, 5, &option_fps_limit, 48, "48 FPS");
  gui_radio(24, 6, &option_fps_limit, 60, "60 FPS");
  gui_radio(4, 7, &option_fps_limit, 0, "Do not limit FPS (not recommended)");

  static char fps_text[16] = {27};
  if (fps_text[0] == 27) {
    if (option_custom_fps == 0) {
      fps_text[0] = 0;
    } else {
      snprintf(fps_text, 5, "%d", option_custom_fps);
      fps_text[3] = 0;
    };
  };
  gui_radio(44, 3, &option_fps_limit, 1, "custom");
  gui_input(45, 5, 4, 4, fps_text, FLAGS);
  option_custom_fps = atoi(fps_text);

  if (option_fog_distance == 0) {
    if (option_fps_limit == 1) {
      if (menu_function_pointer != menus_fps) {
        if (option_fps_goal > option_custom_fps) {
          option_fps_goal = option_custom_fps;
        };
      };
    } else {
      if (option_fps_goal > option_fps_limit) {
        option_fps_goal = option_fps_limit;
      };
    };
  };

};
