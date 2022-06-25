#include "everything.h"

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

  const char *idk = glGetString(GL_RENDERER);
  printf("glGetString(GL_RENDERER) = \"%s\"\n", idk);
};

//--page-split-- glfw_terminate

void glfw_terminate() {
  glfwTerminate();
};

//--page-split-- get_monitor_of_game_window

// To get dual use out of this function, if all of the parameters are positive,
// pretend that's the window position and instead of returning a monitor,
// just return 1 if it looks like an OK place to position the window.
// We'll subtract lucky 7 pixels from the Y position and cut the width and
// height in half since we're we're mostly just worried about whether the
// title bar is accessible and the window is at least partially visible.

static GLFWmonitor *get_monitor_of_game_window(int x, int y, int width, int height) {
  // s = rightmost pixel, t = bottommost pixel
  int wx, wy, ws, wt;
  if (x >= 0 && y >= 0) {
    width /= 2; height /= 2;
    x += width / 2; y -= 7;
    wx = x; wy = y; ws = x + width - 1; wt = y + height - 1;
  } else {
    // get the location and size of the game window
    glfwGetWindowPos(glfw_window, &wx, &wy);
    glfwGetWindowSize(glfw_window, &ws, &wt);
    ws += wx - 1; wt += wy - 1;
  };
  int count;
  GLFWmonitor **monitor = glfwGetMonitors(&count);
  GLFWmonitor *best_monitor = glfwGetPrimaryMonitor();
  //printf("Game window covers x %d to %d and y %d to %d\n", wx, ws, wy, wt);
  int best_coverage = 0;
  for (int i = 0; i < count; i++) {
    // get the location and size of the monitor
    int mx, my;
    glfwGetMonitorPos(monitor[i], &mx, &my);
    const GLFWvidmode *mode = glfwGetVideoMode(monitor[i]);
    int ms = mx + mode->width - 1;
    int mt = my + mode->height - 1;
    //printf("Monitor %d covers x %d to %d and y %d to %d\n", i, mx, ms, my, mt);
    // determine how much of the game window overlaps it
    if (mx < wx) mx = wx;
    if (my < wy) my = wy;
    if (ms > ws) ms = ws;
    if (mt > wt) mt = wt;
    int coverage = (ms - mx + 1) * (mt - my + 1);
    //printf("Game window occupies %d pixels (%d, %d) of monitor %d.\n", coverage, (ms - mx + 1), (mt - my + 1), i);
    if (best_coverage < coverage) {
      best_coverage = coverage;
      best_monitor = monitor[i];
    };
  };
  if (x >= 0 && y >= 0) {
    //printf("%d vs. %d (%d, %d)\n", best_coverage, (ws - wx + 1) * (wt - wy + 1), (ws - wx + 1), (wt - wy + 1));
    if (best_coverage == (ws - wx + 1) * (wt - wy + 1)) {
      return (void *) 1;
    } else {
      return NULL;
    };
  } else {
    return best_monitor;
  };
};

//--page-split-- glfw_open_window

void glfw_open_window() {
  glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
  glfwWindowHint(GLFW_MAXIMIZED, GL_FALSE);
  glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
  glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

  if (option_fsaa_samples) {
    glfwWindowHint(GLFW_SAMPLES, 4);
  } else {
    glfwWindowHint(GLFW_SAMPLES, 0);
  };

  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode = glfwGetVideoMode(monitor);

  if (option_window_width > mode->width) option_window_width = mode->width;
  if (option_window_height > mode->height) option_window_height = mode->height;
  if (option_window_width < 992) option_window_width = 992;
  if (option_window_height < 608) option_window_height = 608;
  // Maybe we should allow at least this small?...
  //if (option_window_width < 928) option_window_width = 928;
  //if (option_window_height < 448) option_window_height = 448;
  //if (option_window_width < 640) option_window_width = 640;
  //if (option_window_height < 360) option_window_height = 360;
  if (!option_remember_size) {
    option_window_width = 992;
    option_window_height = 608;
  };
  glfw_window = glfwCreateWindow(option_window_width, option_window_height, "Multiplayer Map Editor", NULL, context_window);
  if (!glfw_window) easy_fuck("Failed to create a window.\n");
  glfwMakeContextCurrent(glfw_window);

  int center_instead = 0;
  if (option_window_location == 2) {
    if (get_monitor_of_game_window(option_window_location_x, option_window_location_y, option_window_width, option_window_height)) {
      printf("Attempting to restore window position (%d, %d)\n", option_window_location_x, option_window_location_y);
      glfwSetWindowPos(glfw_window, option_window_location_x, option_window_location_y);
      glfwSetWindowSize(glfw_window, option_window_width, option_window_height);
    } else {
      printf("Saved window position is not entirely inside any monitor, so centering window instead.\n");
      center_instead = 1;
    };
  };
  if (option_window_location == 1 || center_instead) {
    GLFWmonitor *monitor = get_monitor_of_game_window(-1, -1, -1, -1);
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

  if (option_fsaa_samples) {
    //glEnable(GL_MULTISAMPLE);
    // this shit seems to be broken
    //void glMinSampleShading(GLfloat value);
    //glMinSampleShading(1.0);
    //glEnable(GL_SAMPLE_SHADING);
  } else {
    //glDisable(GL_MULTISAMPLE);
  };

  int samples;
  glGetIntegerv(GL_SAMPLES, &samples);
  printf("Opened window with GL_SAMPLES == %d.\n", samples);

  // I should try this SRGB thing sometime.
  //glEnable(GL_FRAMEBUFFER_SRGB);

  glfwSwapInterval(!!option_request_vertical_sync);

  glfw_fullscreen(option_fullscreen);
};

//--page-split-- glfw_close_window

void glfw_close_window() {
  glfw_fullscreen(0);
  glfwGetWindowPos(glfw_window, &option_window_location_x, &option_window_location_y);
  glfwGetWindowSize(glfw_window, &option_window_width, &option_window_height);
  printf("Window size at close: %d x %d\n", option_window_width, option_window_height);
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

//--page-split-- glfw_fullscreen

void glfw_fullscreen (int enable) {
  static int saved_x = 0, saved_y = 0, saved_width = 0, saved_height = 0;
  static int is_fullscreen_now = 0;
  if (enable && !is_fullscreen_now) {
    glfwGetWindowPos(glfw_window, &saved_x, &saved_y);
    glfwGetWindowSize(glfw_window, &saved_width, &saved_height);
    GLFWmonitor *monitor = get_monitor_of_game_window(-1, -1, -1, -1);
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowMonitor(glfw_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    is_fullscreen_now = 1;
  };
  if (!enable && is_fullscreen_now) {
    glfwSetWindowMonitor(glfw_window, NULL, saved_x, saved_y, saved_width, saved_height, GLFW_DONT_CARE);
    // In Wine, restoring the window position and size doesn't work,
    // but doing an extra window size call seems to fix the position too.
    glfwSetWindowSize(glfw_window, saved_width, saved_height);
    is_fullscreen_now = 0;
  };
};
