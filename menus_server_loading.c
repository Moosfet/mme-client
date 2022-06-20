#include "everything.h"

int menus_server_loading_active = 0;
int menus_server_loading_progress = 0;
char *menus_server_loading_message = NULL;

//--page-split-- menus_server_loading

void menus_server_loading() {

  if (menus_server_loading_message == NULL) {
    if (gui_window(26, 6, 3)) menu_switch(menus_server_disconnect);
    gui_text(1, 0, "Please wait...");
    gui_text(-1, 2, "Receiving map data...");
  } else {
    int width = 3 + strlen(menus_server_loading_message);
    if (width < 26) width = 26;
    if (gui_window(width, 6, 3)) menu_switch(menus_server_disconnect);
    gui_text(1, 0, "Please wait...");
    gui_text(-1, 2, menus_server_loading_message);
  };

  glPushMatrix();
  glTranslated(gui_menu_x_offset, gui_menu_y_offset, 0);

  //chat_color(9);
  //glBegin(GL_QUADS);
  //glVertex2d(gui_menu_width / 2 - 128 - 1, 4.3 * 24 - 1);
  //glVertex2d(gui_menu_width / 2 + 127 + 1, 4.3 * 24 - 1);
  //glVertex2d(gui_menu_width / 2 + 127 + 1, 4.6 * 24 + 1);
  //glVertex2d(gui_menu_width / 2 - 128 - 1, 4.6 * 24 + 1);
  //glEnd();

  glColor4f(0.0, 0.0, 0.0, 1.0);
  glBegin(GL_QUADS);
  glVertex2d(gui_menu_width / 2 - 128 - 3, 4 * 24 + 4);
  glVertex2d(gui_menu_width / 2 + 127 + 3, 4 * 24 + 4);
  glVertex2d(gui_menu_width / 2 + 127 + 3, 5 * 24 - 4);
  glVertex2d(gui_menu_width / 2 - 128 - 3, 5 * 24 - 4);
  glEnd();

  if (menus_server_loading_progress <= 253) {
    chat_color(15);
    glBegin(GL_QUADS);
    glVertex2d(gui_menu_width / 2 + 127, 4 * 24 + 7);
    glVertex2d(gui_menu_width / 2 - 128 + menus_server_loading_progress + 2, 4 * 24 + 7);
    glVertex2d(gui_menu_width / 2 - 128 + menus_server_loading_progress + 2, 5 * 24 - 7);
    glVertex2d(gui_menu_width / 2 + 127, 5 * 24 - 7);
    glEnd();
  };

  if (menus_server_loading_progress >= 2) {
    chat_color(16);
    glBegin(GL_QUADS);
    glVertex2d(gui_menu_width / 2 - 128, 4 * 24 + 7);
    glVertex2d(gui_menu_width / 2 - 128 + menus_server_loading_progress - 2, 4 * 24 + 7);
    glVertex2d(gui_menu_width / 2 - 128 + menus_server_loading_progress - 2, 5 * 24 - 7);
    glVertex2d(gui_menu_width / 2 - 128, 5 * 24 - 7);
    glEnd();
  };

  glPopMatrix();

};
