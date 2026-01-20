#include "everything.h"

// Then probably read menu.c and menus.c to understand the context in which
// all of this shit is used.  It's an unfortunate mess of macros, but those
// macros do make it all a lot easier, particularly the event processing.

//  Frequently used tiny variabes:
//
//  ps = pixel offset of left edge
//  pt = pixel offset of top edge
//  pu = pixel offset of right edge
//  pv = pixel offset of bottom edge
//
//  Usually they refer to the text, and so things are drawn outside them.

// Let's pretend like the font can be just swapped out for another!
#define CHARACTER_WIDTH 12
#define CHARACTER_HEIGHT 24
#define CHARACTER_X_OFFSET 16
#define CHARACTER_Y_OFFSET 32
#define CHARACTER_X_OVERLAP 2
#define CHARACTER_Y_OVERLAP 4

// Make trying out new color schemes a little easier.
#define COLOR_ORDINARY glColor3f(1.0, 1.0, 1.0)
#define COLOR_CLICKABLE chat_color(14)
#define COLOR_HOVER chat_color(16)
#define COLOR_TOUCH chat_color(16)
#define COLOR_ACTIVE chat_color(15)
#define COLOR_DISABLED chat_color(13)

static GLuint widgets;
static GLuint old_letters;
static GLuint new_letters;
static GLuint background;
static int background_width;
static int background_height;

#define texture_quad(A, B, C, D, E, F, G, H) glTexCoord2f(A, B); glVertex2f(E, F); glTexCoord2f(C, B); glVertex2f(G, F); glTexCoord2f(C, D); glVertex2f(G, H); glTexCoord2f(A, D); glVertex2f(E, H);

//--page-split-- gui_open_window

void gui_open_window(void) {
  extern char data_gui_png; extern int size_gui_png;
  extern char data_old_font_png; extern int size_old_font_png;
  extern char data_new_font_png; extern int size_new_font_png;
  widgets = texture_load(&data_gui_png, size_gui_png, TEXTURE_FLAG_NOFLIP);
  old_letters = texture_load(&data_old_font_png, size_old_font_png, TEXTURE_FLAG_NOFLIP | TEXTURE_FLAG_MIPMAP);
  new_letters = texture_load(&data_new_font_png, size_new_font_png, TEXTURE_FLAG_NOFLIP | TEXTURE_FLAG_MIPMAP);
  extern char data_blackbook_png; extern int size_blackbook_png;
  background = texture_load(&data_blackbook_png, size_blackbook_png, 0);
  background_width = texture_width;
  background_height = texture_height;
};

//--page-split-- gui_close_window

void gui_close_window(void) {

  glDeleteTextures(1, &widgets);
  glDeleteTextures(1, &old_letters);
  glDeleteTextures(1, &new_letters);
  glDeleteTextures(1, &background);
};

int gui_menu_x_offset;
int gui_menu_y_offset;
int gui_menu_width;
int gui_menu_height;
int gui_text_columns;
int gui_text_lines;

//--page-split-- gui_window

int gui_window(int x, int y, int flags) {
  // Draws a window of size (x, y) and also arranges coordinate
  // space so that (0, 0) is the top-left of the window.
  // Returns true when a click is received *outside* this window,
  // or in other words, when the user clicks back into the game.

  int fullpush = 0;

  #define semipush menu_object_data[menu_object_index][0]
  #define drawdown menu_object_data[menu_object_index][1]
  #define hoverglow menu_object_data[menu_object_index][2]

  int pixel_width = x * CHARACTER_WIDTH;
  int pixel_height = y * CHARACTER_HEIGHT;

  int ps = (display_window_width - pixel_width) / 2;
  int pt = (display_window_height - pixel_height) / 2;
  int pu = ps + pixel_width;
  int pv = pt + pixel_height;

  if (++menu_object_index >= MENU_MAX_OBJECTS) easy_fuck("Too many menu objects.");

  #define extra_x 7
  #define extra   6
  #define extra_y 5
  #define downsize 28

  if (menu_process_event) {

    if (MOUSE_TEST(pu-downsize+1,pt-3,pu+extra-3,pt+downsize+2)) {
      if (LEFT_PRESS) {

        semipush = 1;
        drawdown = 1;
      };
      if (LEFT_RELEASE) {
        if (semipush) fullpush = 1;
        drawdown = 0;
        semipush = 0;
      };
      if (HOVER_EVENT && semipush) drawdown = 1;
      hoverglow = 1;

    } else {
      if (LEFT_RELEASE) semipush = 0;
      drawdown = 0;
      hoverglow = 0;

    };

    if (menu_focus_object == menu_object_index) {
      if (KEY_PRESS_EVENT) {
        if (KEY == GLFW_KEY_TAB) {
          if (menu_modifier_shift) {
            menu_next_focus = menu_focus_object - 1;

          } else {
            menu_next_focus = menu_focus_object + 1;

          };
        };
      };
    };

  };

  if (menu_draw_widget) {

    double x[4], y[4], ts, rs;
    if ((flags & 3) == 0) {
      x[0] =  68 / 256.0;
      x[1] =  92 / 256.0;
      x[2] = 100 / 256.0;
      x[3] = 124 / 256.0;
      y[0] =   4 / 256.0;
      y[1] =  28 / 256.0;
      y[2] =  36 / 256.0;
      y[3] =  60 / 256.0;
    } else if ((flags & 3) == 1) {
      x[0] =   4 / 256.0;
      x[1] =  28 / 256.0;
      x[2] =  36 / 256.0;
      x[3] =  92 / 256.0;
      y[0] =  68 / 256.0;
      y[1] = 124 / 256.0;
      y[2] = 132 / 256.0;
      y[3] = 156 / 256.0;
    } else if ((flags & 3) == 2) {
      x[0] = 100 / 256.0;
      x[1] = 124 / 256.0;
      x[2] = 132 / 256.0;
      x[3] = 156 / 256.0;
      y[0] =  68 / 256.0;
      y[1] = 124 / 256.0;
      y[2] = 132 / 256.0;
      y[3] = 156 / 256.0;
    } else if ((flags & 3) == 3) {
      x[0] = 164 / 256.0;
      x[1] = 188 / 256.0;
      x[2] = 196 / 256.0;
      x[3] = 252 / 256.0;
      y[0] =  68 / 256.0;
      y[1] = 124 / 256.0;
      y[2] = 132 / 256.0;
      y[3] = 156 / 256.0;
    };
    rs = 256 * (x[3] - x[2]) - 24;
    ts = 256 * (y[1] - y[0]) - 24;

    //printf("x0=%0.3f x1=%0.3f x2=%0.3f x3=%0.3f y0=%0.3f y1=%0.3f y2=%0.3f y3=%0.3f rs=%0.0f ts=%0.0f\n", x[0], x[1], x[2], x[3], y[0], y[1], y[2], y[3], rs, ts);

    double tx = (pixel_width + 2.0 * extra_x) / background_width;
    double ty = (pixel_height + 2.0 * extra_y) / background_height;

    glPushMatrix();
    glTranslated(0.0, 1.0, 0.0);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, background + RENDER_IN_GRAYSCALE);
    glColor4f(0.5, 0.5, 0.5, 0.8);
    glBegin(GL_QUADS);
    texture_quad(-tx, -ty, +tx, +ty, ps - extra_x, pt - extra_y, pu + extra_x, pv + extra_y);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, widgets + RENDER_IN_GRAYSCALE);

    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_QUADS);

    texture_quad(x[0], y[3], x[1], y[2], ps - (12 + extra_x), pv + (12 + extra_y), ps + (12 - extra_x), pv - (12 - extra_y));
    texture_quad(x[1], y[3], x[2], y[2], ps + (12 - extra_x), pv + (12 + extra_y), pu - (12 - extra_x) - rs, pv - (12 - extra_y));
    texture_quad(x[2], y[3], x[3], y[2], pu - (12 - extra_x) - rs, pv + (12 + extra_y), pu + (12 + extra_x), pv - (12 - extra_y));
    texture_quad(x[2], y[2], x[3], y[1], pu - (12 - extra_x) - rs, pv - (12 - extra_y), pu + (12 + extra_x), pt + (12 - extra_y) + ts);
    texture_quad(x[2], y[1], x[3], y[0], pu - (12 - extra_x) - rs, pt + (12 - extra_y) + ts, pu + (12 + extra_x), pt - (12 + extra_y));
    texture_quad(x[1], y[1], x[2], y[0], ps + (12 - extra_x), pt + (12 - extra_y) + ts, pu - (12 - extra_x) - rs, pt - (12 + extra_y));
    texture_quad(x[0], y[1], x[1], y[0], ps - (12 + extra_x), pt + (12 - extra_y) + ts, ps + (12 - extra_x), pt - (12 + extra_y));
    texture_quad(x[0], y[2], x[1], y[1], ps - (12 + extra_x), pv - (12 - extra_y), ps + (12 - extra_x), pt + (12 - extra_y) + ts);

    glEnd();
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

  };

  gui_menu_x_offset = ps;
  gui_menu_y_offset = pt;
  gui_menu_width = pixel_width;
  gui_menu_height = pixel_height;

  return fullpush && (flags & 1);

  #undef semipush
  #undef drawdown
  #undef hoverglow
  #undef extra

};

