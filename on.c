#include "everything.h"

#ifdef TEST
int on_activate_lag = 0;
#endif

// These are functions to be called when certain things happen, in order that
// everything can allocate or free resources and reset itself to default state.

//--page-split-- on_program_start

void on_program_start(int argc, char **argv) {
  DEBUG("Beginning initialization procedures...");

  printf("Multiplayer Map Editor\n");
  printf("SVN revision %s\n", build_svn_string);
  printf("Compiled %s for %s\n", build_compile_date, build_target);

  #ifdef LINUX
  backtrace_initialize();
  #endif

  easy_initialize();
  cpu_initialize();

  easy_strcpy(&server_portal_address, PORTAL_ADDRESS);

  argument_check(argc, argv);
  printf("server: '%s'\n", server_address);
  printf("portal: '%s'\n", server_portal_address);

  #ifdef UNIX
    chdir(getenv("HOME"));
    mkdir(".multiplayermapeditor", 0777);
    chdir(".multiplayermapeditor");
    mkdir("cache", 0777);
    signal(SIGPIPE, SIG_IGN);
  #endif

  #ifdef WINDOWS
    char *appdata = getenv("APPDATA");
    printf("APPDATA: %s\n", appdata);
    if (appdata == NULL || strlen(appdata) == 0) {
      char directory[1024] = {};
      GetModuleFileName(NULL, directory, 1023);
      chdir(directory);
      mkdir("Multiplayer Map Editor (settings & cache)");
      chdir("Multiplayer Map Editor (settings & cache)");
      mkdir("cache");
    } else {
      chdir(appdata);
      mkdir("Multiplayer Map Editor");
      chdir("Multiplayer Map Editor");
      mkdir("cache");
    };
    WSADATA bullshit;
    WSAStartup(0x0202, &bullshit);
  #endif

  easy_seed_random_number_generator();

  #ifdef TEST
  lag_initialize();
  #endif

  network_initialize();
  glfw_initialize();
  controls_initialize();
  option_load();
  opengl_extentions();
  texture_initialize();
  sound_initialize();
  map_initialize();

  DEBUG("Finished initialization procedures...");
};

//--page-split-- on_program_exit

void on_program_exit(void) {
  DEBUG("Beginning termination procedures...");

  server_terminate();
  sound_terminate();
  glfw_terminate();
  option_save();

  #ifdef TEST
  lag_terminate();
  #endif
  easy_terminate();

  DEBUG("Finished termination procedures...");
};

double on_frame_time = 0;

//--page-split-- on_frame

void on_frame(void) {

  #ifdef GRIEF
    if (map_initialization_flag) {
      for (int i = 0; i < 100; i++) {
        struct int_xyz b;
        b.x = easy_random(map_data.dimension.x);
        b.y = easy_random(map_data.dimension.y);
        b.z = easy_random(map_data.dimension.z);
        int t = easy_random(255) + 1;
        if (block_data[t].visible && block_data[t].index[SIDE_UP] != TEXTURE_SWEET_U) {
          map_modify(b, t, 0);
          server_modify(b, t);
        };
      };
    };
  #endif

  #ifdef TEST
  if (!argument_lag && on_activate_lag) {
    argument_lag = 1;
    lag_initialize();
    on_activate_lag = 0;
  };
  #endif

  if (on_frame_time == 0) {
    on_frame_time = easy_time();
    server_connect_time = on_frame_time;
  };

  sanity_check();

  lag_push(1, "projectile_movement()");
  projectile_movement();
  lag_pop();

  lag_push(1, "player_movement()");
  player_movement();
  lag_pop();

  lag_push(1, "map_mouse_test()");
  map_mouse_test();
  lag_pop();

  lag_push(100, "display_render()");
  // Render a frame as well as the menu on top of it.
  CHECK_GL_ERROR;
  display_render();
  CHECK_GL_ERROR;
  lag_pop();

  glFlush();

  lag_push(100, "cpu_analyze()");
  cpu_analyze(); // Recalculate CPU shares...
  lag_pop();

  lag_push(100, "glfwSwapBuffers()");
  //easy_sleep(1.0);
  glfwSwapBuffers(glfw_window);
  lag_pop();

  on_frame_time = easy_time();

  statistics_count_frame();

  #ifdef TEST
    if (argument_record_all_frames) {

      if (display_window_width != 1280 || display_window_height != 960) {
        fprintf(stderr, "Incorrect window size for frame capture.\n");
        exit(1);
      };

      static char *frame_data = NULL;
      static int frame_size = 0;
      static char *bmp_data = NULL;
      static int bmp_size = 0;

      int size = 3 * display_window_width * display_window_height;
      if (size != frame_size) {
        frame_size = size;
        memory_allocate(&frame_data, frame_size);
      };

      #define COLOR_WIDTH (display_window_width >> 1)
      #define COLOR_HEIGHT (display_window_height >> 1)

      #define LINE_WIDTH (-((-COLOR_WIDTH) & ~3))
      #define HEADER_SIZE 54
      #define DATA_SIZE (3 * LINE_WIDTH * COLOR_HEIGHT)
      #define FILE_SIZE (HEADER_SIZE + DATA_SIZE)
      #define DATA(data, offset, type) *((type*) (((char*) data) + offset))

      if (bmp_size != DATA_SIZE) {
        bmp_size = DATA_SIZE;
        memory_allocate(&bmp_data, bmp_size);
      };

      glReadPixels(0, 0, display_window_width, display_window_height, GL_RGB, GL_UNSIGNED_BYTE, frame_data);

      for (int y = 0; y < COLOR_HEIGHT; y++) {
        for (int x = 0; x < COLOR_WIDTH; x++) {
          int r, g, b;
          r = (
            *(frame_data + 3 * ((2 * y + 0) * display_window_width + (2 * x + 0)) + 0)
          + *(frame_data + 3 * ((2 * y + 0) * display_window_width + (2 * x + 1)) + 0)
          + *(frame_data + 3 * ((2 * y + 1) * display_window_width + (2 * x + 0)) + 0)
          + *(frame_data + 3 * ((2 * y + 1) * display_window_width + (2 * x + 1)) + 0)
          ) >> 2;
          g = (
            *(frame_data + 3 * ((2 * y + 0) * display_window_width + (2 * x + 0)) + 1)
          + *(frame_data + 3 * ((2 * y + 0) * display_window_width + (2 * x + 1)) + 1)
          + *(frame_data + 3 * ((2 * y + 1) * display_window_width + (2 * x + 0)) + 1)
          + *(frame_data + 3 * ((2 * y + 1) * display_window_width + (2 * x + 1)) + 1)
          ) >> 2;
          b = (
            *(frame_data + 3 * ((2 * y + 0) * display_window_width + (2 * x + 0)) + 2)
          + *(frame_data + 3 * ((2 * y + 0) * display_window_width + (2 * x + 1)) + 2)
          + *(frame_data + 3 * ((2 * y + 1) * display_window_width + (2 * x + 0)) + 2)
          + *(frame_data + 3 * ((2 * y + 1) * display_window_width + (2 * x + 1)) + 2)
          ) >> 2;
          *(bmp_data + 3 * (y * LINE_WIDTH + x) + 2) = r;
          *(bmp_data + 3 * (y * LINE_WIDTH + x) + 1) = g;
          *(bmp_data + 3 * (y * LINE_WIDTH + x) + 0) = b;
        };
      };

      char header[HEADER_SIZE];
      memset(header, 0, HEADER_SIZE);
      DATA(header, 0, char) = 'B';
      DATA(header, 1, char) = 'M';
      DATA(header, 2, int) = FILE_SIZE;
      DATA(header, 10, int) = HEADER_SIZE;
      DATA(header, 14, int) = 40;
      DATA(header, 18, int) = COLOR_WIDTH;
      DATA(header, 22, int) = COLOR_HEIGHT;
      DATA(header, 26, short) = 1;
      DATA(header, 28, short) = 24;

      char filename[64];
      snprintf(filename, 63, "/home/pj/trash/frame_%0.3f.bmp", on_frame_time);

      FILE *file;
      file = fopen(filename, "wb");
      if (file == NULL) {
        printf("Error creating file: %s\n", filename);
        exit(1);
      };
      fwrite(header, HEADER_SIZE, 1, file);
      fwrite(bmp_data, bmp_size, 1, file);
      fclose(file);

      printf("Saved %s\n", filename);

    };
  #endif

  fflush(stdout);

//  lag_push(1, "the on_frame() functions");
//  lag_pop();

  lag_push(1, "paint_something()");
  paint_something();
  lag_pop();

  lag_push(1, "server_stuff()");
  server_stuff(); // the code that connects to normal servers
  lag_pop();

  lag_push(1, "event_receive()");
  event_receive(); // must be last so that glfwGetWindowParam() is next.
  lag_pop();

};

