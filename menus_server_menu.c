#include "everything.h"

struct structure_server_menu menus_server_menu_data[MENU_MAX_OBJECTS - 1] = {};

int menus_server_menu_value[256] = {};

int menus_server_menu_active = 0;
int menus_server_menu_number = 0;
int menus_server_menu_width = 0;
int menus_server_menu_height = 0;
int menus_server_menu_options = 0;

#define type menus_server_menu_data[i].type
#define name menus_server_menu_data[i].name
#define value menus_server_menu_data[i].value
#define column menus_server_menu_data[i].column
#define line menus_server_menu_data[i].line
#define width menus_server_menu_data[i].width
#define length menus_server_menu_data[i].length
#define flags menus_server_menu_data[i].flags
#define text menus_server_menu_data[i].text
#define salt_count menus_server_menu_data[i].salt_count
#define salt_one menus_server_menu_data[i].salt_one
#define salt_two menus_server_menu_data[i].salt_two
#define texture menus_server_menu_data[i].texture
#define height menus_server_menu_data[i].height
#define coordinate menus_server_menu_data[i].coordinate

static char salt[20];

//--page-split-- salt_password

static void salt_password(int i, int f) {
  int password_length = strlen(text);
  int buffer_size = password_length + 20;
  if (buffer_size < 40) buffer_size = 40;

  char *buffer = NULL;
  memory_allocate(&buffer, buffer_size);

  char digest[20];
  if (salt_count == 0) {
    memmove(buffer, salt, 20);
    memmove(buffer + 20, text, password_length);
    sha1(digest, buffer, 20 + password_length);
  } else {
    memmove(buffer, salt_one, 20);
    memmove(buffer + 20, text, password_length);
    sha1(digest, buffer, 20 + password_length);
    if (salt_count == 2) {
      memmove(buffer, salt_two, 20);
      memmove(buffer + 20, digest, 20);
      sha1(digest, buffer, 40);
    };
    memmove(buffer, salt, 20);
    memmove(buffer + 20, digest, 20);
    sha1(digest, buffer, 40);
  };
  memmove(buffer + 20, digest, 20);

  packet_send(PACKET_MENU_RESPONSE, name, value, f | MENU_FLAG_STARS, buffer);

  memory_allocate(&buffer, 0);
};

//--page-split-- send_form_data

static void send_form_data(void) {
  easy_random_binary_string(salt, 20);
  for (int i = 0; i < MENU_MAX_OBJECTS - 1; i++) {
    if (type == MENU_CHECK) {
      packet_send(PACKET_MENU_RESPONSE, name, value, flags & MENU_FLAG_ACTIVE, NULL);
    } else if (type == MENU_RADIO) {
      if (menus_server_menu_value[name] == value) {
        packet_send(PACKET_MENU_RESPONSE, name, value, MENU_FLAG_ACTIVE, NULL);
        menus_server_menu_value[name] = -1;
      };
    } else if (type == MENU_INPUT) {
      packet_send(PACKET_MENU_RESPONSE, name, value, MENU_FLAG_NORMAL, text);
    } else if (type == MENU_PASSWORD) {
      salt_password(i, MENU_FLAG_NORMAL);
    };
  };
};

//--page-split-- menus_server_menu_reset

void menus_server_menu_reset(void) {
  menus_server_menu_active = 0;
  for (int i = 0; i < MENU_MAX_OBJECTS - 1; i++) {
    type = MENU_NONE;
    if (text != NULL) memory_allocate(&text, 0);
  };
  for (int i = 0; i < 256; i++) {
    menus_server_menu_value[i] = -1;
  };
};

//--page-split-- menus_server_menu

