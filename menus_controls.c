#include "everything.h"

static char select_text[128];
static int *key_value = NULL;
static double invalid_key_stamp = 0;
int menus_controls_disable_function_keys = 0;

//--page-split-- key_change_link

static int key_change_link (int x, int y, char *desc, int option) {
  char temp[64];

  gui_text(x, y, desc);

  snprintf (temp, 64, "%-16s", controls_key_name (option_key_input[option][0]));

  if (gui_link(x + 15, y, temp)) {
    key_value = &option_key_input[option][0];
    strcpy (select_text, desc);
    menus_controls_disable_function_keys = 1;
    menu_switch(menus_controls_key_select);
  }

  gui_text(x + 30, y, "/");

  snprintf (temp, 64, "%-16s", controls_key_name (option_key_input[option][1]));

  if (gui_link(x + 32, y, temp)) {
    key_value = &option_key_input[option][1];
    strcpy (select_text, desc);
    menus_controls_disable_function_keys = 1;
    menu_switch(menus_controls_key_select);
  }

}

//--page-split-- menus_controls

void menus_controls() {

  if (menu_process_event && KEY_PRESS_EVENT && KEY == GLFW_KEY_ESCAPE) menu_switch(menus_escape);
  if (gui_window(78, 22, 3)) menu_switch(menus_play);

  gui_text(-1, 0, "Control Settings");
  gui_check(4, 2, &option_mouse_invert, 1, "Invert Mouse Movement");
//  gui_check(4, 3, &option_mouse_reverse, 1, "Reverse Mouse Buttons");

  gui_check(4, 4, &option_always_run, 1, "Always Run");
  gui_check(4, 5, &option_fly_fast, 1, "Always Fly Fast");

  if (
    gui_radio(4, 7, &option_noclip, 0, "Hold X to Noclip") ||
    gui_radio(4, 8, &option_noclip, 1, "X Toggles Noclip") ||
    gui_radio(4, 9, &option_noclip, 2, "Flying Activates Noclip")
  ) {
    player_noclip = 0;
  };

  gui_check(4, 12, &option_multiple_paste, 1, "Multiple Pasting");

  gui_check(4, 14, &option_superhuman, 1, "Superhuman Abilities");

  key_change_link(29, 2, "Place Block", CONTROLS_KEY_BLOCK_CREATE);
  key_change_link(29, 3, "Destory Block", CONTROLS_KEY_BLOCK_DESTROY);
  key_change_link(29, 4, "Clone Block", CONTROLS_KEY_BLOCK_CLONE);
  key_change_link(29, 5, "Replace Block", CONTROLS_KEY_BLOCK_REPLACE);
  key_change_link(29, 6, "Move Forward", CONTROLS_KEY_FORWARD);
  key_change_link(29, 7, "Move Backward", CONTROLS_KEY_BACK);
  key_change_link(29, 8, "Move Left", CONTROLS_KEY_LEFT);
  key_change_link(29, 9, "Move Right", CONTROLS_KEY_RIGHT);
  key_change_link(29, 10, "Fly/Swim Up", CONTROLS_KEY_FLY_UP);
  key_change_link(29, 11, "Fly/Swim Down", CONTROLS_KEY_FLY_DOWN);
  key_change_link(29, 12, "Run", CONTROLS_KEY_RUN);
  key_change_link(29, 13, "Jump", CONTROLS_KEY_JUMP);
  key_change_link(29, 14, "Slow Walk", CONTROLS_KEY_SNEAK);
  key_change_link(29, 15, "Fly Mode", CONTROLS_KEY_MODE_FLY);
  key_change_link(29, 16, "No Clip", CONTROLS_KEY_MODE_NOCLIP);
  key_change_link(29, 17, "Chat",CONTROLS_KEY_CHAT);
  key_change_link(29, 18, "Block Menu", CONTROLS_KEY_BLOCK_MENU);
  key_change_link(29, 19, "Server Menu", CONTROLS_KEY_SERVER_MENU);

  if (gui_link(31, 20, "Reset Controls To Defaults")) option_key_reset();

};

//--page-split-- unassign_key

static void unassign_key (int key) {
  int a;
  for (a = 0; a < CONTROLS_KEY_LIMIT; a++) {
    if (option_key_input[a][0] == key) option_key_input[a][0] = 0;
    if (option_key_input[a][1] == key) option_key_input[a][1] = 0;
  }
}

//--page-split-- menus_controls_key_select

void menus_controls_key_select () {

  if (menu_process_event && KEY_PRESS_EVENT) {
    int q = KEY;
    if (q == GLFW_KEY_ESCAPE) q = 0;
    if (q && controls_key_is_invalid(q)) {
      invalid_key_stamp = on_frame_time + 2.0;
    } else {
      unassign_key(q);
      *key_value = q;
      menus_controls_disable_function_keys = 0;
      menu_switch(menus_controls);
    };
  }

  if (gui_window(40, 4, 0)) menu_switch(menus_controls_key_select);

  char *temp;
  asprintf(&temp, "Press a new key for \e\x05%s\e\x09,", select_text);
  gui_text(-1, 1, temp);
  free(temp);

  if (invalid_key_stamp >= on_frame_time) {
    gui_text(-1, 2, "\e\x03""but not that key.  It's reserved.");
  } else {
    gui_text(-1, 2, "or press escape to choose nothing.");
  };

}