//--page-split-- gui_model_name

void gui_model_name(char *string) {
  // for use from model.c to draw player names
  gui_menu_width = 0;
  gui_menu_height = 0;
  gui_menu_x_offset = 0;
  gui_menu_y_offset = 0;
  menu_draw_widget = 1;
  glCullFace(GL_FRONT);
  gui_draw_text(-1, 0, string, NULL, -1);
  glCullFace(GL_BACK);
};

//--page-split-- gui_text

void gui_text(int column, int row, char *string) {
  char *s = NULL; char *c = NULL;
  chat_color_decode(&s, &c, string, FALSE);
  gui_draw_text(column, row, s, c, 0);
  memory_allocate(&s, 0);
  memory_allocate(&c, 0);
};

//--page-split-- gui_draw_text

void gui_draw_text(int column, int row, char *string, char *colors, int flags) {
  // Draws text without selecting a color, as
  // it is presumed you have done so already.

  int pixel_width, pixel_height;
  int pixel_column, pixel_row;

  if (menu_draw_widget) {

    pixel_width = CHARACTER_WIDTH * strlen(string);
    pixel_height = CHARACTER_HEIGHT;

    if (column >= 0) {
      pixel_column = CHARACTER_WIDTH * column;
    } else {
      pixel_column = (gui_menu_width - pixel_width) / 2;
    };

    pixel_row = CHARACTER_HEIGHT * row;

    if (flags & MENU_FLAG_OFFSET) pixel_row += CHARACTER_HEIGHT / 2;

    pixel_row += gui_menu_y_offset;
    pixel_column += gui_menu_x_offset;

    double fxi = 1.0 / 32;
    double fyi = 1.0 / 8;

    double fx, fy;

    if (option_blender_font) {
      glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, new_letters + RENDER_IN_GRAYSCALE);
    } else {
      glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, old_letters + RENDER_IN_GRAYSCALE);
    };
    if (flags < 0) {
      glEnable(GL_ALPHA_TEST); glAlphaFunc(GL_GEQUAL, 0.75);
    } else {
      glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    };

    int current_color = -1;

    glBegin(GL_QUADS);
    while (*string) {
      fx = fmod(((unsigned char) *string) * fxi, 1.0);
      fy = floor(((unsigned char) *string) * fxi) * fyi;
      if (colors != NULL && (*colors != current_color || *colors == 11 || (*colors >= 13 && *colors <= 16))) chat_color(*colors), current_color = *colors;
      texture_quad(fx, fy, fx+fxi, fy + fyi, pixel_column-CHARACTER_X_OVERLAP, pixel_row-CHARACTER_Y_OVERLAP, pixel_column+CHARACTER_WIDTH+CHARACTER_X_OVERLAP, pixel_row+CHARACTER_HEIGHT+CHARACTER_Y_OVERLAP);
      pixel_column += CHARACTER_WIDTH;
      string++; if (colors != NULL) colors++;
    };
    glEnd();

    if (flags < 0) {
      glDisable(GL_ALPHA_TEST);
    } else {
      glDisable(GL_BLEND);
    };
    glDisable(GL_TEXTURE_2D);

  };

};

//--page-split-- gui_button

