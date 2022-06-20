#include "everything.h"

//--page-split-- menus_perspective

void menus_perspective() {

  if (menu_process_event && KEY_PRESS_EVENT && KEY == GLFW_KEY_ESCAPE) menu_switch(menus_escape);
  if (gui_window(74, 18, 3)) menu_switch(menus_play);

  gui_text(-1, 0, "Perspective Options");

  static int angle = 0;
  if (angle == 0) {
    if (option_perspective_perfect) {
      angle = -1;
    } else {
      angle = option_perspective_angle;
    };
  };

  gui_radio(4, 2, &angle, 30, "30 degrees (narrow view)");
  gui_radio(4, 3, &angle, 45, "45 degrees (classic view)");
  gui_radio(4, 4, &angle, 60, "60 degrees (wide view)");
  gui_radio(4, 5, &angle, 90, "90 degrees (distorted, but recommended view)");
  gui_radio(4, 6, &angle, 120, "120 degrees (totally distorted)");
  gui_radio(4, 7, &angle, -1, "Use the Perfect Perspective Calculator");
  if (angle == -1) {
    char buffer[64];
    if (option_perspective_perfect) {
      double current = 2.0 * atan(0.5 * display_window_height * option_perspective_pixel_size / option_perspective_distance) * 180 / M_PI;
      if (display_window_height > display_window_width) {
        double v_factor = tan(0.5 * current * M_PI / 180);
        double h_factor = v_factor * display_window_width / display_window_height;
        current = 2.0 * atan(h_factor) * 180 / M_PI;
      };
      snprintf(buffer, 64, "(%0.0f degrees)", current);
    } else {
      snprintf(buffer, 64, "\e\x0B* invalid measurements *");
    };
    gui_text(44, 7, buffer);
  };

  if (angle != -1) {
    option_perspective_perfect = 0;
    option_perspective_angle = angle;
  } else {
    static char one[13], two[13];
    static double one_value, two_value, last_value;
    static int last_width;
    static int set = 0;
    if (!set++) {
      last_width = display_window_width;
      last_value = option_perspective_pixel_size;
      one_value = last_value * display_window_width;
      two_value = option_perspective_distance;
      snprintf(one, 12, "%0.1f", one_value); one[12] = 0;
      snprintf(two, 12, "%0.1f", two_value); two[12] = 0;
      option_perspective_perfect = 1;
    };
    if (last_width != display_window_width) {
      one_value = last_value * display_window_width;
      snprintf(one, 12, "%0.1f", one_value); one[12] = 0;
      last_width = display_window_width;
    };
    gui_text(1,  8, "Calculating the perfect perspective requires the following measurements:");
    gui_text(1,  9, "(use any units of measure, but use the same units for both measurements)");

    gui_text(1, 11, "Current width of the game window:");
    gui_text(1, 13, "Distance from eyes to the screen:");
    if (gui_input(36, 11, 12, 12, one, NO_FLAGS)) {
      one_value = strtod(one, NULL);
      last_value = one_value / display_window_width;
    };
    if (gui_input(36, 13, 12, 12, two, NO_FLAGS)) two_value = strtod(two, NULL);
    if (gui_button(-1, 15, -1, "Calculate the perfect perspective angle!", NO_FLAGS)) {

      // copy & paste from above, not sure these 3 lines belong here
      one_value = strtod(one, NULL);
      last_value = one_value / display_window_width;
      two_value = strtod(two, NULL);

      option_perspective_pixel_size = one_value / display_window_width;
      option_perspective_distance = two_value;
      snprintf(one, 12, "%0.1f", one_value); one[12] = 0;
      snprintf(two, 12, "%0.1f", two_value); two[12] = 0;
      option_perspective_perfect = 1;
    };

    gui_text(1, 17, "(for best results, you want to be as close to the screen as is possible)");
    option_perspective_perfect = 1;
  };

  if (gui_button(56, 3, -1, "Save Settings", NO_FLAGS)) {
    option_save();
    menu_switch(menus_play);
  };

};
