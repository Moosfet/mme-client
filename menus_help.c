#include "everything.h"

//--page-split-- gui_textf

static void gui_textf (int c, int r, char *str, ... ) {
    va_list  arg;
    char buffer[4096];
    va_start(arg, str);
    vsnprintf(buffer, 4096, str, arg);
    buffer[4095] = 0;
    va_end(arg);
    gui_text(c, r, buffer);
}

//--page-split-- control_choice

static char *control_choice (int key) {
  char *ret = "<not assigned>";
  if (option_key_input[key][0] != 0) ret = controls_key_name (option_key_input[key][0]);
  else if (option_key_input[key][1] != 0) {
    ret = controls_key_name (option_key_input[key][1]);
  }
  return ((char *)ret);
}

//--page-split-- menus_help

void menus_help() {
  option_hyper_help = 0;

  if (menu_process_event && KEY_PRESS_EVENT && KEY == GLFW_KEY_ESCAPE) menu_switch(menus_play);
  if (gui_window(78, 24, 3)) menu_switch(menus_play);

  gui_text(-1, 0, "Help Menu");

  gui_textf(1, 2, "%s destroys blocks, %s creates blocks.", control_choice (CONTROLS_KEY_BLOCK_DESTROY), control_choice (CONTROLS_KEY_BLOCK_CREATE));
  gui_text(1, 3, "...or you may                                     .");
  if (gui_link(15, 3, "click here to customize the controls")) {
    menu_switch (menus_controls);
  };
  gui_text(1, 5, "Brief control summary:");
  gui_textf(1, 7, "%s - choose which block to build with", control_choice (CONTROLS_KEY_BLOCK_MENU));

  gui_textf(1, 8, "%14s - move forward   %14s - toggle flying mode", control_choice (CONTROLS_KEY_FORWARD), control_choice (CONTROLS_KEY_MODE_FLY));
  gui_textf(1, 9, "%14s - move left      %14s - fly upward", control_choice (CONTROLS_KEY_LEFT), control_choice (CONTROLS_KEY_FLY_UP));
  gui_textf(1, 10, "%14s - move backward  %14s - fly downward", control_choice (CONTROLS_KEY_BACK), control_choice(CONTROLS_KEY_FLY_DOWN));
  gui_textf(1, 11, "%14s - move right     %14s - hold to walk through walls", control_choice (CONTROLS_KEY_RIGHT), control_choice (CONTROLS_KEY_MODE_NOCLIP));
  gui_text(1, 12, "");
  gui_textf(1, 13, "%s - \e\x0B" "allows you to run\e\x09 or fly faster, %s - walk slowly", control_choice(CONTROLS_KEY_RUN), control_choice (CONTROLS_KEY_SNEAK));

  gui_text(1, 14, "");
  gui_text(1, 15, "2 - switches to click-and-drag cuboid mode");
  gui_text(1, 16, "3 - switches to selection mode (to copy and paste)");
  gui_text(1, 17, "4 - switches to paste mode (to paste something again)");
  gui_text(1, 18, "");
  gui_textf(1, 19, "%s - chat with other players, and release the mouse pointer", control_choice(CONTROLS_KEY_CHAT));
  gui_textf(1, 20, "%s - access the server's menu (to use server commands)", control_choice(CONTROLS_KEY_SERVER_MENU));
  gui_text(1, 21, "ESC - access the game's menu, and release the mouse pointer");
  gui_text(1, 22, "F4 - create a screenshot");

  gui_check(4, 23, &option_show_f1_help, 1, "Show \"Press H for Help\" at the top of the screen.");

};