int gui_button(int x, int y, int w, char *string, int flags) {
  // (x, y) is position for lettering on button.
  // Column may be -1 to center the button in the window.
  // w specifies the width of the button in characters,
  // and may be -1 to make the button as large as the string.
  // The text is always centered on the button.
  // Returns true when the button is clicked.

  int fullpush = 0;

  #define semipush menu_object_data[menu_object_index][0]
  #define drawdown menu_object_data[menu_object_index][1]
  #define hoverglow menu_object_data[menu_object_index][2]

  if (w < (int) strlen(string)) w = strlen(string);

  int pixel_width = CHARACTER_WIDTH * w;
  int ps = CHARACTER_WIDTH * x;
  int pt = CHARACTER_HEIGHT * y;
  if (flags & MENU_FLAG_OFFSET) pt += CHARACTER_HEIGHT / 2;
  int pu = ps + pixel_width;
  int pv = pt + CHARACTER_HEIGHT;

  if (x < 0) {
    ps = (gui_menu_width - pixel_width) / 2;
    pu = (gui_menu_width + pixel_width) / 2;
  };

  ps += gui_menu_x_offset; pt += gui_menu_y_offset;
  pu += gui_menu_x_offset; pv += gui_menu_y_offset;

  #define test 8

  if (++menu_object_index >= MENU_MAX_OBJECTS) easy_fuck("Too many menu objects.");

  if (menu_process_event) {

    if (MOUSE_TEST(ps-test,pt-test,pu+test,pv+test)) {
      if (LEFT_PRESS) {
        semipush = 1;
        drawdown = 1;
      };
      if (LEFT_RELEASE) {
        if (semipush) fullpush = 1;
        drawdown = 0;
        semipush = 0;
      };
      if (HOVER_EVENT && semipush) drawdown = 1;
      hoverglow = 1;
    } else {
      if (LEFT_RELEASE) semipush = 0;
      drawdown = 0;
      hoverglow = 0;
    };

    if (menu_focus_object == menu_object_index) {
      if (KEY_PRESS_EVENT) {
        if (KEY == GLFW_KEY_TAB) {
          if (menu_modifier_shift) {
            menu_next_focus = menu_focus_object - 1;
          } else {
            menu_next_focus = menu_focus_object + 1;
          };
        };
        if (KEY == GLFW_KEY_SPACE || KEY == GLFW_KEY_ENTER || KEY == GLFW_KEY_KP_ENTER) {
          fullpush = 1;
        };
      };
    };

    if (flags & MENU_FLAG_DISABLE) {
      semipush = 0; drawdown = 0; hoverglow = 0; fullpush = 0;
    };

  };

  #undef test

  if (menu_draw_widget) {

    if (drawdown) {
      glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, widgets + RENDER_IN_GRAYSCALE);
      glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      COLOR_TOUCH;
      glBegin(GL_QUADS);
      #define X3 (14 / 256.0)
      #define X2 (26 / 256.0)
      #define X1 (38 / 256.0)
      #define X0 (50 / 256.0)
      #define Y3 (14 / 256.0)
      #define Y2 (26 / 256.0)
      #define Y1 (38 / 256.0)
      #define Y0 (50 / 256.0)
      texture_quad(X0, Y0, X1, Y1, ps-12, pt-12, ps, pt);
      texture_quad(X1, Y0, X2, Y1, ps, pt-12, pu, pt);
      texture_quad(X2, Y0, X3, Y1, pu, pt-12, pu+12, pt);
      texture_quad(X2, Y1, X3, Y2, pu, pt, pu+12, pv);
      texture_quad(X2, Y2, X3, Y3, pu, pv, pu+12, pv+12);
      texture_quad(X1, Y2, X2, Y3, ps, pv, pu, pv+12);
      texture_quad(X0, Y2, X1, Y3, ps-12, pv, ps, pv+12);
      texture_quad(X0, Y1, X1, Y2, ps-12, pt, ps, pv);
      texture_quad(X1, Y1, X2, Y2, ps, pt, pu, pv);
      #undef X0
      #undef X1
      #undef X2
      #undef X3
      #undef Y0
      #undef Y1
      #undef Y2
      #undef Y3
      glEnd();
      glDisable(GL_BLEND);
      glDisable(GL_TEXTURE_2D);
      glPushMatrix();
      glTranslated(+1, +1, 0);
      int extra_space = w - strlen(string);
      if (x >= 0 && extra_space > 0) {
        glPushMatrix();
        glTranslated(extra_space * 6.0, 0, 0);
        COLOR_ORDINARY; gui_draw_text(x, y, string, NULL, flags);
        glPopMatrix();
      } else {
        COLOR_ORDINARY; gui_draw_text(x, y, string, NULL, flags);
      };
      glPopMatrix();
    } else {
      glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, widgets + RENDER_IN_GRAYSCALE);
      glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      if (hoverglow || menu_focus_object == menu_object_index) {
        COLOR_HOVER;
      } else if (flags & MENU_FLAG_ACTIVE) {
        COLOR_ACTIVE;
      } else if (flags & MENU_FLAG_DISABLE) {
        COLOR_DISABLED;
      } else {
        COLOR_CLICKABLE;
      };
      glBegin(GL_QUADS);
      #define X0 (14 / 256.0)
      #define X1 (26 / 256.0)
      #define X2 (38 / 256.0)
      #define X3 (50 / 256.0)
      #define Y0 (14 / 256.0)
      #define Y1 (26 / 256.0)
      #define Y2 (38 / 256.0)
      #define Y3 (50 / 256.0)
      texture_quad(X0, Y0, X1, Y1, ps-12, pt-12, ps, pt);
      texture_quad(X1, Y0, X2, Y1, ps, pt-12, pu, pt);
      texture_quad(X2, Y0, X3, Y1, pu, pt-12, pu+12, pt);
      texture_quad(X2, Y1, X3, Y2, pu, pt, pu+12, pv);
      texture_quad(X2, Y2, X3, Y3, pu, pv, pu+12, pv+12);
      texture_quad(X1, Y2, X2, Y3, ps, pv, pu, pv+12);
      texture_quad(X0, Y2, X1, Y3, ps-12, pv, ps, pv+12);
      texture_quad(X0, Y1, X1, Y2, ps-12, pt, ps, pv);
      texture_quad(X1, Y1, X2, Y2, ps, pt, pu, pv);
      #undef X0
      #undef X1
      #undef X2
      #undef X3
      #undef Y0
      #undef Y1
      #undef Y2
      #undef Y3
      glEnd();
      glDisable(GL_BLEND);
      glDisable(GL_TEXTURE_2D);
      int extra_space = w - strlen(string);
      if (x >= 0 && extra_space > 0) {
        glPushMatrix();
        glTranslated(extra_space * 6.0, 0, 0);
        COLOR_ORDINARY; gui_draw_text(x, y, string, NULL, flags);
        glPopMatrix();
      } else {
        COLOR_ORDINARY; gui_draw_text(x, y, string, NULL, flags);
      };
    };

  };
  if (fullpush) sound_play (SOUND_BUTTON_PRESS, 0.1f, NULL);
  return fullpush;

  #undef semipush
  #undef drawdown

};

//--page-split-- gui_radio

