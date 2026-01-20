#include "everything.h"

//--page-split-- menus_server_connect

void menus_server_connect(void) {

  lag_push(1, "menus_server_connect()");

  if (server_status == SERVER_STATUS_CONNECTING) {
    if (server_connect_time < on_frame_time - 3.0) {
      if (gui_window(32, 3, 1)) {
        server_connect_time = 0.0;
        menu_switch(menus_server_disconnect);
      };
    } else {
      gui_window(32, 3, 0);
    };
    gui_text(-1, 1, "Connecting to server...");
  } else if (server_status == SERVER_STATUS_CONNECTED) {
    if (server_connect_time < on_frame_time - 3.0) {
      if (gui_window(32, 3, 1)) {
        server_connect_time = 0.0;
        menu_switch(menus_server_disconnect);
      };
    } else {
      gui_window(32, 3, 0);
    };
    gui_text(-1, 1, "Negotiating communication...");
  };

  lag_pop();

};
