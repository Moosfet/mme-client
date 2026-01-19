#include "everything.h"

static int scroll_index = 0;
static int last_chat_count = -1;

//--page-split-- menus_chat_scroll_reset

void menus_chat_scroll_reset() {
    scroll_index = 0;
}

//--page-split-- menus_chat

void menus_chat() {

  static int map;

  #define MAX_CHAT 200
  static char chat[MAX_CHAT + 1] = {};

  if (chat_array_count != last_chat_count) {
    if (scroll_index > 0 && chat_array_count > last_chat_count) {
      scroll_index += chat_array_count - last_chat_count;
    }
    last_chat_count = chat_array_count;
  }

  if (WHEEL_EVENT) {
    scroll_index += event_list[menu_current_event][2];
  }
  if (KEY_PRESS_EVENT) {
    if (KEY == GLFW_KEY_HOME && (glfwGetKey(glfw_window, GLFW_KEY_RIGHT_CONTROL) || glfwGetKey(glfw_window, GLFW_KEY_LEFT_CONTROL))) scroll_index = chat_array_count - gui_text_lines + 4;
    if (KEY == GLFW_KEY_END && (glfwGetKey(glfw_window, GLFW_KEY_RIGHT_CONTROL) || glfwGetKey(glfw_window, GLFW_KEY_LEFT_CONTROL))) scroll_index = 0;
    if (KEY == GLFW_KEY_PAGE_UP) scroll_index += gui_text_lines - 4;
    if (KEY == GLFW_KEY_PAGE_DOWN) scroll_index -= gui_text_lines - 4;
  }

  if (scroll_index > chat_array_count - gui_text_lines + 4)
    scroll_index = chat_array_count - gui_text_lines +  4;

  if (scroll_index < 0) scroll_index = 0;

  if (gui_window(gui_text_columns - 2, gui_text_lines - 1, 1)) {
    if (option_chat_clear) chat[0] = 0;
    menu_switch(menus_play);
  };

  if (menu_focus_object < 1) menu_focus_object = 1, map = 0;

  if (!menu_process_event) {
    chat_render(1, 1, gui_text_columns - 5, gui_text_lines - 4, 1, scroll_index);
  };

  static int cursor = 0;
  menu_object_data[menu_object_index+1][0] = cursor;
  int submit = gui_input(1, gui_text_lines - 3, gui_text_columns - 8, MAX_CHAT, chat, MENU_FLAG_OFFSET) == 2;
  cursor = menu_object_data[menu_object_index][0];

  int length = strlen(chat);
  if (menu_next_focus == 2) {
    menu_next_focus = 1;
    map = !map;
  };

  map = glfwGetKey(glfw_window, GLFW_KEY_TAB);

  if (map) {
    int xo = (gui_text_columns - 32) / 2;
    int yo = (gui_text_lines - 8) / 2;
    if (menu_draw_widget) {
      glColor4f(0.0, 0.0, 0.0, 1.0);
      glBegin(GL_QUADS);
      int x1 = 12 * (xo - 2) + gui_menu_x_offset;
      int x2 = 12 * (xo + 34) + gui_menu_x_offset;
      int y1 = 24 * (yo - 1.5) + gui_menu_x_offset;
      int y2 = 24 * (yo + 9.5) + gui_menu_x_offset;
      glVertex2f(x1, y1);
      glVertex2f(x2, y1);
      glVertex2f(x2, y2);
      glVertex2f(x1, y2);
      glEnd();
    };
    for (int a = 0; a < 256; a++) {
      if (a == 0 || a == 27 || a == 127 || a == 255) continue;
      int xi = a % 32;
      int yi = a / 32;
      if (gui_link(xo + xi, yo + yi, (char *) &a)) {
        if (length < MAX_CHAT) {
          memmove(chat + cursor + 1, chat + cursor, length - cursor + 1);
          chat[cursor] = a;
          cursor++;
          length++;
        };
      };
    };
  };

  char remaining[4];
  snprintf(remaining, 4, "%d", MAX_CHAT - length);
  remaining[3] = 0;
  int x_offset = 6 * (4 - strlen(remaining));
  glPushMatrix();
  glTranslatef(x_offset, 12, 0);
  int left = MAX_CHAT - length;
  if (left < 10) {
    chat_color(11);
  } else if (left < 25) {
    chat_color(2);
  } else if (left < 50) {
    chat_color(3);
  } else if (left < 100) {
    chat_color(4);
  } else {
    chat_color(9);
  };
  gui_draw_text(gui_text_columns - 6, gui_text_lines - 3, remaining, NULL, NO_FLAGS);
  glPopMatrix();

  if (menu_process_event) {
    if (KEY_PRESS_EVENT) {
      if (KEY == GLFW_KEY_ESCAPE) {
        if (option_chat_clear) chat[0] = 0;
        menu_switch(menus_play);
      };
    };
    if (submit) {
      char rewrite[MAX_CHAT + 1];
      char *i = chat;
      char *o = rewrite;
      int space_count = 2;
      while (*i) {
        if (*i == 32) {
          space_count++;
        } else {
          space_count = 0;
        };
        if (space_count > 2) {
          i++;
          continue;
        };
        *o++ = *i++;
      };
      *o = 0;
      while (--o >= rewrite && *o == 32) *o = 0;
      if (strlen(rewrite)) {
        packet_send(PACKET_CHAT_MESSAGE, 0, rewrite);
        //printf("Chat message: '%s'\n", rewrite);
      };
      chat[0] = 0;
      if (option_chat_return ^ menu_modifier_shift) menu_switch(menus_play);
    };
  };
  //if (gui_link(1, 0, "Click Here for Chat Options")) menu_switch(menus_controls);
  gui_check(4, 0, &option_chat_return, 1, "Return to game after sending chat.");
  gui_check(44, 0, &option_chat_clear, 1, "Clear input when leaving menu.");

  scroll_index = gui_scroll_bar_vertical (gui_text_columns - 3, 2, 1, gui_text_lines - 6, chat_array_count, gui_text_lines - 4, scroll_index);

};
