#pragma once

// This little area is just to define what I want to syntax check while
// editing the source code in my text editor.

#ifndef WIN32
  #ifndef LINUX
    #ifndef FREEBSD
      #ifndef UNIX
        #define LINUX
        #define UNIX
      #endif
    #endif
  #endif
#endif

//--page-split-- #include statements

#define _GNU_SOURCE

#ifdef WINDOWS
  #include <w32api.h>
  #define WINVER WindowsXP
  #include <winsock2.h>
  #include <windows.h>
  #include <ws2tcpip.h>
#else
  #include <errno.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  #include <netinet/tcp.h>
  #include <pthread.h>
  #include <signal.h>
#endif

#ifdef LINUX
  #include <alsa/asoundlib.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <fcntl.h>
#include <ctype.h>
#include <fenv.h>

#ifdef WINDOWS
#include "static/glfw3.h"
#include "static/zlib.h"
#else
#include "GLFW/glfw3.h"
#include "zlib.h"
#endif

//#include "../glfw/glfw-2.7.7/include/GL/glfw.h"
//#include "../libraries/zlib.h"
#include "GL/glu.h"

#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_vorbis.h"

#include "protocol.h"

//--page-split-- global constants

// Only "really" global constants go here.  Constants that mostly belong
// to one file are listed along with that file's function prototypes.

#define PORTAL_ADDRESS "portal.multiplayermapeditor.com:44434"

#define TRUE 1
#define FALSE 0
#define NO_FLAGS 0
#define FLAGS NO_FLAGS

#define PLAYER_HEIGHT_METERS 1.63
#define CAMERA_HEIGHT_METERS 1.51 // 1.51 - 0.06 pivot point, 1.45

//--page-split-- global structures

// Only "really" global structures go here.  Structures that mostly belong
// to one file are listed along with that file's function prototypes.

struct double_xyzuv {
  double x, y, z, u, v;
};

struct double_xyz {
  double x, y, z;
};

struct double_xy {
  double x, y;
};

struct double_uv {
  double u, v;
};

struct int_xyzuv {
  int x, y, z, u, v;
};

struct int_xyz {
  int x, y, z;
};

struct char_xyz {
  char x, y, z;
};

struct int_xy {
  int x, y;
};

struct int_st {
  int s, t;
};

struct int_uv {
  int u, v;
};

struct int_xyzs {
  int x, y, z, s;
};

//--page-split-- global macros

// Only "really" global macors go here.  Macros that mostly belong
// to one file are listed along with that file's function prototypes.

#define FUCK printf("\e[1;33m%s:%d\e[0m\n", __FILE__, __LINE__);

#define WHAT display_check_opengl_error

#ifdef TEST
  #define DEBUG(message) fprintf(stderr, "DEBUG: %s\n", message)
  #define lag_push(limit, message) { printf("%s:%d: lag_push(\"%s\")\n", __FILE__, __LINE__, message); lag__push(limit, message); }
  #define lag_pop() { printf("%s:%d: lag_pop()\n", __FILE__, __LINE__); lag__pop(); }
#else
  #define DEBUG(message)
  #define lag_push(limit, message)
  #define lag_pop()
#endif

//--page-split-- global variables & function prototypes

// alsa.h

extern char *alsa_error_function;
extern char *alsa_error_message;
void alsa_initialize();
void alsa_terminate();

// argument.h

extern int argument_packets;
extern int argument_server;
extern int argument_modes;
extern int argument_no_threads;
extern int argument_fullscreen;
extern int argument_lag;
extern int argument_hacks;
extern int argument_record_all_frames;

void argument_check (int argc, char **argv);

// block.h

#define BLOCK_MAX_BLOCKS 257
#define BLOCK_SIDE_UP 0
#define BLOCK_SIDE_FRONT 1
#define BLOCK_SIDE_RIGHT 2
#define BLOCK_SIDE_LEFT 3
#define BLOCK_SIDE_BACK 4
#define BLOCK_SIDE_DOWN 5
#define BLOCK_SIDE_COUNT 6

