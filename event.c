#include "everything.h"

int event_disable_function_keys = 0;
int event_mouse_button_state = 0;
char event_key_state[530];

// See event.h for documentation.

int event_list[EVENT_LIST_SIZE][5];
int event_list_index;

double event_mouse_position_x;
double event_mouse_position_y;

//--page-split-- screenshot

static void screenshot() {
  #ifdef UNIX
  mkdir("screenshots", 0777);
  #else
  mkdir("screenshots");
  #endif
  char *buffer = NULL;
  char *outbuffer = NULL;
  int binsize = display_window_width * display_window_height * 4;

  memory_allocate(&buffer, binsize);
  memory_allocate(&outbuffer, binsize);
  glReadPixels (0, 0, display_window_width, display_window_height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

  int width = display_window_width * 4;
  for (int b = 0; b < display_window_height; b++) {
    memmove (&outbuffer[((display_window_height - 1) * width) - (b * width)], &buffer[b * width], width);
  }

  char filename[64];

  time_t t;
  time (&t);
  struct tm *e;
  e=localtime(&t);

  snprintf (filename, 64, "screenshots/%04i-%02i-%02i %02i.%02i.%02i.png", 1900 + e->tm_year, e->tm_mon + 1, e->tm_mday, e->tm_hour, e->tm_min, e->tm_sec);
  int ret = stbi_write_png(filename, display_window_width, display_window_height, 4, outbuffer, 0);
  memory_allocate(&buffer, 0);
  memory_allocate(&outbuffer, 0);

  char message[96];
  if (ret) {
    snprintf(message, 96, "Saved screeenshot as %s", filename);
  } else {
    strcpy (message, "Unable to save screenshot.");
  }
  chat_message(message);
}

//--page-split-- event_open_window

void event_open_window() {
  glfwGetCursorPos(glfw_window, &event_mouse_position_x, &event_mouse_position_y);
};

//--page-split-- event_receive

void event_receive() {
  DEBUG("enter event_receive()");

  // This "for" loop repeatedly calls glfwPollEvents(), and if it causes some
  // events to be written into the event list, then the contents of the loop
  // are executed, which remove those events and empty the list.  It doesn't
  // stop until a call to glfwPollEvents() fails to return any more events.

  lag_push(1, "glfwPollEvents()");
  glfwPollEvents();
  lag_pop();

  // Make an extra "mouse movement" event, to keep menu highlights
  // up to date when clicks and such cause the menus to change.

  if (event_list_index >= EVENT_LIST_SIZE) easy_fuck("Event list overflow.");
  event_list[event_list_index][0] = 4;
  event_list[event_list_index][1] = -1;
  event_list[event_list_index][2] = -1;
  event_list[event_list_index][3] = event_mouse_position_x;
  event_list[event_list_index][4] = event_mouse_position_y;
  event_list_index++;

  lag_push(1, "menu_process()");
  menu_process();
  lag_pop();

  DEBUG("leave event_receive()");
};

//--page-split-- event_move_mouse

void event_move_mouse(double x, double y) {
  event_mouse_position_x = x;
  event_mouse_position_y = y;
  glfwSetCursorPos(glfw_window, x, y);
};

//--page-split-- event_translate_from_unicode

int event_translate_from_unicode(int character) {
  int code = 0;
  if (character >= 32 && character <= 126) code = character;
  else if (character == 8226) code = 1;
  else if (character == 167) code = 2;
  else if (character == 182) code = 3;
  else if (character == 161) code = 4;
  else if (character == 191) code = 5;
  else if (character == 171) code = 6;
  else if (character == 187) code = 7;
  else if (character == 8249) code = 8;
  else if (character == 8250) code = 9;
  else if (character == 8216) code = 10;
  else if (character == 8217) code = 11;
  else if (character == 8218) code = 12;
  else if (character == 8220) code = 13;
  else if (character == 8221) code = 14;
  else if (character == 8222) code = 15;
  else if (character == 215) code = 16;
  else if (character == 247) code = 17;
  else if (character == 178) code = 18;
  else if (character == 179) code = 19;
  else if (character == 183) code = 20;
  else if (character == 176) code = 21;
  else if (character == 177) code = 22;
  else if (character == 188) code = 23;
  else if (character == 189) code = 24;
  else if (character == 190) code = 25;
  else if (character == 8364) code = 26;
  else if (character == 163) code = 28;
  else if (character == 165) code = 29;
  else if (character == 170) code = 30;
  else if (character == 186) code = 31;
  else if (character == 8230) code = 127;
  else if (character == 192) code = 128;
  else if (character == 224) code = 129;
  else if (character == 193) code = 130;
  else if (character == 225) code = 131;
  else if (character == 194) code = 132;
  else if (character == 226) code = 133;
  else if (character == 195) code = 134;
  else if (character == 227) code = 135;
  else if (character == 196) code = 136;
  else if (character == 228) code = 137;
  else if (character == 197) code = 138;
  else if (character == 229) code = 139;
  else if (character == 258) code = 140;
  else if (character == 259) code = 141;
  else if (character == 260) code = 142;
  else if (character == 261) code = 143;
  else if (character == 198) code = 144;
  else if (character == 230) code = 145;
  else if (character == 199) code = 146;
  else if (character == 231) code = 147;
  else if (character == 262) code = 148;
  else if (character == 263) code = 149;
  else if (character == 268) code = 150;
  else if (character == 269) code = 151;
  else if (character == 270) code = 152;
  else if (character == 271) code = 153;
  else if (character == 280) code = 154;
  else if (character == 281) code = 155;
  else if (character == 282) code = 156;
  else if (character == 283) code = 157;
  else if (character == 200) code = 158;
  else if (character == 232) code = 159;
  else if (character == 201) code = 160;
  else if (character == 233) code = 161;
  else if (character == 202) code = 162;
  else if (character == 234) code = 163;
  else if (character == 203) code = 164;
  else if (character == 235) code = 165;
  else if (character == 204) code = 166;
  else if (character == 236) code = 167;
  else if (character == 205) code = 168;
  else if (character == 237) code = 169;
  else if (character == 206) code = 170;
  else if (character == 238) code = 171;
  else if (character == 207) code = 172;
  else if (character == 239) code = 173;
  else if (character == 313) code = 174;
  else if (character == 314) code = 175;
  else if (character == 317) code = 176;
  else if (character == 318) code = 177;
  else if (character == 321) code = 178;
  else if (character == 322) code = 179;
  else if (character == 209) code = 180;
  else if (character == 241) code = 181;
  else if (character == 323) code = 182;
  else if (character == 324) code = 183;
  else if (character == 327) code = 184;
  else if (character == 328) code = 185;
  else if (character == 210) code = 186;
  else if (character == 242) code = 187;
  else if (character == 211) code = 188;
  else if (character == 243) code = 189;
  else if (character == 212) code = 190;
  else if (character == 244) code = 191;
  else if (character == 213) code = 192;
  else if (character == 245) code = 193;
  else if (character == 214) code = 194;
  else if (character == 246) code = 195;
  else if (character == 216) code = 196;
  else if (character == 248) code = 197;
  else if (character == 336) code = 198;
  else if (character == 337) code = 199;
  else if (character == 338) code = 200;
  else if (character == 339) code = 201;
  else if (character == 340) code = 202;
  else if (character == 341) code = 203;
  else if (character == 344) code = 204;
  else if (character == 345) code = 205;
  else if (character == 346) code = 206;
  else if (character == 347) code = 207;
  else if (character == 350) code = 208;
  else if (character == 351) code = 209;
  else if (character == 352) code = 210;
  else if (character == 353) code = 211;
  else if (character == 536) code = 212;
  else if (character == 537) code = 213;
  else if (character == 354) code = 214;
  else if (character == 355) code = 215;
  else if (character == 356) code = 216;
  else if (character == 357) code = 217;
  else if (character == 538) code = 218;
  else if (character == 539) code = 219;
  else if (character == 222) code = 220;
  else if (character == 254) code = 221;
  else if (character == 217) code = 222;
  else if (character == 249) code = 223;
  else if (character == 218) code = 224;
  else if (character == 250) code = 225;
  else if (character == 219) code = 226;
  else if (character == 251) code = 227;
  else if (character == 220) code = 228;
  else if (character == 252) code = 229;
  else if (character == 366) code = 230;
  else if (character == 367) code = 231;
  else if (character == 368) code = 232;
  else if (character == 369) code = 233;
  else if (character == 221) code = 234;
  else if (character == 253) code = 235;
  else if (character == 376) code = 236;
  else if (character == 255) code = 237;
  else if (character == 377) code = 238;
  else if (character == 378) code = 239;
  else if (character == 379) code = 240;
  else if (character == 380) code = 241;
  else if (character == 381) code = 242;
  else if (character == 382) code = 243;
  else if (character == 223) code = 244;
  else if (character == 208) code = 245;
  else if (character == 272) code = 245;
  else if (character == 240) code = 246;
  else if (character == 273) code = 247;
  else if (character == 286) code = 248;
  else if (character == 287) code = 249;
  else if (character == 304) code = 250;
  else if (character == 305) code = 251;
  else if (character == 330) code = 252;
  else if (character == 331) code = 253;
  else if (character == 181) code = 254;
  else if (character == 8486) code = 255;
  return code;
};

//--page-split-- event_keyboard_char_callback

void event_keyboard_char_callback(GLFWwindow *window, unsigned int character) {
  int code = event_translate_from_unicode(character);
  if (code) {
    if (event_list_index >= EVENT_LIST_SIZE) easy_fuck("Event list overflow.");
    event_list[event_list_index][0] = 0;
    event_list[event_list_index][1] = 1;
    event_list[event_list_index][2] = code;
    event_list[event_list_index][3] = event_mouse_position_x;
    event_list[event_list_index][4] = event_mouse_position_y;
    event_list_index++;
    //printf("Turned %d into %d...\n", character, code);
  } else {
    printf("Unmapped character %d received!\n", character);
  };
};

//--page-split-- event_keyboard_key_callback

void event_keyboard_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (event_list_index >= EVENT_LIST_SIZE) easy_fuck("Event list overflow.");
  event_list[event_list_index][0] = 1;
  event_list[event_list_index][1] = action;
  event_list[event_list_index][2] = key;
  event_list[event_list_index][3] = event_mouse_position_x;
  event_list[event_list_index][4] = event_mouse_position_y;
  event_list_index++;

  event_key_state[key] = action;
  // Monitor for keys that should change things regardless of active menu...

  // menus/controls.c uses this to allow the custom controls input thing
  // to tell the user that these function keys are 'reserved' without
  // toggling their functionality.  Should these be configurable, too?

  if (!event_disable_function_keys) {

    if (action == GLFW_PRESS && key == GLFW_KEY_F3) {
      option_f3_display = !option_f3_display;
    };

    if (action == GLFW_PRESS && key == GLFW_KEY_F4) {
      screenshot();
    }
    #ifdef TEST
    if (action == GLFW_PRESS && key == GLFW_KEY_F10 && argument_hacks) {
      menu_switch(menus_hacks);
    };
    #endif

    if (action == GLFW_PRESS && key == GLFW_KEY_F11) {
      static int fullscreen = 0;
      fullscreen = !fullscreen;
      glfw_fullscreen(fullscreen);
    };

    if (action == GLFW_PRESS && key == GLFW_KEY_F12) {
      static double last_use = -60;
      if (on_frame_time > last_use + 10.0) {
        char os_version[64];
        #ifdef WINDOWS
          OSVERSIONINFO osvi;
          BOOL bIsWindowsXPorLater;
          ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
          osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
          GetVersionEx(&osvi);
          int major = osvi.dwMajorVersion;
          int minor = osvi.dwMinorVersion;
          snprintf(os_version, 64, "Windows %d.%d", major, minor);
          os_version[4095] = 0;
        #else
          strcpy(os_version, build_target);
        #endif
        char string[4096];
        snprintf(string, 4096, "\e\x0B\x01\e\x09 MME version "
        "%s, %s, %d cores, \"%s\", %s, light=%d, textures=%d",
        build_svn_string, os_version, cpu_core_count, glGetString(GL_RENDERER),
        statistics_report, option_lighting, option_textures);
        string[4095] = 0;
        packet_send(PACKET_CHAT_MESSAGE, 0, string);
        last_use = on_frame_time;
      };
    };
  }

};

