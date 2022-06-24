#include "everything.h"

void (*menu_function_pointer)() = menus_server_connect;

static int menu_switch_occured;

int menu_object_data[MENU_MAX_OBJECTS][16];
int menu_object_index;
int menu_focus_object = 0;
int menu_next_focus;

int menu_process_event;
int menu_current_event;
int menu_draw_widget;

int menu_modifier_shift;
int menu_modifier_control;
int menu_modifier_alt;
static int left_shift = 0;
static int right_shift = 0;
static int left_control = 0;
static int right_control = 0;
static int left_alt = 0;
static int right_alt = 0;

int menu_display_data = 0;

//--page-split-- menu_switch

void menu_switch(void (*menu)()) {
  if (menu_function_pointer == menu) return;
//  #ifdef TEST
//  if (menu_function_pointer == menus_server_loading) on_activate_lag = 1;
//  #endif
  if (menu == menus_play && menu_function_pointer != menus_play) glfw_capture_mouse();
  if (menu != menus_play && menu_function_pointer == menus_play) {
    menus_play_focus_lost();
    glfw_release_mouse();
  };
  DEBUG("menu_switch()");
  menu_function_pointer = menu;
  menu_switch_occured = 1;
  menu_focus_object = -1;
};

//--page-split-- menu_process

void menu_process() {
  menu_process_event = 1;
  menu_draw_widget = 0;
  for (menu_current_event = 0; menu_current_event < event_list_index; menu_current_event++) {

    // Keep track of shift states here...
    if (KEY_PRESS_EVENT) {
      if (KEY == GLFW_KEY_LEFT_SHIFT) left_shift = 1;
      if (KEY == GLFW_KEY_RIGHT_SHIFT) right_shift = 1;
    } else if (KEY_RELEASE_EVENT) {
      if (KEY == GLFW_KEY_LEFT_SHIFT) left_shift = 0;
      if (KEY == GLFW_KEY_RIGHT_SHIFT) right_shift = 0;
    };
    menu_modifier_shift = left_shift | right_shift;
    if (KEY_PRESS_EVENT) {
      if (KEY == GLFW_KEY_LEFT_CONTROL) left_control = 1;
      if (KEY == GLFW_KEY_RIGHT_CONTROL) right_control = 1;
    } else if (KEY_RELEASE_EVENT) {
      if (KEY == GLFW_KEY_LEFT_CONTROL) left_control = 0;
      if (KEY == GLFW_KEY_RIGHT_CONTROL) right_control = 0;
    };
    menu_modifier_control = left_control | right_control;
    if (KEY_PRESS_EVENT) {
      if (KEY == GLFW_KEY_LEFT_ALT) left_alt = 1;
      if (KEY == GLFW_KEY_RIGHT_ALT) right_alt = 1;
    } else if (KEY_RELEASE_EVENT) {
      if (KEY == GLFW_KEY_LEFT_ALT) left_alt = 0;
      if (KEY == GLFW_KEY_RIGHT_ALT) right_alt = 0;
    };
    menu_modifier_alt = left_alt | right_alt;

    if (KEY_PRESS_EVENT && !menus_controls_disable_function_keys) {
      if (KEY == GLFW_KEY_F1) menu_switch(menus_help);
    };

    // Also check for ESC and Fn key presses here.

    //printf("focus=%d shift=%d type=%d state=%d data=%d mouse=(%d, %d)\n", menu_focus_object, menu_modifier_shift, event_list[menu_current_event][0], event_list[menu_current_event][1], event_list[menu_current_event][2], event_list[menu_current_event][3], event_list[menu_current_event][4]);

    // Reset window offset each frame...
    //gui_menu_x_offset = 0;
    //gui_menu_y_offset = 0;
    // This seems unnecessary...

    menu_switch_occured = 0;
    menu_object_index = -1;
    if (menu_focus_object < 0) menu_focus_object = 0;
    menu_next_focus = menu_focus_object;

    lag_push(1, "menu_function() in menu_process()");
    menu_function_pointer();
    lag_pop();

    if (menu_next_focus < 0) menu_next_focus = menu_object_index;
    if (menu_next_focus > menu_object_index) menu_next_focus = 0;
    menu_focus_object = menu_next_focus;

    if (menu_switch_occured) {
      memset(menu_object_data, 0, sizeof(menu_object_data));
      menu_focus_object = 0;
    };

  };
  event_list_index = 0;
};

//--page-split-- f3_display