struct structure_block_data {
  char *comment;         // Text description of this block type.
  int visible;           // Whether or not anything needs to be rendered.
  int reverse;           // Draw reverse sides of transparent textures.
  int between;           // Draw between transparent blocks of same type.
  int impassable;        // Can the player walk into this block?
  int transparent;       // For determining where textures render.
  int emission;          // light emission type
  int index[6];          // Index into texture_data[] for each side.
};

extern struct structure_block_data block_data[BLOCK_MAX_BLOCKS];

void block_reset();

// build.h

extern char build_target[];
extern char build_compile_date[];
extern int build_time_utc;
extern int build_svn_revision;
extern int build_svn_modified;
extern char build_svn_string[];

// cache.h

extern char *cache_data;
extern int cache_size;

int cache_validate(char *digest);

// chat.h

extern int chat_array_count;
extern int chat_color_cycle_index;

void chat_reset();
void chat_color(int color);
void chat_color_decode(char **string, char **colors, char *input, int strip_spaces);
void chat_message(char *message);
void chat_render(int x, int y, int width, int height, int flag, int scroll_offset);

// controls.h

void controls_initialize();
int controls_key_is_invalid (int key);
char *controls_key_name (int value);
int controls_get_key(int key_id);
int controls_menu_key (int key_id);
int controls_menu_pressed (int key_id);
int controls_menu_released (int key_id);

// these are used as indexes array of what key should do what

#define CONTROLS_KEY_FORWARD 0
#define CONTROLS_KEY_BACK 1
#define CONTROLS_KEY_LEFT 2
#define CONTROLS_KEY_RIGHT 3
#define CONTROLS_KEY_RUN 4
#define CONTROLS_KEY_JUMP 5
#define CONTROLS_KEY_CHAT 6
#define CONTROLS_KEY_BLOCK_MENU 7
#define CONTROLS_KEY_SERVER_MENU 8
#define CONTROLS_KEY_FLY_UP 9
#define CONTROLS_KEY_FLY_DOWN 10
#define CONTROLS_KEY_MODE_FLY 11
#define CONTROLS_KEY_MODE_NOCLIP 12
#define CONTROLS_KEY_BLOCK_CREATE 13
#define CONTROLS_KEY_BLOCK_DESTROY 14
#define CONTROLS_KEY_BLOCK_CLONE 15
#define CONTROLS_KEY_SNEAK 16
#define CONTROLS_KEY_BLOCK_REPLACE 17
#define CONTROLS_KEY_COUNT 18
#define CONTROLS_KEY_LIMIT 64

// these are used as fake key codes for the mouse events

#define CONTROLS_MOUSE_KEY (GLFW_KEY_LAST + 1)
#define CONTROLS_MOUSE_KEY_LEFT (CONTROLS_MOUSE_KEY + 0)
#define CONTROLS_MOUSE_KEY_RIGHT (CONTROLS_MOUSE_KEY + 1)
#define CONTROLS_MOUSE_KEY_MIDDLE (CONTROLS_MOUSE_KEY + 2)
#define CONTROLS_MOUSE_KEY_BUTTON4 (CONTROLS_MOUSE_KEY + 3)
#define CONTROLS_MOUSE_KEY_BUTTON5 (CONTROLS_MOUSE_KEY + 4)
#define CONTROLS_MOUSE_KEY_BUTTON6 (CONTROLS_MOUSE_KEY + 5)
#define CONTROLS_MOUSE_KEY_BUTTON7 (CONTROLS_MOUSE_KEY + 6)
#define CONTROLS_MOUSE_KEY_BUTTON8 (CONTROLS_MOUSE_KEY + 7)
#define CONTROLS_MOUSE_KEY_LIMIT (CONTROLS_MOUSE_KEY + 8)

// cpu.h

extern double cpu_sleep_time;
extern int cpu_core_count;

void cpu_initialize();
void cpu_analyze();

// directsound.h

extern char *directsound_error_function;
extern char *directsound_error_message;
void directsound_initialize();
void directsound_terminate();

// display.h

void display_open_window();
void display_close_window();
extern int display_window_width, display_window_height;
void display_render();
void display_background_image();
void display_check_opengl_error(char * message);

