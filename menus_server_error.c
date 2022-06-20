#include "everything.h"

//--page-split-- menus_server_error

void menus_server_error() {

  lag_push(1, "menus_server_error()");

  int size = 0;
  if (server_error_string != NULL) size = 3 + strlen(server_error_string);
  if (size < 44) size = 44;

  gui_window(size, 5, 0);

  if (server_error_string != NULL) {
    gui_text(-1, 1, server_error_string);
  } else {
    gui_text(-1, 1, "It sucks when there is no error message!");
  };

  if (gui_button(size / 2 + 9, 3, -1, "That sucks!", 0)) {
    server_error_string = NULL;
    menu_switch(menus_server_disconnect);
  };

  if (server_reconnect_time > 0) {

    int wait = ceil(server_reconnect_time - easy_time());
    if (wait > 0) {
      char message[256];
      sprintf(message, "Reconnect in %d seconds...", wait);
      gui_button(size / 2 - 20, 3, -1, message, MENU_FLAG_DISABLE);
    } else {
      if (gui_button(size / 2 - 20, 3, -1, "Reconnect to the server...", 0)) {
        server_error_string = NULL;
        server_action = SERVER_ACTION_CONNECT;
        menu_switch(menus_server_connect);
      };
    };

  };

  lag_pop();

};
