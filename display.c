#include "everything.h"

static GLuint background;
static int background_width;
static int background_height;
static GLuint portal;
static int portal_width;
static int portal_height;

int display_window_width, display_window_height;

#define texture_sprite(A, B, C, D, E, F, G, H) glTexCoord2f(A, B); glVertex2f(E, F); glTexCoord2f(C, B); glVertex2f(G, F); glTexCoord2f(C, D); glVertex2f(G, H); glTexCoord2f(A, D); glVertex2f(E, H);

//--page-split-- display_open_window

void display_open_window() {
  extern char data_wallpaper_png; extern int size_wallpaper_png;
  background = texture_load(&data_wallpaper_png, size_wallpaper_png, 0);
  background_width = texture_width;
  background_height = texture_height;
  extern char data_portal_png; extern int size_portal_png;
  portal = texture_load(&data_portal_png, size_portal_png, 0);
  portal_width = texture_width;
  portal_height = texture_height;
};

//--page-split-- display_close_window

void display_close_window() {
  glDeleteTextures(1, &background);
};

//--page-split-- display_background_image

void display_background_image() {

  glMatrixMode(GL_PROJECTION); glLoadIdentity();
  glMatrixMode(GL_MODELVIEW); glLoadIdentity();

  glClearColor(0.05, 0.05, 0.05, 1.0);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glClear(GL_COLOR_BUFFER_BIT);

  glEnable(GL_TEXTURE_2D);

  double tx, ty, offset;
  if (server_address != NULL && !strcmp(server_address, PORTAL_ADDRESS)) {
    glBindTexture(GL_TEXTURE_2D, portal + option_anaglyph_enable);
    tx = 0.5 * display_window_width / portal_width;
    ty = 0.5 * display_window_height / portal_height;
    offset = fmod(0.0 * on_frame_time, portal_height) / portal_height;
  } else {
    glBindTexture(GL_TEXTURE_2D, background + option_anaglyph_enable);
    tx = 0.5 * display_window_width / background_width;
    ty = 0.5 * display_window_height / background_height;
    offset = fmod(60.0 * on_frame_time, background_height) / background_height;
  };

  glColor3f(1.0, 1.0, 1.0);
  glBegin(GL_QUADS);
  texture_sprite(-tx, -ty + offset, +tx, +ty + offset, -1, -1, +1, +1);
  glEnd();

  glDisable(GL_TEXTURE_2D);

};

//--page-split-- display_check_opengl_error

void display_check_opengl_error(char * message) {
  int error = glGetError();
  if (error && message) {
    printf("At checkpoint '%s':\n", message);
    printf("OpenGL error %d: %s\n", error, gluErrorString(error));
  };
};

//--page-split-- display_render

void display_render() {
  DEBUG("enter display_render()");

  chat_color_cycle_index = 0;

  glfwGetWindowSize(glfw_window, &display_window_width, &display_window_height);
  if (display_window_width <= 0 || display_window_height <= 0) return;
  glViewport(0, 0, display_window_width, display_window_height);

  if (map_is_active() && menu_function_pointer != menus_server_loading) {

    lag_push(100, "map_render()");
    map_render();
    lag_pop();

  } else {

    lag_push(1, "display_background_image()");
    display_background_image();
    lag_pop();

  };

  lag_push(5, "menu_render()");
  menu_render();
  lag_pop();

  WHAT("while rendering stuff"); // Displays OpenGL errors...

  DEBUG("leave display_render()");
};