// easy.h

void easy_fuck(char * message);
void easy_error(char * message, int error_code);
double easy_time();
void easy_sleep(double seconds);
void easy_seed_random_number_generator();
int easy_random(int limit);
void easy_random_binary_string(char *string, int bytes);
void easy_random_hex_string(char *string, int length);
void easy_binary_to_ascii(char *output, char *input, int size);
int easy_strnlen(char *string, int limit);

// error.h

const char *error_string(int error_code);

// event.h

#define EVENT_LIST_SIZE 1024

extern int event_mouse_button_state;
extern char event_key_state[530];
extern int event_list[EVENT_LIST_SIZE][5];
extern int event_list_index;
int event_translate_from_unicode(int character);
void event_open_window();
void event_receive();
void event_move_mouse(double x, double y);
void event_keyboard_char_callback(GLFWwindow *window, unsigned int character);
void event_keyboard_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void event_mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void event_mouse_scroll_callback(GLFWwindow *window, double x, double y);
void event_mouse_position_callback(GLFWwindow *window, double x, double y);
extern double event_mouse_position_x;
extern double event_mouse_position_y;

// event_macros.h

/*

  Documentation of "event_list[][]"

  It's a list of GLFW events that have occured since the last frame.

  event_list[][0] tells what kind of event:
    0 = keyboard character (letters and symbols)
    1 = keyboard key (arrow keys, shift keys, etc.)
    2 = mouse button
    3 = mouse wheel

  0: Keyboard Character
    [][1] = GLFW_PRESS or GLFW_RELEASE
    [][2] = character
    [][3] = Mouse X position at time of event.
    [][4] = Mouse Y position at time of event.

  1: Keyboard Key
    [][1] = GLFW_PRESS or GLFW_RELEASE
    [][2] = GLFW Key Code
    [][3] = Mouse X position at time of event.
    [][4] = Mouse Y position at time of event.

  2: Mouse Button
    [][1] = GLFW_PRESS or GLFW_RELEASE
    [][2] = GLFW_MOUSE_BUTTON_LEFT or GLFW_MOUSE_BUTTON_RIGHT
    [][3] = Mouse X position at time of event.
    [][4] = Mouse Y position at time of event.

  3: Mouse Wheel
    [][1] = undefined
    [][2] = Delta change in mouse wheel position.
    [][3] = Mouse X position at time of event.
    [][4] = Mouse Y position at time of event.

  4: Mouse Position
    [][1] = undefined
    [][2] = undefined
    [][3] = Mouse X position at time of event.
    [][4] = Mouse Y position at time of event.

  So, since offsets are identical for multiple events, let's macro them!

*/

#define CHARACTER_EVENT (event_list[menu_current_event][0] == 0 && event_list[menu_current_event][1] == GLFW_PRESS)
#define KEY_PRESS_EVENT (event_list[menu_current_event][0] == 1 && event_list[menu_current_event][1] == GLFW_PRESS)
#define KEY_REPEAT_EVENT (event_list[menu_current_event][0] == 1 && event_list[menu_current_event][1] == GLFW_REPEAT)
#define KEY_RELEASE_EVENT (event_list[menu_current_event][0] == 1 && event_list[menu_current_event][1] == GLFW_RELEASE)
#define BUTTON_EVENT (event_list[menu_current_event][0] == 2)
#define WHEEL_EVENT (event_list[menu_current_event][0] == 3)
#define HOVER_EVENT (event_list[menu_current_event][0] == 4)
#define PRESS (event_list[menu_current_event][1] == GLFW_PRESS)
#define RELEASE (event_list[menu_current_event][1] == GLFW_RELEASE)
#define LEFT_BUTTON (event_list[menu_current_event][2] == GLFW_MOUSE_BUTTON_LEFT)
#define RIGHT_BUTTON (event_list[menu_current_event][2] == GLFW_MOUSE_BUTTON_RIGHT)
#define MIDDLE_BUTTON (event_list[menu_current_event][2] == GLFW_MOUSE_BUTTON_MIDDLE)
#define CHARACTER event_list[menu_current_event][2]
#define KEY event_list[menu_current_event][2]
#define DELTA event_list[menu_current_event][2]
#define MOUSE_X (event_list[menu_current_event][3] + 0.5)
#define MOUSE_Y (event_list[menu_current_event][4] + 0.5)

