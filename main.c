#include "everything.h"

int main_restart_flag = 0;
int main_shutdown_flag = 0;
int main_terminate_flag = 0;

//--page-split-- main

int main(int argc, char **argv) {

  on_program_start(argc, argv); CHECK_GL_ERROR;

  do {
    main_restart_flag = 0;
    on_open_window(); CHECK_GL_ERROR;
    while (!main_restart_flag && !main_terminate_flag) on_frame();
    on_close_window(); CHECK_GL_ERROR;
  } while (main_restart_flag);

  on_program_exit();

};

//--page-split-- main_restart

void main_restart() {
  main_restart_flag = 1;
};

//--page-split-- main_shutdown

void main_shutdown() {
  main_shutdown_flag = 1;
  if (server_status != SERVER_STATUS_DISCONNECTED) {
    menu_switch(menus_server_disconnect);
  } else {
    if (menu_function_pointer == menus_exit) {
      menus_exit_immediately();
    } else {
      menu_switch(menus_exit);
    };
  };
};