int gui_radio(int x, int y, int *p, int v, char *string) {
  // (x, y) specifies location of text next to button
  // *p is a variable to write v into when button is selected.
  // returns true if value is changed

  #define semipush menu_object_data[menu_object_index][0]
  #define drawdown menu_object_data[menu_object_index][1]
  #define hoverglow menu_object_data[menu_object_index][2]

  int ps = CHARACTER_WIDTH * (x - 3);
  int pt = CHARACTER_HEIGHT * y;
  int pu = CHARACTER_WIDTH * (x + strlen(string));
  int pv = CHARACTER_HEIGHT * (y + 1);

  ps += gui_menu_x_offset; pt += gui_menu_y_offset;
  pu += gui_menu_x_offset; pv += gui_menu_y_offset;

  #define offset 0
  #define test 0

  if (++menu_object_index >= MENU_MAX_OBJECTS) easy_fuck("Too many menu objects.");

  int change = 0;

  if (menu_process_event) {

    if (MOUSE_TEST(ps-test,pt-test+offset,ps+2*CHARACTER_WIDTH+test,pv+test+offset) || MOUSE_TEST(ps+2*CHARACTER_WIDTH-test,pt-test,pu+test,pv+test)) {
      if (LEFT_PRESS) {
        semipush = 1;
        drawdown = 1;
      };
      if (LEFT_RELEASE) {
        if (semipush && *p != v) *p = v, change = 1;
        drawdown = 0;
        semipush = 0;
      };
      if (HOVER_EVENT && semipush) drawdown = 1;
      hoverglow = 1;
    } else {
      if (LEFT_RELEASE) semipush = 0;
      drawdown = 0;
      hoverglow = 0;
    };

    if (menu_focus_object == menu_object_index) {
      if (KEY_PRESS_EVENT) {
        if (KEY == GLFW_KEY_TAB) {
          if (menu_modifier_shift) {
            menu_next_focus = menu_focus_object - 1;
          } else {
            menu_next_focus = menu_focus_object + 1;
          };
        };
        if (KEY == GLFW_KEY_SPACE || KEY == GLFW_KEY_ENTER || KEY == GLFW_KEY_KP_ENTER) {
          *p = v; change = 1;
        };
      };
    };

  };

  #undef test

  if (menu_draw_widget) {

    glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, widgets + RENDER_IN_GRAYSCALE);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(0.0, 0.8, 0.0);
    glBegin(GL_QUADS);
    if (hoverglow || menu_focus_object == menu_object_index) {
      COLOR_HOVER;
    } else {
      COLOR_CLICKABLE;
    };
    texture_quad(0.5, 0.125, 0.5 + 24 / 256.0, 0.125 + 24 / 256.0, ps, pt + offset, ps + 24, pv + offset);
    if (*p == v || drawdown) {
      if (*p == v) {
        COLOR_ACTIVE;
      } else {
        COLOR_TOUCH;
      };
      texture_quad(0.625, 0.125, 0.625 + 24 / 256.0, 0.125 + 24 / 256.0, ps, pt + offset, ps + 24, pv + offset);
    };
    glEnd();
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    if (hoverglow || menu_focus_object == menu_object_index) {
      COLOR_HOVER;
    } else {
      COLOR_ORDINARY;
    };
    gui_draw_text(x, y, string, NULL, 0);

  };

  #undef offset

  #undef semipush
  #undef drawdown
  #undef hoverglow
  if (change) sound_play (SOUND_BUTTON_PRESS, 0.1f, NULL);
  return change;

};

//--page-split-- gui_check

int gui_check(int x, int y, int *p, int v, char *string) {
  // (x, y) is location of text next to check box
  // *p is where state of check box is stored.
  // v is value of *p when check box is marked.
  // Both *p and v will be treated as true/false values.
  // return value is true when value is changed.

  #define semipush menu_object_data[menu_object_index][0]
  #define drawdown menu_object_data[menu_object_index][1]
  #define hoverglow menu_object_data[menu_object_index][2]

  int ps = CHARACTER_WIDTH * (x - 3);
  int pt = CHARACTER_HEIGHT * y;
  int pu = CHARACTER_WIDTH * (x + strlen(string));
  int pv = CHARACTER_HEIGHT * (y + 1);

  ps += gui_menu_x_offset; pt += gui_menu_y_offset;
  pu += gui_menu_x_offset; pv += gui_menu_y_offset;

  #define offset -3
  #define test -1

  if (++menu_object_index >= MENU_MAX_OBJECTS) easy_fuck("Too many menu objects.");

  int change = 0;

  if (menu_process_event) {

    if (MOUSE_TEST(ps-test,pt-test+offset,ps+2*CHARACTER_WIDTH+test,pv+test+offset) || MOUSE_TEST(ps+2*CHARACTER_WIDTH-test,pt-test,pu+test,pv+test)) {
      if (LEFT_PRESS) {
        semipush = 1;
        drawdown = 1;
      };
      if (LEFT_RELEASE) {
        if (semipush) *p = !*p, change = 1;
        drawdown = 0;
        semipush = 0;
      };
      if (HOVER_EVENT && semipush) drawdown = 1;
      hoverglow = 1;
    } else {
      if (LEFT_RELEASE) semipush = 0;
      drawdown = 0;
      hoverglow = 0;
    };

    if (menu_focus_object == menu_object_index) {
      if (KEY_PRESS_EVENT) {
        if (KEY == GLFW_KEY_TAB) {
          if (menu_modifier_shift) {
            menu_next_focus = menu_focus_object - 1;
          } else {
            menu_next_focus = menu_focus_object + 1;
          };
        };
        if (KEY == GLFW_KEY_SPACE || KEY == GLFW_KEY_ENTER || KEY == GLFW_KEY_KP_ENTER) {
          *p = !*p; change = 1;
        };
      };
    };

  };

  #undef test

  if (menu_draw_widget) {

    glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, widgets + RENDER_IN_GRAYSCALE);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(0.0, 0.8, 0.0);
    glBegin(GL_QUADS);
    if (hoverglow || menu_focus_object == menu_object_index) {
      COLOR_HOVER;
    } else {
      COLOR_CLICKABLE;
    };
    texture_quad(0.5, 0.0, 0.5 + 24 / 256.0, 24 / 256.0, ps, pt + offset, ps + 24, pv + offset);
    if (!*p == !v || drawdown) {
      if (!*p == !v) {
        COLOR_ACTIVE;
      } else {
        COLOR_TOUCH;
      };
      texture_quad(0.625, 0.0, 0.625 + 24 / 256.0, 24 / 256.0, ps, pt + offset, ps + 24, pv + offset);
    };
    glEnd();
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    if (hoverglow || menu_focus_object == menu_object_index) {
      COLOR_HOVER;
    } else {
      COLOR_ORDINARY;
    };
    gui_draw_text(x, y, string, NULL, 0);

  };

  #undef offset

  #undef semipush
  #undef drawdown
  #undef hoverglow
  if (change) sound_play (SOUND_BUTTON_PRESS, 0.1f, NULL);
  return change;

};

//--page-split-- gui_input