//--page-split-- on_open_window

void on_open_window(void) {
  CHECK_GL_ERROR;

  glfw_open_window();       CHECK_GL_ERROR;
  statistics_open_window(); CHECK_GL_ERROR;
  display_open_window();    CHECK_GL_ERROR;
  gui_open_window();        CHECK_GL_ERROR;
  event_open_window();      CHECK_GL_ERROR;
  model_open_window();      CHECK_GL_ERROR;
  map_open_window();        CHECK_GL_ERROR;
  stars_open_window();      CHECK_GL_ERROR;
  texture_open_window();    CHECK_GL_ERROR;

  glfw_set_window_title(NULL);

  CHECK_GL_ERROR;
};

//--page-split-- on_close_window

void on_close_window(void) {
  CHECK_GL_ERROR;

  texture_close_window();   CHECK_GL_ERROR;
  stars_close_window();     CHECK_GL_ERROR;
  map_close_window();       CHECK_GL_ERROR;
  model_close_window();     CHECK_GL_ERROR;
  gui_close_window();       CHECK_GL_ERROR;
  display_close_window();   CHECK_GL_ERROR;
  glfw_close_window();      CHECK_GL_ERROR;

};

//--page-split-- on_map_load

void on_map_load(void) {
  DEBUG("enter on_map_load()");

  model_reset();
  player_reset();

  group_reset();
  block_reset();
  sky_reset();
  texture_reset();

  menus_play_reset();

  map_begin_render();

  DEBUG("leave on_map_load()");
};

//--page-split-- on_map_unload

void on_map_unload(void) {
  DEBUG("enter on_map_unload()");

  map_cease_render();

  projectile_reset();

  group_reset();
  block_reset();
  texture_reset();
  paint_reset();

  DEBUG("leave on_map_unload()");
};

//--page-split-- on_server_connect

void on_server_connect(void) {
  DEBUG("enter on_server_connect()");

  server_connect_time = on_frame_time;

  int length;
  length = strlen(server_address);
  char *name = NULL;
  memory_allocate(&name, length + 1);
  strcpy(name, server_address);
  if (!strcmp(name + length - 6, ":44434")) {
    name[length - 6] = 0;
  };
  glfw_set_window_title(name);
  memory_allocate(&name, 0);

  menu_switch(menus_play);

  DEBUG("leave on_server_connect()");
};

//--page-split-- on_server_disconnect

void on_server_disconnect(void) {
  DEBUG("enter on_server_disconnect()");

  server_connect_time = -1.0;

  chat_reset();
  menus_play_reset();
  glfw_set_window_title(NULL);

  DEBUG("leave on_server_disconnect()");
};