#define LEFT_PRESS (KEY_PRESS_EVENT && KEY == CONTROLS_MOUSE_KEY_LEFT)
#define MIDDLE_PRESS (KEY_PRESS_EVENT && KEY == CONTROLS_MOUSE_KEY_MIDDLE)
#define RIGHT_PRESS (KEY_PRESS_EVENT && KEY == CONTROLS_MOUSE_KEY_RIGHT)
#define LEFT_RELEASE (KEY_RELEASE_EVENT && KEY == CONTROLS_MOUSE_KEY_LEFT)
#define MIDDLE_RELEASE (KEY_RELEASE_EVENT && KEY == CONTROLS_MOUSE_KEY_MIDDLE)
#define RIGHT_RELEASE (KEY_RELEASE_EVENT && KEY == CONTROLS_MOUSE_KEY_RIGHT)

// Note: It is impossible to match MOUSE_TEST(7, 7, 7, 7) because the specified area has zero size.
#define MOUSE_TEST(s, t, u, v) ((MOUSE_X >= (s)) && (MOUSE_Y >= (t)) && (MOUSE_X < (u)) && (MOUSE_Y < (v)))

// glfw.h

extern int glfw_mouse_capture_flag;
extern int glfw_mouse_x_center;
extern int glfw_mouse_y_center;
extern GLFWwindow *glfw_window;

void glfw_initialize();
void glfw_terminate();
void glfw_open_window();
void glfw_close_window();
void glfw_capture_mouse();
void glfw_release_mouse();
void glfw_fullscreen(int enable);
void glfw_set_window_title(char *title);

// group.h

struct structure_group_data {
  char *name;
  int size;
  int *list;
};

extern struct structure_group_data group_data[256];

void group_reset();

// gui.h

extern int gui_menu_x_offset;
extern int gui_menu_y_offset;
extern int gui_menu_width;
extern int gui_menu_height;
extern int gui_text_columns;
extern int gui_text_lines;

void gui_open_window();
void gui_close_window();
int gui_window(int x, int y, int flags);
void gui_model_name(char *string);
void gui_text(int column, int row, char *string);
void gui_draw_text(int column, int row, char *string, char *colors, int flags);
int gui_button(int x, int y, int w, char *string, int flags);
int gui_radio(int x, int y, int *p, int v, char *string);
int gui_check(int x, int y, int *p, int v, char *string);
int gui_input(int x, int y, int size, int length, char *string, int flags);
int gui_link(int x, int y, char *string);
void gui_texture(int column, int line, int width, int height, int texture, double ts, double tt, double tu, double tv);
void gui_draw_block (int block_type, int column, int line, int width, int height);
void gui_percent (double column, double line, double width, double height, double percent, int chat_color_bg, int chat_color_fg);
int gui_scroll_bar_vertical (double column, double line, double width, double height, int max_lines, int page_lines, int current_line);

// lag.h

void lag__push(int limit, const char *message);
void lag__pop();
void lag_initialize();
void lag_terminate();

// main.h

extern int main_restart_flag;
extern int main_shutdown_flag;
extern int main_terminate_flag;

void main_restart();
void main_shutdown();

// map.h

struct structure_map_data {
  char *block;                   // Block data.
  struct int_xyz dimension;      // Dimensions of map.
  int limit;                     // Total number of blocks.
  struct double_xyz resolution;  // Size of each block in meters.
  struct int_xyz wrap;           // Whether the map should wrap at the edges.
  int sealevel;                  // Z value of ground level.
};

struct structure_map_selection {
  int valid;
  int side;
  struct int_xyz create;
  struct int_xyz destroy;
};

