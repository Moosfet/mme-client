#include "everything.h"

//--page-split-- menus_escape

void menus_escape(void) {

  int row_count = 8;

  if (menu_process_event && KEY_PRESS_EVENT && KEY == GLFW_KEY_ESCAPE) menu_switch(menus_play);
  if (gui_window(52, row_count + 2, 3)) menu_switch(menus_play);

  gui_text(-1, 0, "The Menu of Menus");

  int line = 2;

  int flags;

  flags = MENU_FLAG_DISABLE * !(packet_is_sendable(PACKET_MENU_REQUEST) && strcmp(server_address, server_portal_address));
  if (!flags) {
    if (gui_button(2, line, 25, "This Server's Menu", flags | MENU_FLAG_OFFSET)) {
      server_menu_request();
      menu_switch(menus_play);
    };
  };
  line += 2;

  flags = MENU_FLAG_DISABLE * !strcmp(server_address, server_portal_address);
  #ifdef TEST_SERVER_ONLY
    flags = MENU_FLAG_DISABLE;
  #endif
  if (!flags) {
    if (gui_button(2, line, 25, "Return to Server List", flags | MENU_FLAG_OFFSET)) menu_switch(menus_server_disconnect);
  };
  line += 2;

  if (gui_button(2, line, 25, "Quit Playing the Game", MENU_FLAG_OFFSET)) main_shutdown();

  line = 2;
  if (gui_link(30, line++, "Configure Controls")) menu_switch(menus_controls);
  if (gui_link(30, line++, "Fog & Settings")) menu_switch(menus_fog);
  if (gui_link(30, line++, "Graphics Settings")) menu_switch(menus_graphics);
  if (gui_link(30, line++, "3D Anaglyph Settings")) menu_switch(menus_anaglyph);
  if (gui_link(30, line++, "Perspective Settings")) menu_switch(menus_perspective);
  if (gui_link(30, line++, "Miscellaneous Options")) menu_switch(menus_misc);
  if (gui_link(30, line++, "Fence Post Calculator")) menu_switch(menus_fencepost);
  if (gui_link(30, line++, "Sound Settings")) menu_switch(menus_sound);

};
