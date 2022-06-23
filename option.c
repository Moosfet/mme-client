#include "everything.h"

#define DATA(OFFSET, TYPE) (*((TYPE*) (buffer + OFFSET)))
#define BYTE(OFFSET) (*((unsigned char*) (buffer + OFFSET)))
#define WORD(OFFSET) (*((unsigned short*) (buffer + OFFSET)))
#define UINT(OFFSET) (*((unsigned int*) (buffer + OFFSET)))
#define SINT(OFFSET) (*((signed int*) (buffer + OFFSET)))
#define DOUBLE(OFFSET) (*((double *) (buffer + OFFSET)))
#define FLOAT(OFFSET) (*((float *) (buffer + OFFSET)))

int option_fps_limit;
int option_fps_goal;
int option_fog_type;
int option_fog_distance;
int option_mouse_invert;
int option_mouse_reverse_unused;
int option_perspective_angle;
int option_anaglyph_enable;
int option_show_f1_help;
double option_anaglyph_pixel_size;
double option_anaglyph_distance;
int option_fps_display;
int option_request_vertical_sync;
int option_noclip;
int option_chat_return;
int option_chat_clear;
int option_optimize_chunks;
int option_window_width;
int option_window_height;
int option_window_location;
int option_remember_size;
int option_fsaa_samples;
int option_multiple_paste;
int option_custom_fps;
int option_hyper_help;
char option_cookie_data[20];
int option_have_cookie;
int option_blender_font;
int option_lighting;
int option_always_run;
int option_fly_fast;
int option_textures;
int option_advertise;
int option_sound;
int option_hud_display;
int option_superhuman;
float option_volume;
int option_key_input[CONTROLS_KEY_LIMIT][2];
int option_anisotropic_filtering;
int option_window_location_x;
int option_window_location_y;
int option_anaglyph_units;
double option_pupil_distance;
int option_anaglyph_filter[2];

// unsaved options:
int option_f3_display = 0;
int option_anaglyph = 0;

#define OPTION_SIZE 4096

//--page-split-- option_key_reset

void option_key_reset() {
  option_key_input[CONTROLS_KEY_FORWARD][0] = 'W';
  option_key_input[CONTROLS_KEY_FORWARD][1] = GLFW_KEY_UP;
  option_key_input[CONTROLS_KEY_BACK][0] = 'S';
  option_key_input[CONTROLS_KEY_BACK][1] = GLFW_KEY_DOWN;
  option_key_input[CONTROLS_KEY_LEFT][0] = 'A';
  option_key_input[CONTROLS_KEY_LEFT][1] = GLFW_KEY_LEFT;
  option_key_input[CONTROLS_KEY_RIGHT][0] = 'D';
  option_key_input[CONTROLS_KEY_RIGHT][1] = GLFW_KEY_RIGHT;
  option_key_input[CONTROLS_KEY_RUN][0] = GLFW_KEY_LEFT_SHIFT;
  option_key_input[CONTROLS_KEY_RUN][1] = GLFW_KEY_RIGHT_SHIFT;
  option_key_input[CONTROLS_KEY_JUMP][0] = ' ';
  option_key_input[CONTROLS_KEY_JUMP][1] = 0;
  option_key_input[CONTROLS_KEY_CHAT][0] = GLFW_KEY_ENTER;
  option_key_input[CONTROLS_KEY_CHAT][1] = 'T';
  option_key_input[CONTROLS_KEY_BLOCK_MENU][0] = 'B';
  option_key_input[CONTROLS_KEY_BLOCK_MENU][1] = 0;
  option_key_input[CONTROLS_KEY_SERVER_MENU][0] = GLFW_KEY_TAB;
  option_key_input[CONTROLS_KEY_SERVER_MENU][1] = GLFW_KEY_F2;
  option_key_input[CONTROLS_KEY_FLY_UP][0] = 'Q';
  option_key_input[CONTROLS_KEY_FLY_UP][1] = GLFW_KEY_HOME;
  option_key_input[CONTROLS_KEY_FLY_DOWN][0] = 'E';
  option_key_input[CONTROLS_KEY_FLY_DOWN][1] = GLFW_KEY_END;
  option_key_input[CONTROLS_KEY_MODE_FLY][0] = 'Z';
  option_key_input[CONTROLS_KEY_MODE_FLY][1] = 0;
  option_key_input[CONTROLS_KEY_MODE_NOCLIP][0] = 'X';
  option_key_input[CONTROLS_KEY_MODE_NOCLIP][1] = 0;
  option_key_input[CONTROLS_KEY_BLOCK_CREATE][0] = CONTROLS_MOUSE_KEY_LEFT;
  option_key_input[CONTROLS_KEY_BLOCK_CREATE][1] = 0;
  option_key_input[CONTROLS_KEY_BLOCK_DESTROY][0] = CONTROLS_MOUSE_KEY_RIGHT;
  option_key_input[CONTROLS_KEY_BLOCK_DESTROY][1] = 0;
  option_key_input[CONTROLS_KEY_BLOCK_CLONE][0] = CONTROLS_MOUSE_KEY_MIDDLE;
  option_key_input[CONTROLS_KEY_BLOCK_CLONE][1] = 0;
  option_key_input[CONTROLS_KEY_SNEAK][0] = 0;
  option_key_input[CONTROLS_KEY_SNEAK][1] = 0;
  option_key_input[CONTROLS_KEY_BLOCK_REPLACE][0] = 0;
  option_key_input[CONTROLS_KEY_BLOCK_REPLACE][1] = 0;
}

