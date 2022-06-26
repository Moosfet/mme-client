#include "everything.h"

static double termination_time = 0;
static int cache = 0, nag = 0;

//--page-split-- menus_exit_immediately

void menus_exit_immediately() {
  nag = 0; cache = 1;
  termination_time = on_frame_time - 1.0;
};

//--page-split-- menus_exit

void menus_exit() {

  glfw_fullscreen(0);

  if (!cache) {
    nag = option_advertise;
    cache = 1;
  };

  if (!nag) {
    if (termination_time == 0) termination_time = on_frame_time;

    if (on_frame_time > termination_time + 0.5) main_terminate_flag = 1;

    gui_window(22, 3, 0);
    gui_text(-1, 1, "Thanks for playing!");
  } else {
    if (gui_window(57, 15, 3)) nag = 0;

    if (KEY_PRESS_EVENT) {
      if (KEY == GLFW_KEY_ESCAPE) nag = 0;
    };

    gui_text(-1, 0, "Please help us advertise Multiplayer Map Editor");

    gui_text(2, 2, "Do you like Multiplayer Map Editor, but just wish");
    gui_text(2, 3, "that more people played the game?  You can help.");

    gui_text(2, 5, "Just share the link on Facebook so that your friends");
    gui_text(2, 6, "and your friend's friends will play the game as well.");

    gui_text(-1, 8, "http://www.multiplayermapeditor.com/");

    if (gui_button(2, 10, 53, "Awesome idea!  Remind me next time in case I forget.", MENU_FLAG_OFFSET)) {
      nag = 0;
    };
    if (gui_button(2, 13, 53, "I already have.  No need to remind me about it again.", FLAGS)) {
      option_advertise = 0;
      nag = 0;
    };

  };

};