static void f3_display() {
  #define LINES 20
  char line[LINES][64] = {};

  int l = 0;
  snprintf(line[l++], 63,
  "Location: (%0.1f, %0.1f, %0.1f), %0.0f\x15", player_view.x, player_view.y, player_view.z, player_view.u * 180 / M_PI);
  if (menus_play_last_click_valid) {
    struct int_xyz c = menus_play_last_click;
    snprintf(line[l++], 63, "Last click: (%d, %d, %d)", c.x, c.y, c.z);
  } else {
    snprintf(line[l++], 63, "Last click: [unavailable]");
  };
  //#ifdef TEST
    snprintf(line[l++], 63, "Ping: %0.0f ms (accuracy limited by FPS)", 1000 * server_ping_time);
    if (server_connect_time >= 0) {
      int t = on_frame_time - server_connect_time;
      int s = t % 60;
      int m = (t / 60) % 60;
      int h = t / 3600;
      snprintf(line[l++], 63, "Server Connect Time: %d:%02d:%02d", h, m, s);
    } else {
      snprintf(line[l++], 63, "Server Connect Time: [not connected]");
    };
    snprintf(line[l++], 63, "Input Buffer Size: %d bytes", server_input_size);
    snprintf(line[l++], 63, "Output Buffer Size: %d bytes", server_output_size);
  //#endif
  if (l > LINES) easy_fuck("Damn you.\n");

  for (int i = 0; i < LINES; i++) {
    if (line[i][0]) {
      gui_text(0, i, line[i]);
    };
  };

};

//--page-split-- fps_display

static void fps_display() {
  char buffer[64];
  int length;
  snprintf(buffer, 64, "%0.0f FPS", statistics_current_fps); buffer[63] = 0;
  length = strlen(buffer);
  gui_text(gui_text_columns - length, 0, buffer);
  snprintf(buffer, 64, "%0.2f Q", statistics_current_fps_q); buffer[63] = 0;
  length = strlen(buffer);
  gui_text(gui_text_columns - length, 1, buffer);
  if (map_initialization_flag) {
    static double chunk_building_time = 0;
    if (statistics_current_cps > 0.0) {
      chunk_building_time = on_frame_time;
    };
    if (chunk_building_time > on_frame_time - 3.0) {
      snprintf(buffer, 64, "%0.0f c/s", statistics_current_cps); buffer[63] = 0;
      length = strlen(buffer);
      gui_text(gui_text_columns - length, 2, buffer);
    };
    static double incomplete_time = 0;
    if (statistics_chunk_percentage < 100.0) {
      incomplete_time = on_frame_time;
    };
    if (incomplete_time > on_frame_time - 4.0) {
      snprintf(buffer, 64, "%0.0f%%", statistics_chunk_percentage); buffer[63] = 0;
      length = strlen(buffer);
      gui_text(gui_text_columns - length, 3, buffer);
    };
  };
};

//--page-split-- cheap_hud_display

static void cheap_hud_display() {
  int row = 0;
  if (option_fps_display) row = 4;
  if (player_fly) gui_text(gui_text_columns - 6, row++, "FLYING");
  if (player_noclip) gui_text(gui_text_columns - 6, row++, "NOCLIP");

  gui_draw_block (menus_play_create_type, gui_text_columns - 3, gui_text_lines - 2, 2, 2);
}

//--page-split-- menu_render

void menu_render() {
//  if (!glfwGetWindowParam(GLFW_OPENED)) return;

  GLuint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  glMatrixMode(GL_PROJECTION); glLoadIdentity();
  glMatrixMode(GL_MODELVIEW); glLoadIdentity();
  glTranslated(-1.0, +1.0, 0.0);
  glScaled(2.0 / viewport[2], -2.0 / viewport[3], 1.0);

  gui_text_columns = floor((display_window_width - 3) / 12.0);
  gui_text_lines = floor((display_window_height - 3) / 24.0);
  gui_menu_width = 12 * gui_text_columns;
  gui_menu_height = 24 * gui_text_lines;;
  gui_menu_x_offset = (display_window_width - gui_menu_width) / 2;
  gui_menu_y_offset = (display_window_height - gui_menu_height) / 2;

  menu_process_event = 0;
  menu_draw_widget = 1;
  menu_switch_occured = 0;
  menu_object_index = -1;

  if (option_hud_display && glfw_mouse_capture_flag && map_is_active() && menu_function_pointer != menus_server_loading) cheap_hud_display();

  if (menu_function_pointer != menus_chat) {
    glPushMatrix();
    glTranslatef(0, gui_menu_height - 24 * gui_text_lines - 8, 0);
    chat_render(1, 1, gui_text_columns - 2, gui_text_lines - 1, 0, 0);
    glPopMatrix();
  };

  if (option_f3_display) f3_display();
  if (option_fps_display) fps_display();

  lag_push(5, "menu_function() in menu_render()");
  menu_function_pointer();
  lag_pop();

  if (menu_switch_occured) {
    //easy_fuck("Menu switch occured during menu render.");
    memset(menu_object_data, 0, sizeof(menu_object_data));
    menu_focus_object = 0;
  };

};