//--page-split-- event_mouse_button_callback

void event_mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {

  int state = (action == GLFW_PRESS);

  // Mice are keyboards now.

  if (event_list_index >= EVENT_LIST_SIZE) easy_fuck("Event list overflow.");
  event_list[event_list_index][0] = 1;
  event_list[event_list_index][1] = state;
  event_list[event_list_index][2] = button + CONTROLS_MOUSE_KEY;
  event_list[event_list_index][3] = event_mouse_position_x;
  event_list[event_list_index][4] = event_mouse_position_y;
  event_list_index++;
  event_key_state[button + 512] = state;

};

//--page-split-- event_mouse_scroll_callback

void event_mouse_scroll_callback(GLFWwindow *window, double x, double y) {
  if (event_list_index >= EVENT_LIST_SIZE) easy_fuck("Event list overflow.");
  event_list[event_list_index][0] = 3;
  event_list[event_list_index][1] = -1;
  event_list[event_list_index][2] = y;
  event_list[event_list_index][3] = event_mouse_position_x;
  event_list[event_list_index][4] = event_mouse_position_y;
  event_list_index++;
};

//--page-split-- event_mouse_position_callback

void event_mouse_position_callback(GLFWwindow *window, double x, double y) {
  if (event_list_index >= EVENT_LIST_SIZE) easy_fuck("Event list overflow.");
  event_list[event_list_index][0] = 4;
  event_list[event_list_index][1] = -1;
  event_list[event_list_index][2] = -1;
  event_list[event_list_index][3] = x;
  event_list[event_list_index][4] = y;
  event_list_index++;
  if (glfw_mouse_capture_flag) {
    player_mouse_x_accumulator += x - event_mouse_position_x;
    player_mouse_y_accumulator += y - event_mouse_position_y;
  };
  event_mouse_position_x = x;
  event_mouse_position_y = y;
};