void menus_server_menu(void) {

  lag_push(1, "menus_server_menu()");

  if (menu_process_event) {
    if (KEY_PRESS_EVENT) {
      if (KEY == GLFW_KEY_F2 && (menus_server_menu_options & 1)) {
        menus_server_menu_reset();
        packet_send(PACKET_MENU_RESPONSE);
        menu_switch(menus_play);
      };
      if (KEY == GLFW_KEY_ESCAPE) {
        if (menus_server_menu_options & 1) {
          menus_server_menu_reset();
          packet_send(PACKET_MENU_RESPONSE);
          menu_switch(menus_play);
        } else {
          menu_switch(menus_escape);
        };
      };
    };
  };

  if (gui_window(menus_server_menu_width, menus_server_menu_height, menus_server_menu_options)) {
    menus_server_menu_reset();
    packet_send(PACKET_MENU_RESPONSE);
    menu_switch(menus_play);
  };

  if (!menus_server_menu_active) {
    menu_switch(menus_play);
    lag_pop();
    return;
  };

  int base = menu_object_index;
  int last_object_index = menu_object_index;
  int true_next_focus = menu_next_focus;
  menu_next_focus = -1;

  for (int i = 0; i < MENU_MAX_OBJECTS - 1; i++) {
    if (type == MENU_NONE) continue;
    if (type == MENU_TEXT) {
      char *s = NULL; char *c = NULL;
      chat_color_decode(&s, &c, text, FALSE);
      gui_draw_text(column, line, s, c, 0);
      memory_allocate(&s, 0);
      memory_allocate(&c, 0);
      continue;
    };
    if (menu_next_focus != -1) {
      true_next_focus = menu_object_index + 1;
      menu_next_focus = -1;
    };
    if (type == MENU_LINK) {
      if (gui_link(column, line, text)) {
        packet_send(PACKET_MENU_RESPONSE, name, value, MENU_FLAG_ACTIVE, NULL);
        menus_server_menu_reset();
        break;
      };
    } else if (type == MENU_CHECK) {
      int state = flags & MENU_FLAG_ACTIVE;
      if (gui_check(column, line, &state, TRUE, text)) {
        if (flags & MENU_FLAG_LIVE) {
          flags = MENU_FLAG_LIVE | (state * MENU_FLAG_ACTIVE);
          packet_send(PACKET_MENU_RESPONSE, name, value, flags, NULL);
        } else {
          flags = state * MENU_FLAG_ACTIVE;
        };
      };
    } else if (type == MENU_RADIO) {
      if (gui_radio(column, line, &menus_server_menu_value[name], value, text)) {
        if (flags & MENU_FLAG_LIVE) {
          packet_send(PACKET_MENU_RESPONSE, name, value, MENU_FLAG_LIVE | MENU_FLAG_ACTIVE, NULL);
        };
      };
    } else if (type == MENU_INPUT || type == MENU_PASSWORD) {
      int result = gui_input(column, line, width, length, text, flags);
      if (result) {
        if (flags & MENU_FLAG_LIVE) {
          if (type == MENU_PASSWORD) {
            // Don't send anything.  They don't need to know.
          } else {
            packet_send(PACKET_MENU_RESPONSE, name, value, MENU_FLAG_LIVE, text);
          };
        };
      };
      if ((flags & MENU_FLAG_ACTIVE) && result == 2) {
        send_form_data();
        if (type == MENU_PASSWORD) {
          salt_password(i, MENU_FLAG_ACTIVE);
        } else {
          packet_send(PACKET_MENU_RESPONSE, name, value, MENU_FLAG_ACTIVE, text);
        };
        menus_server_menu_reset();
      };
    } else if (type == MENU_BUTTON) {
      if (gui_button(column, line, width, text, flags)) {
        send_form_data();
        packet_send(PACKET_MENU_RESPONSE, name, value, MENU_FLAG_ACTIVE, NULL);
        menus_server_menu_reset();
      };
    } else if (type == MENU_TEXTURE) {
      if (menu_draw_widget) {
        float tc[4];
        tc[0] = coordinate[0] / 256.0;
        tc[1] = coordinate[1] / 256.0;
        tc[2] = coordinate[2] / 256.0;
        tc[3] = coordinate[3] / 256.0;
        if (tc[2] < 0.001) tc[2] = 1.0;
        if (tc[3] < 0.001) tc[3] = 1.0;
        tc[1] = 1.0 - tc[1];
        tc[3] = 1.0 - tc[3];
        gui_texture(column, line, width, height, texture, tc[0], tc[1], tc[2], tc[3]);
      };
    } else {
      easy_fuck("Unknown server menu widget type.");
    };
    if (menu_next_focus != -1) {
      if (menu_next_focus == 0) {
        true_next_focus = 0;
        menu_next_focus = -1;
      } else if (menu_next_focus == menu_object_index) {
        true_next_focus = menu_object_index;
        menu_next_focus = -1;
      } else if (menu_next_focus == menu_object_index - 1) {
        true_next_focus = last_object_index;
        menu_next_focus = -1;
      };
    };
    last_object_index = menu_object_index;
  };

  if (menu_next_focus != -1) {
    menu_next_focus = 0;
  } else {
    menu_next_focus = true_next_focus;
  };

  //if (menu_next_focus == 1) menu_next_focus = 256;
  //if (menu_next_focus == 255) menu_next_focus = 0;

  lag_pop();

};
