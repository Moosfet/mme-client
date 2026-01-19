#include "everything.h"

//--page-split-- menus_server_disconnect

void menus_server_disconnect() {

  lag_push(1, "menus_disconnect()");

  static double disconnect_time = 0;

  if (server_action != SERVER_ACTION_DISCONNECT) {
    server_action = SERVER_ACTION_DISCONNECT;
    disconnect_time = easy_time();
  };

  if (server_status == SERVER_STATUS_DISCONNECTED) {
    if (main_shutdown_flag || !strcmp(server_address, server_portal_address)) {
      main_shutdown();
    } else {
      memory_allocate(&server_address, 0);
      server_action = SERVER_ACTION_CONNECT;
      menu_switch(menus_server_connect);
    };
  } else {
    gui_window(32, 3, 0);
    gui_text(-1, 1, "Disconnecting from server...");
  };

  lag_pop();

};
