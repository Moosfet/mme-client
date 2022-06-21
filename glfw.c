#include "everything.h"

int glfw_fullscreen_flag = 0;
int glfw_mouse_capture_flag = 0;
int glfw_mouse_x_center;
int glfw_mouse_y_center;
int glfw_cpu_cores = 0;

static GLFWwindow *context_window = NULL;
GLFWwindow *glfw_window = NULL;

static double mouse_save_x = 0;
static double mouse_save_y = 0;

//--page-split-- error_callback

static void error_callback (int code, char *text) {
  fprintf(stderr, "GLFW Error %d: %s\n", code, text);
};

//--page-split-- window_close_callback

static void window_close_callback(GLFWwindow *window) {
  glfwSetWindowShouldClose(window, GLFW_FALSE);
  main_shutdown();
};

//--page-split-- glfw_capture_mouse

void glfw_capture_mouse() {
  if (!glfwGetWindowAttrib(glfw_window, GLFW_FOCUSED)) return;
  if (!glfw_mouse_capture_flag) {
    glfw_mouse_capture_flag = 1;
    glfwGetCursorPos(glfw_window, &mouse_save_x, &mouse_save_y);
    if (glfwRawMouseMotionSupported()) glfwSetInputMode(glfw_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  };
};

//--page-split-- glfw_release_mouse

void glfw_release_mouse() {
  if (glfw_mouse_capture_flag) {
    glfw_mouse_capture_flag = 0;
    glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    event_move_mouse(mouse_save_x, mouse_save_y);
    event_mouse_position_x = mouse_save_x;
    event_mouse_position_y = mouse_save_y;
  };
};

//--page-split-- glfw_initialize

void glfw_initialize() {
 glfwSetErrorCallback((GLFWerrorfun) error_callback);
  int result = glfwInit();
  if (!result) easy_fuck("glfwInit() returned a non-zero value!");

  glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
  context_window = glfwCreateWindow(64, 64, "", NULL, NULL);
  glfwMakeContextCurrent(context_window);

  // The first buffer swap seems to set some stuff detected by thread sanatizer,
  //glfwSwapBuffers(share); // so we'll pointlessly swap the buffers here.
  // ...except I think that was me using GLFW wrong so maybe we don't need it.

  glfw_fullscreen_flag = 0; // was argument_fullscreen;
};

//--page-split-- glfw_terminate

void glfw_terminate() {
  glfwTerminate();
};

static GLFWmonitor *get_monitor_of_game_window() {
  // get the location and size of the game window
  int wx, wy, ws, wt;
  glfwGetWindowPos(glfw_window, &wx, &wy);
  glfwGetWindowSize(glfw_window, &ws, &wt);
  int count;
  GLFWmonitor **monitor = glfwGetMonitors(&count);
  GLFWmonitor *best_monitor = glfwGetPrimaryMonitor();
  printf("Game window covers x %d to %d and y %d to %d\n", wx, wx + ws, wy, wy + wt);
  int best_coverage = 0;
  for (int i = 0; i < count; i++) {
    // get the location and size of the monitor
    int mx, my;
    glfwGetMonitorPos(monitor[i], &mx, &my);
    const GLFWvidmode *mode = glfwGetVideoMode(monitor[i]);
    int ms = mode->width;
    int mt = mode->height;
    printf("Monitor %d covers x %d to %d and y %d to %d\n", i, mx, mx + ms, my, my + mt);
    // determine how much of the game window overlaps it
    if (mx < wx) mx = wx;
    if (my < wy) my = wy;
    if (mx + ms > wx + ws) ms = mx + (ws - wx);
    if (my + mt > wy + wt) mt = my + (wt - wy);
    int coverage = (ms - mx) * (mt - my);
    printf("Game window occupies %d pixels of monitor %d.\n", coverage, i);
    if (coverage > best_coverage) best_monitor = monitor[i];
  };
  return best_monitor;
};

//--page-split-- glfw_open_window

void glfw_open_window() {
  glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
  glfwWindowHint(GLFW_MAXIMIZED, GL_FALSE);
  glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
  glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
  glfwWindowHint(GLFW_SAMPLES, 4 * option_fsaa_samples);

  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode = glfwGetVideoMode(monitor);

  if (option_window_width > mode->width) option_window_width = mode->width;
  if (option_window_height > mode->height) option_window_height = mode->height;
  if (option_window_width < 992) option_window_width = 992;
  if (option_window_height < 608) option_window_height = 608;
  if (!option_remember_size) {
    option_window_width = 992;
    option_window_height = 608;
  };
  glfw_window = glfwCreateWindow(option_window_width, option_window_height, "Multiplayer Map Editor", NULL, context_window);
  if (!glfw_window) easy_fuck("Failed to create a window.\n");
  glfwMakeContextCurrent(glfw_window);

  if (option_center_window) {
    GLFWmonitor *monitor = get_monitor_of_game_window();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos(glfw_window, (mode->width - option_window_width) / 2, (mode->height - option_window_height) / 2);
  };

  glfwSetKeyCallback(glfw_window, event_keyboard_key_callback);
  glfwSetCharCallback(glfw_window, event_keyboard_char_callback);
  glfwSetMouseButtonCallback(glfw_window, event_mouse_button_callback);
  glfwSetScrollCallback(glfw_window, event_mouse_scroll_callback);
  glfwSetCursorPosCallback(glfw_window, event_mouse_position_callback);
  glfwSetWindowCloseCallback(glfw_window, window_close_callback);

  glfw_mouse_capture_flag = 0;
  if (glfw_fullscreen_flag && menu_function_pointer == menus_play) glfw_capture_mouse();
  int samples = glfwGetWindowAttrib(glfw_window, GLFW_SAMPLES);
  printf("Opened window with GLFW_SAMPLES == %d.\n", samples);

  // I should try this SRGB thing sometime.
  //glEnable(GL_FRAMEBUFFER_SRGB);

  if (option_request_vertical_sync) {
    glfwSwapInterval(1);  // Enables vertical sync, sometimes.
  } else {
    glfwSwapInterval(0);  // Disables vertical sync, sometimes.
  };
};

//--page-split-- glfw_close_window

void glfw_close_window() {
  glfwGetWindowSize(glfw_window, &option_window_width, &option_window_height);
  glfwDestroyWindow(glfw_window);
  glfw_window = NULL;
};

//--page-split-- glfw_set_window_title

void glfw_set_window_title(char *title) {
  if (title != NULL && strlen(title)) {
    char *suffix = " -- Multiplayer Map Editor";
    char *full = NULL;
    memory_allocate(&full, strlen(title) + strlen(suffix) + 1);
    char *o = full;
    for (char *i = title; *i != 0; *i++) {
      if (*i >= 32 && *i < 127) *o++ = *i;
    };
    for (char *i = suffix; *i != 0; *i++) {
      if (*i >= 32 && *i < 127) *o++ = *i;
    };
    *o = 0;
    glfwSetWindowTitle(glfw_window, full);
    memory_allocate(&full, 0);
  } else {
    glfwSetWindowTitle(glfw_window, "Multiplayer Map Editor");
  };
};