extern double map_percentage_in_view;
extern struct structure_map_data map_data;
extern int map_chunk_bits;
extern int map_initialization_flag;
extern int map_chunk_thread_count;
extern int map_cursor_color;
extern int map_selection_color;
extern struct int_xyz map_selection_corner_one;
extern struct int_xyz map_selection_corner_two;
extern struct int_xyz map_selection_dimensions;
extern double map_perspective_angle;
extern struct structure_map_selection map_selection;

void map_initialize();
void map_open_window();
void map_close_window();
void map_begin_render();
void map_cease_render();
void map_invalidate_chunks();
int map_is_active();
void map_render();
void map_modify_bunch(struct int_xyz one, struct int_xyz two, int t, int priority);
void map_modify(struct int_xyz block, int t, int priority);
int map_get_block_type(struct int_xyz block);
void map_mouse_test();
int map_process_a_chunk();
void *map_chunk_thread(void *parameter);

// math.h

void math_rotate_vector(struct double_xyzuv *vector);
void math_reverse_rotate_vector(struct double_xyzuv *vector);
void math_origin_vector(struct double_xyzuv *vector);
void math_normalize_vector(struct double_xyz *vector);

// memory.h

extern struct structure_rwlock memory_lock;

#define memory_allocate(pointer, size) { void *d95NNwASESLjBSXFn3k2 = *(pointer); memory__allocate(&d95NNwASESLjBSXFn3k2, size, __FILE__, __LINE__); *(pointer) = d95NNwASESLjBSXFn3k2; }
void memory__allocate(void **pointer, int size, char *file, int line);
void memory_initialize();
void memory_terminate();

// menu.h

#define MENU_MAX_OBJECTS 512

#define MENU_NONE 0
#define MENU_TEXT 1
#define MENU_LINK 2
#define MENU_CHECK 3
#define MENU_RADIO 4
#define MENU_INPUT 5
#define MENU_BUTTON 6
#define MENU_PASSWORD 7
#define MENU_TEXTURE 8

#define MENU_FLAG_NORMAL 0
#define MENU_FLAG_ACTIVE 1
#define MENU_FLAG_LIVE 2
#define MENU_FLAG_DISABLE 4
#define MENU_FLAG_OFFSET 8
#define MENU_FLAG_STARS 0x100

extern void (*menu_function_pointer)();
extern int menu_object_data[MENU_MAX_OBJECTS][16];
extern int menu_object_index;
extern int menu_focus_object;
extern int menu_next_focus;
extern int menu_process_event;
extern int menu_current_event;
extern int menu_draw_widget;
extern int menu_modifier_shift;
extern int menu_display_data;

void menu_switch(void (*menu)());
void menu_process();
void menu_render();

// menus.h

struct structure_server_menu {
  int type;
  int column;
  int line;
  int width;
  int length;
  int flags;
  int name;
  int value;
  char *text;
  int salt_count;
  char the_salt[20];
  char salt_one[20];
  char salt_two[20];
  int height;
  int texture;
  char coordinate[4];
};

void menus_anaglyph();
void menus_autofog();
void menus_chat_scroll_reset();
void menus_chat();
void menus_controls();
void menus_controls_key_select ();
void menus_escape();
void menus_exit_immediately();
void menus_exit();
void menus_fencepost();
void menus_fog();
void menus_fps();
void menus_graphics();
void menus_group();
void menus_hacks();
void menus_help();
void menus_misc();
void menus_perspective();
extern int menus_play_box_enable;
extern int menus_play_box_dimension_limit;
extern int menus_play_box_volume_limit;
extern int menus_play_create_type;
extern int menus_play_destroy_type;
extern int menus_play_last_click_valid;
extern struct int_xyz menus_play_last_click;
void menus_play_reset();
void menus_play_focus_lost();
void menus_play();
void menus_server_connect();
void menus_server_disconnect();
void menus_server_error();
extern int menus_server_loading_active;
extern int menus_server_loading_progress;
extern char *menus_server_loading_message;
void menus_server_loading();
extern struct structure_server_menu menus_server_menu_data[MENU_MAX_OBJECTS - 1];
extern int menus_server_menu_value[256];
extern int menus_server_menu_active;
extern int menus_server_menu_number;
extern int menus_server_menu_width;
extern int menus_server_menu_height;
extern int menus_server_menu_submit;
extern int menus_server_menu_options;
void menus_server_menu_reset();
void menus_server_menu();
void menus_server_select();
void menus_sound();