int gui_input(int x, int y, int size, int length, char *string, int flags) {
  // (x, y) is location of left-most character in box.
  // size is the size of the box in characters.
  // length is the maximum length of the input.
  // return value is 1 if a change is made, 2 if enter was pressed

  if (size < 1) return 0;

  char visible[size+1];

  #define cursor menu_object_data[menu_object_index][0]
  #define offset menu_object_data[menu_object_index][1]
  #define hoverglow menu_object_data[menu_object_index][2]
  #define lastfocus menu_object_data[menu_object_index][3]
  #define blinktime *((double*) &menu_object_data[menu_object_index][4])

  int pixel_width = CHARACTER_WIDTH * size;
  int pixel_height = CHARACTER_HEIGHT;

  int ps = CHARACTER_WIDTH * x;
  int pt = CHARACTER_HEIGHT * y;
  if (flags & MENU_FLAG_OFFSET) pt += CHARACTER_HEIGHT / 2;
  int pu = ps + pixel_width;
  int pv = pt + pixel_height;

  ps += gui_menu_x_offset; pt += gui_menu_y_offset;
  pu += gui_menu_x_offset; pv += gui_menu_y_offset;

  #define test 8

  if (++menu_object_index >= MENU_MAX_OBJECTS) easy_fuck("Too many menu objects.");

  int current = strlen(string);
  if (cursor > current) cursor = current;

  int change = 0;
  int enter = 0;

  if (menu_process_event) {

    if (menu_focus_object == menu_object_index && !lastfocus) {
      blinktime = on_frame_time;
    };
    lastfocus = menu_focus_object == menu_object_index;

    if (MOUSE_TEST(ps-test,pt-test,pu+test,pv+test)) {
      if (LEFT_PRESS) {
        menu_next_focus = menu_object_index;
        for (int i = 0; i <= size; i++) {
          int is = ps + i * CHARACTER_WIDTH - CHARACTER_WIDTH / 2;
          if (MOUSE_TEST(is, pt-test, is + CHARACTER_WIDTH, pv+test)) {
            cursor = offset + i;
            if (cursor > current) cursor = current;
          };
        };
        blinktime = on_frame_time;
      };
      hoverglow = 1;
    } else {
      hoverglow = 0;
    };

    if (menu_focus_object == menu_object_index) {

      if (KEY_PRESS_EVENT) {
        blinktime = on_frame_time;
        if (KEY == GLFW_KEY_TAB) {
          if (menu_modifier_shift) {
            menu_next_focus = menu_focus_object - 1;
          } else {
            menu_next_focus = menu_focus_object + 1;
          };
        };
      };
      if (KEY_PRESS_EVENT || KEY_REPEAT_EVENT) {
        if (KEY == GLFW_KEY_LEFT) {
          if (cursor > 0) cursor--;
        };
        if (KEY == GLFW_KEY_RIGHT) {
          if (cursor < current) cursor++;
        };
        if (KEY == GLFW_KEY_DELETE) {
          if (menu_modifier_shift) {
            if (string[cursor] != 0) {
              string[cursor] = 0;
              change = 1;
            };
          } else {
            if (cursor < current) {
              memmove(string + cursor, string + cursor + 1, length - cursor);
              change = 1;
            };
          };
        };
        if (KEY == GLFW_KEY_BACKSPACE) {
          if (cursor > 0) {
            cursor--;
            memmove(string + cursor, string + cursor + 1, length - cursor);
            change = 1;
          };
        };
        if (KEY == GLFW_KEY_HOME) {
          cursor = 0;
        };
        if (KEY == GLFW_KEY_END) {
          cursor = current;
        };
        if (KEY == GLFW_KEY_ENTER || KEY == GLFW_KEY_KP_ENTER) {
          menu_next_focus = menu_focus_object + 1;
          enter = 1;
        };
        if (KEY == GLFW_KEY_ESCAPE) {
          menu_next_focus = 0;
        };
        if ((KEY == GLFW_KEY_INSERT && menu_modifier_shift) || (KEY == GLFW_KEY_V && menu_modifier_control)) {
          char *clipboard = NULL;
          char *text = (char *) glfwGetClipboardString(NULL);
          if (text) {
            char *p = text;
            memory_allocate(&clipboard, strlen(text) + 1);
            char *output = clipboard;
            while (*p) {
              int codepoint, count;
              if ((*p & 0x80) == 0x00) { count = 0; codepoint = *p & 0x7F; };
              if ((*p & 0xC0) == 0x80) break; // invalid
              if ((*p & 0xE0) == 0xC0) { count = 1; codepoint = *p & 0x1F; };
              if ((*p & 0xF0) == 0xE0) { count = 2; codepoint = *p & 0x0F; };
              if ((*p & 0xF8) == 0xF0) { count = 3; codepoint = *p & 0x07; };
              if ((*p & 0xF8) == 0xF0) break; // invalid
              for (int i = 1; i <= count; i++) {
                if (!p[i] || (p[i] & 0xC0) != 0x80) break;
                codepoint <<= 6;
                codepoint |= p[i] & 0x3F;
              };
              *output = event_translate_from_unicode(codepoint);
              if (*output) output++;
              p += count + 1;
            };
            if (*p) {
              fprintf(stderr, "Invalid Unicode received from clipboard.\n");
            } else {
              *output = 0;
              int l = output - clipboard;
              for (int i = 0; i < l; i++) {
                if (current < length) {
                  memmove(string + cursor + 1, string + cursor, length - cursor);
                  string[cursor] = clipboard[i];
                  cursor++;
                  change = 1;
                  current++;
                };
              };
            };
            memory_allocate(&clipboard, 0);
          };
        };
      };
      if (CHARACTER_EVENT) {
        if (current < length) {
          memmove(string + cursor + 1, string + cursor, length - cursor);
          string[cursor] = CHARACTER;
          cursor++;
          change = 1;
        };
      };
    };

  };

  #undef test

  current = strlen(string);
  if (cursor > size + offset) offset = cursor - size;
  if (cursor < offset) offset = cursor;
  if (offset > current - size) offset = current - size;
  if (offset < 0) offset = 0;
  if (current <= size) {
    strcpy(visible, string);
  } else {
    memmove(visible, string + offset, size);
    visible[size] = 0;
  };

  if (menu_draw_widget) {

    glDisable(GL_BLEND);

    glColor3f(0.0, 0.0, 0.0);

    #define extra 7
    glBegin(GL_QUADS);
    glVertex2f(ps - extra, pt - extra);
    glVertex2f(pu + extra, pt - extra);
    glVertex2f(pu + extra, pv + extra);
    glVertex2f(ps - extra, pv + extra);
    glEnd();
    #undef extra

    glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, widgets + RENDER_IN_GRAYSCALE);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (menu_focus_object == menu_object_index || hoverglow) {
      COLOR_HOVER;
    } else {
      COLOR_ORDINARY;
    };

    glBegin(GL_QUADS);
    #define X0 ( 68 / 256.0)
    #define X1 ( 92 / 256.0)
    #define X2 (100 / 256.0)
    #define X3 (124 / 256.0)
    #define Y0 (  4 / 256.0)
    #define Y1 ( 24 / 256.0)
    #define Y2 ( 40 / 256.0)
    #define Y3 ( 60 / 256.0)
    #define TWENTY 18
    #define ZERO 2
    texture_quad(X0, Y0, X1, Y1, ps-TWENTY, pt-TWENTY, ps+ZERO, pt+ZERO);
    texture_quad(X1, Y0, X2, Y1, ps+ZERO, pt-TWENTY, pu-ZERO, pt+ZERO);
    texture_quad(X2, Y0, X3, Y1, pu-ZERO, pt-TWENTY, pu+TWENTY, pt+ZERO);
    texture_quad(X2, Y1, X3, Y2, pu-ZERO, pt+ZERO, pu+TWENTY, pv-ZERO);
    texture_quad(X2, Y2, X3, Y3, pu-ZERO, pv-ZERO, pu+TWENTY, pv+TWENTY);
    texture_quad(X1, Y2, X2, Y3, ps+ZERO, pv-ZERO, pu-ZERO, pv+TWENTY);
    texture_quad(X0, Y2, X1, Y3, ps-TWENTY, pv-ZERO, ps+ZERO, pv+TWENTY);
    texture_quad(X0, Y1, X1, Y2, ps-TWENTY, pt+ZERO, ps+ZERO, pv-ZERO);
    #undef X0
    #undef X1
    #undef X2
    #undef X3
    #undef Y0
    #undef Y1
    #undef Y2
    #undef Y3
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    COLOR_ORDINARY;

    if (flags & MENU_FLAG_STARS) {
      int j = strlen(visible);
      for (int i = 0; i < j; i++) {
        visible[i] = '*';
      };
    };

    gui_draw_text(x, y, visible, NULL, flags);

    glDisable(GL_TEXTURE_2D);

    if (menu_focus_object == menu_object_index) {
      if (fmod(on_frame_time - blinktime, 0.5) < 0.3) {
        int pp = ps + CHARACTER_WIDTH * (cursor - offset) + 1;
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glLineWidth(2.0);
        COLOR_ACTIVE;
        glBegin(GL_LINE_STRIP);
        glVertex2f(pp, pt);
        glVertex2f(pp, pv);
        glEnd();
        glDisable(GL_BLEND);
      };
    };

  };

  #undef cursor
  #undef offset
  #undef hoverglow
  #undef lastfocus
  #undef blinktime
  if (enter) sound_play (SOUND_BUTTON_PRESS, 0.1f, NULL);

  if (enter) return 2;
  if (change) return 1;
  return 0;

};

