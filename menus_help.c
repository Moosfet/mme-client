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
  if (gui_window(76, 18, 3)) menu_switch(menus_play);

  gui_text(-1, 0, "Help Menu");

                 // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  gui_text(1, 2  , "You can                       .  This describes the default configuration.");
  if (gui_link(9, 2, "customize the controls")) menu_switch(menus_controls);

  int line, column;
  line = 3; column = 3;

  line++;                // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  gui_text(column, line++, "\e\x05""ESC\e\x09 opens the game's menu");
  gui_text(column, line++, "\e\x05""F2\e\x09 opens the server's menu");
  gui_text(column, line++, "\e\x05""T\e\x09 opens the chat");

  line++;                // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  gui_text(column, line++, "\e\x05""Clicks\e\x09 create and destroy blocks");
  gui_text(column, line++, "\e\x05""B\e\x09 opens the block selection menu");
  gui_text(column, line++, "\e\x05""WASD\e\x09 or arrows keys move around");
  gui_text(column, line++, "hold \e\x05""shift\e\x09 to move faster and/or");
  gui_text(column, line  , "                              \e\x03*");
  gui_check(column + 3, line++, &option_superhuman, 1, "enable superhuman abilities");
  line++;           // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

  line = 3; column = 39;

  line++;                // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  gui_text(column, line++, "\e\x05""Q\e\x09 and \e\x05""E\e\x09 swim or fly\e\x03*\e\x09 up and down");
  gui_text(column, line++, "\e\x05""Z\e\x09 toggles flying mode\e\x03*");
  gui_text(column, line++, "hold \e\x05""X\e\x09 to walk through walls\e\x03*");

  line++;                // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  gui_text(column, line++, "\e\x09""Mode selection\e\x09 keys:");
  gui_text(column, line++, "\e\x05""2\e\x09 click & drag cuboids (default)");
  gui_text(column, line++, "\e\x05""3\e\x09 select map regions to copy\e\x03*");
  gui_text(column, line++, "\e\x05""4\e\x09 paste selected map regions\e\x03*");
  gui_text(column, line++, "\e\x05""6\e\x09 throw bouncy balls or rocks\e\x03*");

  line++;                // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  gui_text(5, line++, "\e\x03*\e\x09 marks items that will only work if the server allows them");
  line++;                                   // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  gui_check(10, line, &option_show_f1_help, 1, "Show \"                 \" in the corner of the screen.");
  gui_text(16, line, "\e\x0B""Press F1 for Help");

};