extern int menus_server_menu_value[256];
extern int menus_server_menu_active;
extern int menus_server_menu_number;
extern int menus_server_menu_width;
extern int menus_server_menu_height;
extern int menus_server_menu_submit;
extern int menus_server_menu_options;

extern int menus_controls_disable_function_keys;

// mixer.h

void mixer_initialize();
void mixer_generate(float *buffer, int frames);
void mixer_play(short *buffer, int count, float volume, struct double_xyz *location);

// model.h

struct structure_model_player {
  struct double_xyzuv position;
  int valid;
  int color;
  char name[25];
  int texture;
};

void model_open_window();
void model_close_window();
void model_reset();
extern struct structure_model_player model_player[256];
void model_move(int player, struct double_xyzuv position);
void model_render();

// network.h

struct structure_socket_data;

extern double network_last_response;
extern char *network_error_string;
void network_initialize();
void *network_initiate_connect(struct structure_socket_data **the_socket, char *address);
void network_free_socket(struct structure_socket_data **the_socket);
int network_status(struct structure_socket_data **the_socket);
int network_read(struct structure_socket_data **the_socket, char **buffer, int *size);
int network_write(struct structure_socket_data **the_socket, char **buffer, int *size);

// on.h

extern int on_activate_lag;
extern double on_frame_time;

void on_program_start(int argc, char **argv);
void on_program_exit();
void on_frame();
void on_open_window();
void on_close_window();
void on_map_load();
void on_map_unload();
void on_server_connect();
void on_server_disconnect();

// opengl.h

void opengl_extentions();

// option.h

extern int option_fps_limit;
extern int option_fps_goal;
extern int option_fog_type;
extern int option_fog_distance;
extern int option_mouse_invert;
extern int option_mouse_reverse_unused;
extern int option_perspective_angle;
extern int option_anaglyph_enable;
extern int option_show_f1_help;
extern double option_anaglyph_pixel_size;
extern double option_anaglyph_distance;
extern int option_fps_display;
extern int option_request_vertical_sync;
extern int option_noclip;
extern int option_chat_return;
extern int option_chat_clear;
extern int option_optimize_chunks;
extern int option_window_width;
extern int option_window_height;
extern int option_window_location;
extern int option_remember_size;
extern int option_fsaa_samples;
extern int option_multiple_paste;
extern int option_custom_fps;
extern int option_hyper_help;
extern char option_cookie_data[20];
extern int option_have_cookie;
extern int option_blender_font;
extern int option_lighting;
extern int option_always_run;
extern int option_fly_fast;
extern int option_textures;
extern int option_advertise;
extern int option_sound;
extern int option_hud_display;
extern int option_superhuman;
extern float option_volume;
extern int option_key_input[CONTROLS_KEY_LIMIT][2];
extern int option_anisotropic_filtering;
extern int option_f3_display;
extern int option_window_location_x;
extern int option_window_location_y;
extern int option_anaglyph_units;
extern double option_pupil_distance;

void option_key_reset();
void option_load();
void option_save();

// packet.h

extern const char *packet_type_string[];
char *packet_parse_stream(char *buffer, int length);

void packet_reset();
int packet_is_sendable(int packet_type);
int packet_send(int packet_type, ...);
const char *packet_type_to_string(int packet_type);

// paint.h

void paint_reset();
void paint_something ();

#define MAX_PAINT 1000

extern struct int_xyz (*paint_list)[MAX_PAINT];
extern int paint_list_size;
extern int paint_target_type;
extern int paint_with_type;
extern double paint_start_time;
extern int paint_total;

// player.h

extern int player_allow_flying;
extern int player_allow_speedy;
extern int player_allow_noclip;
extern struct double_xyzuv player_view;
extern struct double_xyzuv player_position;
extern struct double_xyzuv player_velocity;
extern int player_fly;
extern int player_noclip;
extern int player_stuck;
extern double player_mouse_x_accumulator;
extern double player_mouse_y_accumulator;