//--page-split-- option_load

void option_load() {

  option_fps_limit = 60;
  option_fps_goal = 20;
  option_mouse_invert = 0;
  option_mouse_reverse_unused = 0;
  option_fog_type = 1;
  option_fog_distance = 256;
  option_perspective_angle = 90;
  option_anaglyph_enable = 0;
  option_anaglyph_pixel_size = 0.03;
  option_anaglyph_distance = 50.0;
  option_fps_display = 0;
  option_show_f1_help = 1;
  option_request_vertical_sync = 0;
  option_noclip = 0;
  option_chat_return = 1;
  option_chat_clear = 1;
  option_optimize_chunks = 1;
  option_window_width = 0;
  option_window_height = 0;
  option_window_location = 1;
  option_remember_size = 0;
  option_fsaa_samples = 0;
  option_multiple_paste = 0;
  option_custom_fps = 100;
  option_hyper_help = 1;
  memset(option_cookie_data, 0, 20);
  option_have_cookie = 0;
  option_blender_font = 0;
  option_lighting = 1;
  option_always_run = 0;
  option_fly_fast = 0;
  option_textures = 1;
  option_advertise = 1;
  option_sound = 1;
  option_hud_display = 1;
  option_superhuman = 1;
  option_volume = 1.0f;
  option_anisotropic_filtering = 0;
  option_window_location_x = 0;
  option_window_location_y = 0;
  option_anaglyph_units = 0;
  option_pupil_distance = 5.5;
  option_anaglyph_filter[0] = 0;
  option_anaglyph_filter[1] = 4;

  option_key_reset();

  FILE *file;
  char buffer[OPTION_SIZE] = {};

  int file_offset = 0;
  int file_limit = 0;

  #define READ_BYTE(x) if (file_limit >= file_offset + 1) x = BYTE(file_offset); else goto DONE; file_offset += 1
  #define READ_WORD(x) if (file_limit >= file_offset + 2) x = WORD(file_offset); else goto DONE; file_offset += 2
  #define READ_UINT(x) if (file_limit >= file_offset + 4) x = UINT(file_offset); else goto DONE; file_offset += 4
  #define READ_FLOAT(x) if (file_limit >= file_offset + 4) x = FLOAT(file_offset); else goto DONE; file_offset += 4
  #define READ_DOUBLE(x) if (file_limit >= file_offset + 8) x = DOUBLE(file_offset); else goto DONE; file_offset += 8

  file = fopen("options.bin", "rb");
  if (file != NULL) {
    file_limit = fread(buffer, 1, OPTION_SIZE, file);
    file_offset += 4; // Magic Number
    file_offset += 4; // I dunno...
    READ_BYTE(option_fps_limit);
    READ_BYTE(option_fps_goal);
    READ_BYTE(option_mouse_invert);
    READ_BYTE(option_mouse_reverse_unused);
    READ_WORD(option_fog_type);
    READ_WORD(option_fog_distance);
    READ_BYTE(option_perspective_angle);
    READ_BYTE(option_anaglyph_enable);
    READ_DOUBLE(option_anaglyph_pixel_size);
    READ_DOUBLE(option_anaglyph_distance);
    READ_BYTE(option_fps_display);
    file_offset += 1; // option_wom_addict
    READ_BYTE(option_show_f1_help);
    READ_BYTE(option_request_vertical_sync);
    READ_BYTE(option_noclip);
    READ_BYTE(option_chat_return);
    READ_BYTE(option_chat_clear);
    READ_BYTE(option_optimize_chunks);
    READ_WORD(option_window_width);
    READ_WORD(option_window_height);
    READ_BYTE(option_window_location);
    READ_BYTE(option_remember_size);
    READ_BYTE(option_fsaa_samples);
    READ_BYTE(option_multiple_paste);
    READ_WORD(option_custom_fps);
    READ_BYTE(option_hyper_help);
    if (file_limit >= file_offset + 20) {
      memmove(option_cookie_data, &BYTE(file_offset), 20);
      fprintf(stderr, "loaded cookie data from options file\n");
      file_offset += 20;
    } else goto DONE;
    READ_BYTE(option_have_cookie);
    file_offset += 1; // option_rainbow_mute
    READ_BYTE(option_always_run);
    READ_BYTE(option_blender_font);
    READ_BYTE(option_lighting);
    READ_BYTE(option_fly_fast);
    READ_BYTE(option_textures);
    READ_BYTE(option_advertise);
    file_offset += 1; // option_hd_lighting
    READ_BYTE(option_sound);
    READ_BYTE(option_hud_display);
    READ_BYTE(option_superhuman);
    READ_FLOAT(option_volume);
    READ_UINT(option_key_input[CONTROLS_KEY_FORWARD][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_FORWARD][1]);
    READ_UINT(option_key_input[CONTROLS_KEY_BACK][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_BACK][1]);
    READ_UINT(option_key_input[CONTROLS_KEY_LEFT][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_LEFT][1]);
    READ_UINT(option_key_input[CONTROLS_KEY_RIGHT][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_RIGHT][1]);
    READ_UINT(option_key_input[CONTROLS_KEY_RUN][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_RUN][1]);
    READ_UINT(option_key_input[CONTROLS_KEY_JUMP][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_JUMP][1]);
    READ_UINT(option_key_input[CONTROLS_KEY_CHAT][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_CHAT][1]);
    READ_UINT(option_key_input[CONTROLS_KEY_BLOCK_MENU][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_BLOCK_MENU][1]);
    READ_UINT(option_key_input[CONTROLS_KEY_SERVER_MENU][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_SERVER_MENU][1]);
    READ_UINT(option_key_input[CONTROLS_KEY_FLY_UP][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_FLY_UP][1]);
    READ_UINT(option_key_input[CONTROLS_KEY_FLY_DOWN][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_FLY_DOWN][1]);
    READ_UINT(option_key_input[CONTROLS_KEY_MODE_NOCLIP][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_MODE_NOCLIP][1]);
    READ_UINT(option_key_input[CONTROLS_KEY_BLOCK_CREATE][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_BLOCK_CREATE][1]);
    READ_UINT(option_key_input[CONTROLS_KEY_BLOCK_DESTROY][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_BLOCK_DESTROY][1]);
    READ_UINT(option_key_input[CONTROLS_KEY_BLOCK_CLONE][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_BLOCK_CLONE][1]);
    READ_UINT(option_key_input[CONTROLS_KEY_SNEAK][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_SNEAK][1]);
    READ_BYTE(option_anisotropic_filtering);
    READ_UINT(option_key_input[CONTROLS_KEY_BLOCK_REPLACE][0]);
    READ_UINT(option_key_input[CONTROLS_KEY_BLOCK_REPLACE][1]);
    READ_WORD(option_window_location_x);
    READ_WORD(option_window_location_y);
    READ_BYTE(option_anaglyph_units);
    READ_DOUBLE(option_pupil_distance);
    READ_BYTE(option_anaglyph_filter[0]);
    READ_BYTE(option_anaglyph_filter[1]);

    fclose(file);

    DONE:;
    for (int a = 0; a < CONTROLS_KEY_LIMIT; a++) {
      for (int b = 0; b < 2; b++) {
        if (option_key_input[a][b] < 0 || option_key_input[a][b] > CONTROLS_MOUSE_KEY_LIMIT) {
          printf ("Invalid control key value; resetting keys to defaults.\n");
          option_key_reset();
          break;
        };
      };
    };
  };

  option_optimize_chunks = 1;

};