static int last_hover = 0;

//--page-split-- gui_link

int gui_link(int x, int y, char *string) {
  // returns true when string is clicked.

  int fullpush = 0;

  #define semipush menu_object_data[menu_object_index][0]
  #define drawdown menu_object_data[menu_object_index][1]
  #define hoverglow menu_object_data[menu_object_index][2]

  int pixel_width = CHARACTER_WIDTH * strlen(string);
  int ps = CHARACTER_WIDTH * x;
  int pt = CHARACTER_HEIGHT * y;
  int pu = ps + pixel_width;
  int pv = pt + CHARACTER_HEIGHT;

  if (x < 0) {
    ps = (gui_menu_width - pixel_width) / 2;
    pu = (gui_menu_width + pixel_width) / 2;
  };

  if (y < 0) {
    pt = (gui_menu_height - CHARACTER_HEIGHT) / 2;
    pv = (gui_menu_height + CHARACTER_HEIGHT) / 2;
  };

  ps += gui_menu_x_offset; pt += gui_menu_y_offset;
  pu += gui_menu_x_offset; pv += gui_menu_y_offset;

  #define test 0

  if (++menu_object_index >= MENU_MAX_OBJECTS) easy_fuck("Too many menu objects.");

  if (menu_process_event) {

    if (MOUSE_TEST(ps-test,pt-test,pu+test,pv+test)) {
      if (LEFT_PRESS) {
        semipush = 1;
        drawdown = 1;
      };
      if (LEFT_RELEASE) {
        if (semipush) fullpush = 1;
        drawdown = 0;
        semipush = 0;
      };
      if (HOVER_EVENT && semipush) {
        drawdown = 1;

      }
      hoverglow = 1;
    } else {
      if (LEFT_RELEASE) semipush = 0;
      drawdown = 0;
      hoverglow = 0;
    };

    if (menu_focus_object == menu_object_index) {
      if (KEY_PRESS_EVENT) {
        if (KEY == GLFW_KEY_TAB) {
          if (menu_modifier_shift) {
            menu_next_focus = menu_focus_object - 1;
          } else {
            menu_next_focus = menu_focus_object + 1;
          };
        };
        if (KEY == GLFW_KEY_SPACE || KEY == GLFW_KEY_ENTER || KEY == GLFW_KEY_KP_ENTER) {
          fullpush = 1;
        };
      };
    };

  };

  #undef test

  #define offset -2

  if (menu_draw_widget) {

    if (drawdown) {
      COLOR_ACTIVE;
      gui_draw_text(x, y, string, NULL, 0);
    } else {
      if (hoverglow || menu_focus_object == menu_object_index) {
        COLOR_HOVER;
      } else {
        COLOR_CLICKABLE;
      };
      gui_draw_text(x, y, string, NULL, 0);
    };

  };

  #undef offset
  if (fullpush) sound_play (SOUND_BUTTON_PRESS, 0.1f, NULL);
  return fullpush;

  #undef semipush
  #undef drawdown

};

//--page-split-- gui_texture