void player_reset();
void player_teleport(struct double_xyzuv position);
void player_movement();
int player_get_vector(struct int_xyz *v);

// projectile.h

void projectile_reset();
void projectile_render();
void projectile_movement();
void projectile_create(int who, int type, struct double_xyzuv start_position);

// sanity.h

void sanity_check();

// server.h

#define SERVER_ACTION_CONNECT              1
#define SERVER_ACTION_DISCONNECT           2
#define SERVER_STATUS_CONNECTING           3
#define SERVER_STATUS_CONNECTED            4
#define SERVER_STATUS_DISCONNECTING        5
#define SERVER_STATUS_DISCONNECTED         6
#define SERVER_STATUS_ERROR                7

extern int server_action;
extern int server_status;
extern char *server_error_string;
extern double server_reconnect_time;
extern int server_input_size;
extern int server_output_size;
extern double server_connect_time;
extern double server_ping_time;
extern char *server_address;
extern char *server_authentication;
extern char *server_portal_authentication;

void server_terminate();
void server_stuff();
void server_menu_request();
void server_send_raw_data(char *buffer, int length);
void server_process_packet(int packet_type, char *packet_data, int packet_size);
void server_modify(struct int_xyz block, int type);
void server_modify_bunch(struct int_xyz one, struct int_xyz two, int type);
int server_paste(struct int_xyz lower_corner, struct int_xyz dimension, char *data);

// sha1.h

void sha1(char *digest, char *message, int length);

// sky.h

extern int sky_box_flags;
extern int sky_box_type;
extern int sky_box_texture[6];

void sky_reset();

// sort.h

void sort_something(void *data, int record_size, int record_count);

// sound.h

#define SOUND_MAX_SERVER_SOUNDS 1024
#define SOUND_BUTTON_PRESS (SOUND_MAX_SERVER_SOUNDS + 0)
#define SOUND_CHAT (SOUND_MAX_SERVER_SOUNDS + 1)
#define SOUND_COLLISION (SOUND_MAX_SERVER_SOUNDS + 2)
#define SOUND_BOUNCE (SOUND_MAX_SERVER_SOUNDS + 3)
#define SOUND_THROW (SOUND_MAX_SERVER_SOUNDS + 4)
#define SOUND_BLOCK_PLACE (SOUND_MAX_SERVER_SOUNDS + 5)
#define SOUND_BLOCK_REMOVE (SOUND_MAX_SERVER_SOUNDS + 6)
#define SOUND_WALK (SOUND_MAX_SERVER_SOUNDS + 7)
#define SOUND_MAX_SOUNDS (SOUND_MAX_SERVER_SOUNDS + 8)

struct structure_sound_data {
  char *digest;          // SHA-1 digest of image file, only when awaiting download.
  char *file;            // File sound is loaded from, or NULL if no file.
  short *data;           // raw PCM data of sound
  int size;              // Size of memory in which sound is stored.
  int samples;           // number of samples in data
  int channels;          // number of audio channels
  int samplerate;        // sample rate
};

extern struct structure_sound_data sound_list[SOUND_MAX_SOUNDS];

void sound_reset ();
void sound_initialize();
void sound_terminate();
void sound_play (int sound_id, float volume, struct double_xyz *location);

// stars.h

void stars_open_window();
void stars_close_window();
void stars_render();

// statistics.h

extern char statistics_string[1024];
extern char statistics_report[1024];
extern double statistics_minimum_fog;
extern double statistics_fog_distance;
extern double statistics_maximum_fog;
extern int statistics_vsync_rate;
extern double statistics_current_fps;
extern double statistics_current_cps;
extern double statistics_peak_frame_time;
extern double statistics_current_fps_q;
extern double statistics_chunks_per_second;
extern double statistics_frames_per_second;
extern double statistics_chunk_percentage;

void statistics_count_frame();
void statistics_count_chunk();
void statistics_count_quads(int change);
void statistics_complete_chunks(int count, int total);
void statistics_open_window();

// texture.h