//--page-split-- option_save

void option_save() {

  FILE *file;
  char buffer[OPTION_SIZE] = {};

  int file_offset = 0;

  #define WRITE_BYTE(x) BYTE(file_offset) = x; file_offset += 1;
  #define WRITE_WORD(x) WORD(file_offset) = x; file_offset += 2;
  #define WRITE_UINT(x) UINT(file_offset) = x; file_offset += 4;
  #define WRITE_FLOAT(x) FLOAT(file_offset) = x; file_offset += 4;
  #define WRITE_DOUBLE(x) DOUBLE(file_offset) = x; file_offset += 8;

  WRITE_UINT(0xEFEACF7F);
  WRITE_UINT(0x0);
  WRITE_BYTE(option_fps_limit);
  WRITE_BYTE(option_fps_goal);
  WRITE_BYTE(option_mouse_invert);
  WRITE_BYTE(option_mouse_reverse_unused);
  WRITE_WORD(option_fog_type);
  WRITE_WORD(option_fog_distance);
  WRITE_BYTE(option_perspective_angle);
  WRITE_BYTE(option_anaglyph_enable);
  WRITE_DOUBLE(option_anaglyph_pixel_size);
  WRITE_DOUBLE(option_anaglyph_distance);
  WRITE_BYTE(option_fps_display);
  file_offset += 1; // option_wom_addict
  WRITE_BYTE(option_show_f1_help);
  WRITE_BYTE(option_request_vertical_sync);
  WRITE_BYTE(option_noclip);
  WRITE_BYTE(option_chat_return);
  WRITE_BYTE(option_chat_clear);
  WRITE_BYTE(option_optimize_chunks);
  WRITE_WORD(option_window_width);
  WRITE_WORD(option_window_height);
  WRITE_BYTE(option_window_location);
  WRITE_BYTE(option_remember_size);
  WRITE_BYTE(option_fsaa_samples);
  WRITE_BYTE(option_multiple_paste);
  WRITE_WORD(option_custom_fps);
  WRITE_BYTE(option_hyper_help);
  memmove(&BYTE(file_offset), option_cookie_data, 20); file_offset += 20;
  WRITE_BYTE(option_have_cookie);
  file_offset += 1; // option_rainbow_mute
  WRITE_BYTE(option_always_run);
  WRITE_BYTE(option_blender_font);
  WRITE_BYTE(option_lighting);
  WRITE_BYTE(option_fly_fast);
  WRITE_BYTE(option_textures);
  WRITE_BYTE(option_advertise);
  file_offset += 1; // option_hd_lighting
  WRITE_BYTE(option_sound);
  WRITE_BYTE(option_hud_display);
  WRITE_BYTE(option_superhuman);
  WRITE_FLOAT(option_volume);
  WRITE_UINT(option_key_input[CONTROLS_KEY_FORWARD][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_FORWARD][1]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_BACK][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_BACK][1]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_LEFT][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_LEFT][1]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_RIGHT][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_RIGHT][1]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_RUN][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_RUN][1]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_JUMP][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_JUMP][1]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_CHAT][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_CHAT][1]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_BLOCK_MENU][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_BLOCK_MENU][1]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_SERVER_MENU][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_SERVER_MENU][1]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_FLY_UP][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_FLY_UP][1]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_FLY_DOWN][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_FLY_DOWN][1]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_MODE_NOCLIP][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_MODE_NOCLIP][1]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_BLOCK_CREATE][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_BLOCK_CREATE][1]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_BLOCK_DESTROY][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_BLOCK_DESTROY][1]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_BLOCK_CLONE][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_BLOCK_CLONE][1]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_SNEAK][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_SNEAK][1]);
  WRITE_BYTE(option_anisotropic_filtering);
  WRITE_UINT(option_key_input[CONTROLS_KEY_BLOCK_REPLACE][0]);
  WRITE_UINT(option_key_input[CONTROLS_KEY_BLOCK_REPLACE][1]);
  WRITE_WORD(option_window_location_x);
  WRITE_WORD(option_window_location_y);
  WRITE_BYTE(option_anaglyph_units);
  WRITE_DOUBLE(option_pupil_distance);
  WRITE_BYTE(option_anaglyph_filter[0]);
  WRITE_BYTE(option_anaglyph_filter[1]);

  #ifdef UNIX
  close(open("options.bin", O_CREAT, 0600));
  file = fopen("options.bin", "r+b");
  #else
  file = fopen("options.bin", "wb");
  #endif

  if (file != NULL) {
    fwrite(buffer, file_offset, 1, file);
    #ifdef UNIX
    ftruncate(fileno(file), file_offset);
    #endif
    fclose(file);
  };

};
