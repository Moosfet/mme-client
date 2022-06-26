#include "everything.h"

static int current_group = -1;

//--page-split-- menus_group

void menus_group() {
  DEBUG("enter menus_group()");

  int the_width = gui_text_columns - 2;
  int the_height = gui_text_lines - 1;

  if (the_width > 76) the_width = 76;
  if (the_height > 22) the_height = 22;

  static int mouse_over;
  int size, across, down, x_offset, y_offset;

  if (gui_window(the_width, the_height, 1)) menu_switch(menus_play);

  // Initialization:  Make current_group always point to a group with blocks,
  // even if the data about the block groups changes (like with new servers).

  if (current_group == -1 || group_data[current_group].name == NULL) {
    for (int i = 0; i < 256; i++) {
      if (group_data[i].name != NULL) {
        current_group = i;
        break;
      };
    };
  };

  if (current_group >= 0) {

    // Draw group buttons...

    int horizontal = 0;
    int vertical = 0;
    for (int i = 0; i < 256; i++) {
      if (group_data[i].name == NULL) continue;
      int size = strlen(group_data[i].name);
      if (vertical == 0) {
        if (horizontal > 0 && horizontal + size + 6 > the_width) {
          vertical += 2; horizontal = 0;
        };
      } else {
        if (horizontal > 0 && horizontal + size + 2 > the_width) {
          vertical += 2; horizontal = 0;
        };
      };
      if (gui_button(horizontal + 1, vertical, 0, group_data[i].name, MENU_FLAG_OFFSET | ((i == current_group) * (MENU_FLAG_ACTIVE | MENU_FLAG_DISABLE)))) current_group = i;
      horizontal += size + 2;
    };

    // Subtract height used for group buttons.

    int usable_offset = (vertical + 2) * 24;
    int usable_height = gui_menu_height - usable_offset - 24;

    // figure out how small the blocks have to be to all fit on the screen

    for (size = 256; size > 0; size--) {
      across = floor((double) gui_menu_width / size);
      down = floor((double) usable_height / size);
      if (across * down >= group_data[current_group].size) break;
    };

    // adjust screen offset so that blocks will be centered...

    x_offset = (gui_menu_width - across * size) / 2;
    y_offset = usable_offset + (usable_height - down * size) / 2;

    // add in the menu offset

    x_offset += gui_menu_x_offset;
    y_offset += gui_menu_y_offset;

  };

  if (menu_process_event) {
    if (KEY_PRESS_EVENT) {
      if (KEY == GLFW_KEY_ESCAPE) menu_switch(menus_play);
    };
    if (RIGHT_PRESS) {
      menu_switch(menus_play);
    };
    if (current_group >= 0) {
      if (WHEEL_EVENT) {
        if (DELTA > 0) {
          for (int i = 0; i < DELTA; i++) {
            for (int j = 1; j < 256; j++) {
              int k = current_group - j;
              if (k < 0) k += 256;
              if (group_data[k].size) {
                current_group = k;
                break;
              };
            };
          };
        } else {
          for (int i = 0; i < -DELTA; i++) {
            for (int j = 1; j < 256; j++) {
              int k = current_group + j;
              if (k >= 256) k -= 256;
              if (group_data[k].size) {
                current_group = k;
                break;
              };
            };
          };
        };
      };
      mouse_over = -1;
      int index = 0;
      for (int y = 0; y < down; y++) {
        for (int x = 0; x < across; x++) {
          if (x + y * across >= group_data[current_group].size) break;
          int s = x_offset + x * size;
          int t = y_offset + y * size;
          if (MOUSE_TEST(s, t, s + size, t + size)) {
            mouse_over = group_data[current_group].list[index];
          };
          index++;
        };
      };

//      if (KEY_PRESS_EVENT) printf ("PRESS ME %i\n", KEY);
//      KEY_PRESS_EVENT && KEY == MOUSE_KEY_LEFT

      if (LEFT_PRESS && mouse_over >= 0) {
        menus_play_create_type = mouse_over;
        menu_switch(menus_play);
      };
    };
  } else {
    if (current_group >= 0) {

      if (mouse_over >= 0) {
        if (block_data[mouse_over].comment != NULL) {
          char status[256];
          snprintf(status, 256, "%d: %s", mouse_over, block_data[mouse_over].comment);
          status[255] = 0;
          gui_text(0, the_height - 1, status);
          //gui_text(0, the_height - 1, block_data[mouse_over].comment);
        };
      };

      glEnable(GL_TEXTURE_2D);

      int index = 0;
      for (int y = 0; y < down; y++) {
        for (int x = 0; x < across; x++) {
          if (x + y * across >= group_data[current_group].size) break;

          glPushMatrix();
          glTranslatef(x_offset + size * (x + 0.5), y_offset + size * (y + 0.5), 0);
          glScalef(size / 2.2, -size / 2.2, 1);

          #define type group_data[current_group].list[index]

          if (type == mouse_over) {
            glDisable(GL_TEXTURE_2D);
            glColor3f(1.0, 0.333, 0.0);
            glLineWidth(3);
            glBegin(GL_LINE_LOOP);
            glVertex2f(-1.1, -1.1);
            glVertex2f(+1.1, -1.1);
            glVertex2f(+1.1, +1.1);
            glVertex2f(-1.1, +1.1);
            glEnd();
            glEnable(GL_TEXTURE_2D);
          };

          #define X0 0
          #define Y0 0
          #define X1 0.866
          #define Y1 0.5
          #define X2 0
          #define Y2 0.95
          #define X3 -0.866
          #define Y3 0.5
          #define X4 -0.823
          #define Y4 -0.475
          #define X5 0
          #define Y5 -1
          #define X6 0.823
          #define Y6 -0.475

          #define TC0 0.0
          #define TC1 1.0
          #define TC2 2.0

          if (block_data[type].visible) {
            glColor3f(1.0, 1.0, 1.0);
            if (texture_data[block_data[type].index[BLOCK_SIDE_UP]].alpha == 1.0) {
              glEnable(GL_BLEND);
              glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            } else if (texture_data[block_data[type].index[BLOCK_SIDE_UP]].alpha > 0.0) {
              glEnable(GL_ALPHA_TEST);
              glAlphaFunc(GL_GEQUAL, texture_data[block_data[type].index[BLOCK_SIDE_UP]].alpha);
            };
            glBindTexture(GL_TEXTURE_2D, texture_data[block_data[type].index[BLOCK_SIDE_UP]].name + RENDER_IN_GRAYSCALE);
            glBegin(GL_QUADS);
            glTexCoord2f(TC1, TC1);
            glVertex2f(X0, Y0);
            glTexCoord2f(TC1, TC2);
            glVertex2f(X1, Y1);
            glTexCoord2f(TC0, TC2);
            glVertex2f(X2, Y2);
            glTexCoord2f(TC0, TC1);
            glVertex2f(X3, Y3);
            glEnd();
            if (texture_data[block_data[type].index[BLOCK_SIDE_UP]].alpha == 1.0) {
              glDisable(GL_BLEND);
            } else if (texture_data[block_data[type].index[BLOCK_SIDE_UP]].alpha > 0.0) {
              glDisable(GL_ALPHA_TEST);
            };
            if (texture_data[block_data[type].index[BLOCK_SIDE_FRONT]].alpha == 1.0) {
              glEnable(GL_BLEND);
              glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            } else if (texture_data[block_data[type].index[BLOCK_SIDE_FRONT]].alpha > 0.0) {
              glEnable(GL_ALPHA_TEST);
              glAlphaFunc(GL_GEQUAL, texture_data[block_data[type].index[BLOCK_SIDE_UP]].alpha);
            };
            glColor3f(0.8, 0.8, 0.8);
            glBindTexture(GL_TEXTURE_2D, texture_data[block_data[type].index[BLOCK_SIDE_FRONT]].name + RENDER_IN_GRAYSCALE);
            glBegin(GL_QUADS);
            glTexCoord2f(TC0, TC0);
            glVertex2f(X4, Y4);
            glTexCoord2f(TC1, 0);
            glVertex2f(X5, Y5);
            glTexCoord2f(TC1, TC1);
            glVertex2f(X0, Y0);
            glTexCoord2f(TC0, TC1);
            glVertex2f(X3, Y3);
            glEnd();
            if (texture_data[block_data[type].index[BLOCK_SIDE_FRONT]].alpha == 1.0) {
              glDisable(GL_BLEND);
            } else if (texture_data[block_data[type].index[BLOCK_SIDE_FRONT]].alpha > 0.0) {
              glDisable(GL_ALPHA_TEST);
            };
            if (texture_data[block_data[type].index[BLOCK_SIDE_RIGHT]].alpha == 1.0) {
              glEnable(GL_BLEND);
              glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            } else if (texture_data[block_data[type].index[BLOCK_SIDE_RIGHT]].alpha > 0.0) {
              glEnable(GL_ALPHA_TEST);
              glAlphaFunc(GL_GEQUAL, texture_data[block_data[type].index[BLOCK_SIDE_RIGHT]].alpha);
            };
            glColor3f(0.6, 0.6, 0.6);
            glBindTexture(GL_TEXTURE_2D, texture_data[block_data[type].index[BLOCK_SIDE_RIGHT]].name + RENDER_IN_GRAYSCALE);
            glBegin(GL_QUADS);
            glTexCoord2f(TC1, TC0);
            glVertex2f(X5, Y5);
            glTexCoord2f(TC2, TC0);
            glVertex2f(X6, Y6);
            glTexCoord2f(TC2, TC1);
            glVertex2f(X1, Y1);
            glTexCoord2f(TC1, TC1);
            glVertex2f(X0, Y0);
            glEnd();
            if (texture_data[block_data[type].index[BLOCK_SIDE_RIGHT]].alpha == 1.0) {
              glDisable(GL_BLEND);
            } else if (texture_data[block_data[type].index[BLOCK_SIDE_RIGHT]].alpha > 0.0) {
              glDisable(GL_ALPHA_TEST);
            };
          };
          glPopMatrix();
          index++;
        };
      };

      glDisable(GL_TEXTURE_2D);

    } else {
      gui_text(-1, -1, "Server has not defined any block groups!");
    };

  };

  DEBUG("leave menus_group()");
};
