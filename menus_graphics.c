#include "everything.h"

//--page-split-- menus_graphics

void menus_graphics() {

  int small_blocks = map_data.resolution.x < 0.1625;

  if (menu_process_event && KEY_PRESS_EVENT && KEY == GLFW_KEY_ESCAPE) menu_switch(menus_escape);
  if (gui_window(54, 22, 3)) menu_switch(menus_play);

  gui_text(1, 0, "Graphics Options");

  int previous_setting = option_lighting;
  gui_radio(4, 2, &option_lighting, 0, "Disable lighting for all block sizes.");
  gui_radio(4, 3, &option_lighting, 1, "Enable lighting for larger block sizes only.");
  gui_radio(4, 4, &option_lighting, 2, "Enable lighting even for smaller block sizes.");
  if (previous_setting != option_lighting) {
    if (small_blocks) {
      if (previous_setting == 2 || option_lighting == 2) map_invalidate_chunks();
    } else {
      if (previous_setting == 0 || option_lighting == 0) map_invalidate_chunks();
    };
  };

  gui_text(3, 6, "Enabling lighting for small block sizes can");
  gui_text(3, 7, "cause blocks to fail to appear immediately.");

  int color = option_fog_type & 1;
  if (
    gui_radio(4, 9, &color, 1, "Day Mode") |
    gui_radio(4, 10, &color, 0, "Night Mode")
  ) {
    option_fog_type &= ~1;
    option_fog_type |= color;
    if (option_lighting) {
      map_invalidate_chunks();
    };
  };

  if (gui_check(4, 12, &option_textures, 0, "Disable Textures")) map_invalidate_chunks();

  gui_check(4, 13, &option_blender_font, 1, "Use a smoother but somewhat harder to read font.");

  gui_text(1, 15, "MSAA samples: (higher quality, lower FPS)");
  int change = 0;
  change |= gui_radio(4, 16, &option_msaa_samples, 1, "disabled");
  change |= gui_radio(4, 17, &option_msaa_samples, 4, "enabled");
  change |= gui_radio(4, 18, &option_msaa_samples, 9, "crazy");
  change |= gui_radio(4, 19, &option_msaa_samples, 16, "insane");
  if (change) {
    glfw_close_window();
    glfw_open_window();
  };
  gui_text(1, 20, "This option closes and re-opens the game window.");

};