void gui_texture(int column, int line, int width, int height, int texture, double ts, double tt, double tu, double tv) {

  if (!menu_draw_widget) return;

  int pixel_width = width * CHARACTER_WIDTH;
  int pixel_height = height * CHARACTER_HEIGHT;

  int ps = gui_menu_x_offset + column * CHARACTER_WIDTH;
  int pt = gui_menu_y_offset + line * CHARACTER_HEIGHT;
  int pu = ps + pixel_width;
  int pv = pt + pixel_height;

  glEnable(GL_TEXTURE_2D);
  glCallList(texture_list_base + texture + RENDER_IN_GRAYSCALE);

  //printf("Rendering menu texture with alpha = %f\n", texture_data[texture].alpha);
  if (texture_data[texture].alpha >= 1) {
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else if (texture_data[texture].alpha > 0) {
    glEnable(GL_ALPHA_TEST); glAlphaFunc(GL_GEQUAL, texture_data[texture].alpha);
  };

  glColor3f(1.0, 1.0, 1.0);
  glBegin(GL_QUADS);
  texture_quad(ts, tt, tu, tv, ps, pt, pu, pv);
  glEnd();

  if (texture_data[texture].alpha >= 1) {
    glDisable(GL_BLEND);
  } else if (texture_data[texture].alpha > 0) {
    glDisable(GL_ALPHA_TEST);
  };
  glDisable(GL_TEXTURE_2D);

};

    #undef X0
    #undef X1
    #undef X2
    #undef X3
    #undef Y0
    #undef Y1
    #undef Y2
    #undef Y3

//--page-split-- gui_draw_block

void gui_draw_block (int block_type, int column, int line, int width, int height) {
  if (!menu_draw_widget) return;

  double pixel_width = width * CHARACTER_WIDTH;
  double pixel_height = height * CHARACTER_HEIGHT;

  double ps = gui_menu_x_offset + column * CHARACTER_WIDTH;
  double pt = gui_menu_y_offset + line * CHARACTER_HEIGHT;
  double pu = ps + pixel_width;
  double pv = pt + pixel_height;

  glEnable(GL_TEXTURE_2D);

  glPushMatrix();
  glTranslatef(ps + (pixel_width / 2.0) + 5, pt + (pixel_height / 2.0), 0);
  glScalef(pixel_width , -pixel_width , 1);

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

  if (block_data[block_type].visible) {
    glColor3f(1.0, 1.0, 1.0);
    if (texture_data[block_data[block_type].index[BLOCK_SIDE_UP]].alpha == 1.0) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else if (texture_data[block_data[block_type].index[BLOCK_SIDE_UP]].alpha > 0.0) {
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GEQUAL, texture_data[block_data[block_type].index[BLOCK_SIDE_UP]].alpha);
    };
    glBindTexture(GL_TEXTURE_2D, texture_data[block_data[block_type].index[BLOCK_SIDE_UP]].name + RENDER_IN_GRAYSCALE);
    glBegin(GL_QUADS);
    glTexCoord2f(TC1, TC1);        glVertex2f(X0, Y0);
    glTexCoord2f(TC1, TC2);        glVertex2f(X1 , Y1);
    glTexCoord2f(TC0, TC2);        glVertex2f(X2, Y2);
    glTexCoord2f(TC0, TC1);        glVertex2f(X3, Y3);
    glEnd();
    if (texture_data[block_data[block_type].index[BLOCK_SIDE_UP]].alpha == 1.0) {
      glDisable(GL_BLEND);
    } else if (texture_data[block_data[block_type].index[BLOCK_SIDE_UP]].alpha > 0.0) {
      glDisable(GL_ALPHA_TEST);
    };
    if (texture_data[block_data[block_type].index[BLOCK_SIDE_FRONT]].alpha == 1.0) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else if (texture_data[block_data[block_type].index[BLOCK_SIDE_FRONT]].alpha > 0.0) {
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GEQUAL, texture_data[block_data[block_type].index[BLOCK_SIDE_UP]].alpha);
    };
    glColor3f(0.8, 0.8, 0.8);
    glBindTexture(GL_TEXTURE_2D, texture_data[block_data[block_type].index[BLOCK_SIDE_FRONT]].name + RENDER_IN_GRAYSCALE);
    glBegin(GL_QUADS);
    glTexCoord2f(TC0, TC0);        glVertex2f(X4, Y4);
    glTexCoord2f(TC1, 0);          glVertex2f(X5, Y5);
    glTexCoord2f(TC1, TC1);        glVertex2f(X0, Y0);
    glTexCoord2f(TC0, TC1);        glVertex2f(X3, Y3);
    glEnd();
    if (texture_data[block_data[block_type].index[BLOCK_SIDE_FRONT]].alpha == 1.0) {
      glDisable(GL_BLEND);
    } else if (texture_data[block_data[block_type].index[BLOCK_SIDE_FRONT]].alpha > 0.0) {
      glDisable(GL_ALPHA_TEST);
    };
    if (texture_data[block_data[block_type].index[BLOCK_SIDE_RIGHT]].alpha == 1.0) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else if (texture_data[block_data[block_type].index[BLOCK_SIDE_RIGHT]].alpha > 0.0) {
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GEQUAL, texture_data[block_data[block_type].index[BLOCK_SIDE_RIGHT]].alpha);
    };
    glColor3f(0.6, 0.6, 0.6);
    glBindTexture(GL_TEXTURE_2D, texture_data[block_data[block_type].index[BLOCK_SIDE_RIGHT]].name + RENDER_IN_GRAYSCALE);
    glBegin(GL_QUADS);
    glTexCoord2f(TC1, TC0);         glVertex2f(X5, Y5);
    glTexCoord2f(TC2, TC0);         glVertex2f(X6, Y6);
    glTexCoord2f(TC2, TC1);         glVertex2f(X1, Y1);
    glTexCoord2f(TC1, TC1);         glVertex2f(X0, Y0);
    glEnd();
    if (texture_data[block_data[block_type].index[BLOCK_SIDE_RIGHT]].alpha == 1.0) {
      glDisable(GL_BLEND);
    } else if (texture_data[block_data[block_type].index[BLOCK_SIDE_RIGHT]].alpha > 0.0) {
      glDisable(GL_ALPHA_TEST);
    };
  };
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);

}

//--page-split-- gui_percent

void gui_percent (double column, double line, double width, double height, double percent, int chat_color_bg, int chat_color_fg) {
  if (!menu_draw_widget) return;

  double pixel_width = width * CHARACTER_WIDTH;
  double pixel_height = height * CHARACTER_HEIGHT;

  double ps = gui_menu_x_offset + column * CHARACTER_WIDTH;
  double pt = gui_menu_y_offset + line * CHARACTER_HEIGHT;
  double pu = ps + pixel_width;
  double pv = pt + pixel_height;

  glColor3f (1.0, 1.0, 1.0);
  glLineWidth(1);
  glBegin(GL_LINES);
  glVertex2d(ps, pt);
  glVertex2d(ps + width * CHARACTER_WIDTH, pt);

  glVertex2d(ps + width * CHARACTER_WIDTH, pt);
  glVertex2d(ps + width * CHARACTER_WIDTH, pt + height * CHARACTER_HEIGHT);

  glVertex2d(ps + width * CHARACTER_WIDTH, pt + height * CHARACTER_HEIGHT);
  glVertex2d(ps, pt + height * CHARACTER_HEIGHT);

  glVertex2d(ps, pt + height * CHARACTER_HEIGHT);
  glVertex2d(ps, pt);

  glEnd();

  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  double length = percent * ((width * CHARACTER_WIDTH) - 3)/ 100.0;

  chat_color(chat_color_bg);
  glBegin(GL_QUADS);
  glVertex2d(ps + 1, pt + 1);
  glVertex2d(ps + (width * CHARACTER_WIDTH) - 2, pt + 1);
  glVertex2d(ps + (width * CHARACTER_WIDTH) - 2, pt + (height * CHARACTER_HEIGHT) - 2);
  glVertex2d(ps + 1, pt + (height * CHARACTER_HEIGHT) - 2);
  glEnd();

  chat_color(chat_color_fg);
  glBegin(GL_QUADS);
  glVertex2d(ps + 1, pt + 1);
  glVertex2d(ps + 1 + length, pt + 1);
  glVertex2d(ps + 1 + length, pt + (height * CHARACTER_HEIGHT) - 2);
  glVertex2d(ps + 1, pt + (height * CHARACTER_HEIGHT) - 2);
  glEnd();
}

//--page-split-- gui_scroll_bar_vertical

