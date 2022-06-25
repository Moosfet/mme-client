#include "everything.h"

//--page-split-- menus_anaglyph

void menus_anaglyph() {

  static char one[13], two[13], three[13];
  static double one_value, two_value, three_value, last_value;
  static int last_width;

  void apply () {
    one_value = strtod(one, NULL);
    last_value = one_value / display_window_width;
    two_value = strtod(two, NULL);
    three_value = strtod(three, NULL);
    snprintf(one, 12, "%0.1f", one_value); one[12] = 0;
    snprintf(two, 12, "%0.1f", two_value); two[12] = 0;
    snprintf(three, 12, "%0.1f", three_value); three[12] = 0;
    option_anaglyph_pixel_size = one_value / display_window_width;
    option_anaglyph_distance = two_value;
    option_pupil_distance = three_value;
  };

  int small_blocks = map_data.resolution.x < 0.1625;

  if (menu_process_event && KEY_PRESS_EVENT && KEY == GLFW_KEY_ESCAPE) apply(), menu_switch(menus_escape);
  if (gui_window(76, 18, 3)) apply(), menu_switch(menus_play);

  gui_text(1, 0, "3D Anaglyph Configuration");
  int line = 2;
                   // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  gui_text(1, line++, "If you have red and blue 3D glasses, Multiplayer Map Editor can create");
  gui_text(1, line++, "a very realistic 3D rendering.  Figure out how close you can sit to your");
  gui_text(1, line++, "your monitor without going blind, then enter the measurements requested.");
  line++;
  line++;

  static int set = 0;
  if (!set++) {
    last_width = display_window_width;
    last_value = option_anaglyph_pixel_size;
    one_value = last_value * display_window_width;
    two_value = option_anaglyph_distance;
    three_value = option_pupil_distance;
    snprintf(one, 12, "%0.3f", one_value); one[12] = 0;
    snprintf(two, 12, "%0.3f", two_value); two[12] = 0;
    snprintf(three, 12, "%0.3f", three_value); three[12] = 0;
  };

  if (last_width != display_window_width) {
    one_value = last_value * display_window_width;
    snprintf(one, 12, "%0.1f", one_value); one[12] = 0;
    last_width = display_window_width;
  };

  gui_text(1, line, "Current width of the game window:");
  if (gui_input(36, line, 12, 12, one, NO_FLAGS)) apply();
  gui_text(51, line, "Measurement Units:");
  line++;
  line++;
  gui_text(1, line, "Distance from eyes to the screen:");
  if (gui_input(36, line, 12, 12, two, NO_FLAGS)) apply();
  gui_radio(55, line, &option_anaglyph_units, 0, "Centimeters");
  line++;
  gui_radio(55, line, &option_anaglyph_units, 1, "Inches");
  line++;
  gui_text(1, line, "Distance between your pupils:");
  if (gui_input(36, line, 12, 12, three, NO_FLAGS)) apply();
  line++;
  line++;
                     // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  //gui_text(-1, line++, "Measure as accurately as you can and include a decimal digit.");
  //gui_text(-1, line++, "If all measurements aren't correct, it won't work very well.");
  char *color[6] = {"red", "yellow", "green", "cyan", "blue", "magenta"};

  for (int j = 0; j < 2; j++) {
    int x = 18;
    if (j == 0) gui_text(1, line, "Left color:");
    if (j == 1) gui_text(1, line, "Right color:");
    for (int i = 0; i < 6; i++) {
      gui_radio(x, line, &option_anaglyph_filter[j], i, color[i]);
      x += 5 + strlen(color[i]);
    };
    line++;
  };

  line++;
  if (gui_check(4, line, &option_anaglyph_enable, 1, "Enable 3D Anaglyph Mode according to the above information.")) apply(), texture_rebind();

};