#define TEXTURE_FLAG_LINEAR 1
#define TEXTURE_FLAG_MIPMAP 2
#define TEXTURE_FLAG_NOFLIP 4
#define TEXTURE_FLAG_PIXELATE 8

#define TEXTURE_MAX_SERVER_TEXTURES 4096
#define TEXTURE_NO_ZERO (TEXTURE_MAX_SERVER_TEXTURES + 0)
#define TEXTURE_UNKNOWN (TEXTURE_MAX_SERVER_TEXTURES + 1)
#define TEXTURE_LOADING (TEXTURE_MAX_SERVER_TEXTURES + 2)
#define TEXTURE_INVALID (TEXTURE_MAX_SERVER_TEXTURES + 3)
#define TEXTURE_SWEET_U (TEXTURE_MAX_SERVER_TEXTURES + 4)
#define TEXTURE_SWEET_S (TEXTURE_MAX_SERVER_TEXTURES + 5)
#define TEXTURE_SWEET_D (TEXTURE_MAX_SERVER_TEXTURES + 6)
#define TEXTURE_MAP_EDGE (TEXTURE_MAX_SERVER_TEXTURES + 7)
#define TEXTURE_MAX_TEXTURES (TEXTURE_MAX_SERVER_TEXTURES + 8)

struct structure_texture_data {
  GLuint name;           // OpenGL texture "name"
  GLuint list;           // Display list number that loads texture.
  char *digest;          // SHA-1 digest of image file, only when awaiting download.
  char *file;            // File texture is loaded from, or NULL if no file.
  char *memory;          // Memory texture is loaded from, or NULL if no memory.
  int size;              // Size of memory in which image is stored.
  int flags;             // Flags for texture_load()
  double alpha;          // 0.0 = no alpha, 1.0 = alpha blend, between = alpha test
  double x_scale;        // How many times texture repeats per meter.
  double y_scale;        // How many times texture repeats per meter.
  double r, g, b, a;     // Average RGBA of texture.
};

extern int texture_list_base;
extern int texture_width;
extern int texture_height;
extern int texture_r;
extern int texture_g;
extern int texture_b;
extern int texture_a;
extern GLuint texture_full_screen;
extern GLuint texture_half_screen;
extern struct structure_texture_data texture_data[TEXTURE_MAX_TEXTURES];

void texture_list(int texture, char *file, int size, int flags);
GLuint texture_load(char *file, int size, int flags);
void texture_initialize();
void texture_open_window();
void texture_close_window();
void texture_reload();
void texture_reset();

// thread.h

#ifdef WINDOWS
  #define MUTEX HANDLE
  #define MUTEX_INIT(i) if (NULL == (i = CreateSemaphore(NULL, 1, 1, NULL))) { printf("GetLastError() = %d\n", GetLastError()); easy_fuck("Failed to create semaphore!"); };
  #define MUTEX_LOCK(i) if (WAIT_OBJECT_0 != WaitForSingleObject(i, INFINITE)) { printf("GetLastError() = %d\n", GetLastError()); easy_fuck("Failed to WaitForSingleObject!"); };
  #define MUTEX_UNLOCK(i) if (!ReleaseSemaphore(i, 1, NULL)) { printf("GetLastError() = %d\n", GetLastError()); easy_fuck("Failed to release semaphore!"); };
#else
  #define MUTEX pthread_mutex_t
  #define MUTEX_INIT(i) pthread_mutex_init(&i, NULL)
  #define MUTEX_LOCK(i) pthread_mutex_lock(&i)
  #define MUTEX_UNLOCK(i) pthread_mutex_unlock(&i)
#endif

struct structure_rwlock {
  MUTEX change;
  MUTEX in_use;
  int count;
};

void thread_lock_init(struct structure_rwlock *rwlock);
void thread_lock_read(struct structure_rwlock *rwlock);
void thread_unlock_read(struct structure_rwlock *rwlock);
void thread_lock_write(struct structure_rwlock *rwlock);
void thread_unlock_write(struct structure_rwlock *rwlock);
void thread_priority_background();
void thread_create(void (*function(void *)), void *parameter);
void thread_exit();