int gui_scroll_bar_vertical (double column, double line, double width, double height, int max_lines, int page_lines, int current_line) {
  int return_value = current_line;
  static int y_drag_start = 0;
  static int cv = 0;
  if (height < 2) return (0);
  double pixel_width = width * CHARACTER_WIDTH;
  double pixel_height = height * CHARACTER_HEIGHT;
  static int bpos = 0;
  static double dd = 0;

  double ps = gui_menu_x_offset + column * CHARACTER_WIDTH;
  double pt = gui_menu_y_offset + line * CHARACTER_HEIGHT;
  double pu = ps + pixel_width;
  double pv = pt + pixel_height;

  int fullpush = 0;

  #define semipush menu_object_data[menu_object_index][0]
  #define drawdown menu_object_data[menu_object_index][1]
  #define hoverglow menu_object_data[menu_object_index][2]

  if (++menu_object_index >= MENU_MAX_OBJECTS) easy_fuck("Too many menu objects.");

  //Convert the parameter values from document scale to screen scale
  int total_screen_range = (height * CHARACTER_HEIGHT) - 2;
  int pager_size = total_screen_range;
  int y_offset = 0;
  int the_line = 0;
  if (max_lines > page_lines) {
    pager_size = total_screen_range - ((max_lines / page_lines) * CHARACTER_HEIGHT);
    if (pager_size < 32) pager_size = 32;
    if (pager_size >= total_screen_range) pager_size = total_screen_range;
    the_line = max_lines - current_line - page_lines;
    y_offset = nearbyint ((double)the_line / (double)(max_lines - page_lines) * (double)(total_screen_range - pager_size));
  }
  //printf ("total_screen_range %i, pager_size %i, max_lines %i, page_lines %i, the_line %i, y_offset %i\n",
  //    total_screen_range, pager_size, max_lines, page_lines, the_line, y_offset);

  if (menu_process_event) {
    if (MOUSE_X >= ps + 2 && MOUSE_X <= ps + width * CHARACTER_WIDTH - 2) {

      if (MOUSE_Y >= pt + 2 && MOUSE_Y < pt + 2 + y_offset) {
          if (LEFT_PRESS) {
           semipush = 1;
            drawdown = 1;
          };
          if (LEFT_RELEASE) {
            if (semipush == 1) fullpush = 1;
            drawdown = 0;
            semipush = 0;
          };
          if (HOVER_EVENT && semipush) drawdown = 1;
          hoverglow = 1;
      } else if (MOUSE_Y > pt + y_offset + pager_size && MOUSE_Y <= pt + total_screen_range + 2) {
          if (LEFT_PRESS) {
            semipush = 2;
            drawdown = 2;
          };
          if (LEFT_RELEASE) {
            if (semipush == 2) fullpush = 2;
            drawdown = 0;
            semipush = 0;
          };
          if (HOVER_EVENT && semipush) drawdown = 2;
          hoverglow = 2;
      } else if (MOUSE_Y > pt + 2 + y_offset && MOUSE_Y < pt + 2 + y_offset + pager_size) {
          if (LEFT_PRESS) {
            if (semipush != 3) {
                semipush = 3;
                drawdown = 3;

                //int pos = event_list[menu_current_event][4] - pt;
                //if (pos < 0) pos = 0;
                //if (pos > total_screen_range) pos = total_screen_range;
                //y_drag_start = pos * max_lines / total_screen_range;

                y_drag_start = MOUSE_Y;
                bpos = current_line;
                cv = 0;
                dd = 0;
            }
          };
          if (LEFT_RELEASE) {
            drawdown = 0;
            semipush = 0;
          };
          if (HOVER_EVENT && semipush) drawdown = 3;
          hoverglow = 3;
      } else {
        if (LEFT_RELEASE) semipush = 0;
        drawdown = 0;
        hoverglow = 0;
      }
    } else {
      if (LEFT_RELEASE) semipush = 0;
      drawdown = 0;
      hoverglow = 0;
    }

    if (menu_focus_object == menu_object_index) {
      if (KEY_PRESS_EVENT) {
        if (KEY == GLFW_KEY_TAB) {
          if (menu_modifier_shift) {
            menu_next_focus = menu_focus_object - 1;
          } else {
            menu_next_focus = menu_focus_object + 1;
          };
        };
      };
    };
  };

  if (fullpush == 1) {
    return_value = current_line + page_lines;
  } else if (fullpush == 2) {
    return_value = current_line - page_lines;
  } else if (menu_process_event && semipush == 3) {
    if (event_list[menu_current_event][0] == 4 && max_lines > 0 ) {
      //  int diff = y_drag_start - event_list[menu_current_event][4];
      //  y_drag_start = event_list[menu_current_event][4];
      //  double scale = 0;
      //  if (max_lines > 0) scale = floor ((double)(total_screen_range) / (double)(max_lines));
      //  printf ("Scale %g %i %i %i\n", scale, (int)(floor(diff * scale)), max_lines, total_screen_range - 2);
      //  if (scale > 0) return_value = current_line + (int)floor(diff * scale);

      //  int diff = y_drag_start - event_list[menu_current_event][4];
      //  y_drag_start = event_list[menu_current_event][4];
      //  if (diff != 0) {
      //      double d = max_lines;
      //      d /= (double) total_screen_range;

      //      cv += diff;
      //      dd += ((double)diff * d);
      //      printf ("%i diff %i %g %g ts=%i ml=%i\n", diff, cv, d, dd, total_screen_range, max_lines);
      //  }

      //  if (total_screen_range > 0 && max_lines > 0) {
      //      int pos = event_list[menu_current_event][4] - pt;
      //      if (pos < 0) pos = 0;
      //      if (pos > total_screen_range) pos = total_screen_range;
      //
      //      int p = pos * max_lines / total_screen_range;
      //      if (p - y_drag_start != 0) {
      //          return_value = current_line -= p - y_drag_start;
      //          //printf ("P %i\n",p - y_drag_start);
      //          y_drag_start = p;
      //      }
      //  }

        int diff = y_drag_start - event_list[menu_current_event][4];

        if (diff != 0) {
            double d = diff;
            d = (double)total_screen_range / (double)max_lines;
            printf ("%i %g\n", diff, d);
            if (d >= 1 || d <= -1) {

            }
        }
    }
  }

  if (menu_draw_widget) {
    // Draw the outline
    glColor3f (1.0, 1.0, 1.0);
    glLineWidth(1);
    glBegin(GL_LINES);
    glVertex2d(ps, pt);
    glVertex2d(ps + width * CHARACTER_WIDTH, pt);

    glVertex2d(ps + width * CHARACTER_WIDTH, pt);
    glVertex2d(ps + width * CHARACTER_WIDTH, pt + height * CHARACTER_HEIGHT);

    glVertex2d(ps + width * CHARACTER_WIDTH, pt + height * CHARACTER_HEIGHT);
    glVertex2d(ps, pt + height * CHARACTER_HEIGHT);

    glVertex2d(ps, pt + height * CHARACTER_HEIGHT);
    glVertex2d(ps, pt);

    glEnd();

    // draw the bar
    if (drawdown == 3) COLOR_TOUCH;
    else if (hoverglow == 3 || menu_focus_object == menu_object_index) COLOR_HOVER;
    else COLOR_CLICKABLE;

    glBegin(GL_QUADS);

    glVertex2d(ps + 2, pt + 2 + y_offset); //top left
    glVertex2d((ps + width * CHARACTER_WIDTH) - 2 , pt + 2 + y_offset); //top right

    glVertex2d((ps + width * CHARACTER_WIDTH) - 2, (pt) + y_offset + pager_size); // bottom right
    glVertex2d(ps + 2, pt +  y_offset + pager_size); // bottom left

    glEnd();
  }

  #undef semipush
  #undef drawdown
  return (return_value);
}
