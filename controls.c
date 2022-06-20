#include "everything.h"

static const int invalids[] = {
  GLFW_KEY_ESCAPE, GLFW_KEY_F1, GLFW_KEY_F3, GLFW_KEY_F4, GLFW_KEY_F10, GLFW_KEY_F11, GLFW_KEY_F12,
  GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9,
  GLFW_KEY_H, GLFW_KEY_F, GLFW_KEY_G,
  -1
};

static char *key_name[CONTROLS_MOUSE_KEY_LIMIT];

//--page-split-- controls_initialize

void controls_initialize() {
  key_name[' '] = "space";
  key_name[GLFW_KEY_APOSTROPHE] = "'\"";
  key_name[GLFW_KEY_COMMA] = ",<";
  key_name[GLFW_KEY_MINUS] = "-_";
  key_name[GLFW_KEY_PERIOD] = ".>";
  key_name[GLFW_KEY_SLASH] = "/?";
  key_name[GLFW_KEY_0] = "0";
  key_name[GLFW_KEY_1] = "1";
  key_name[GLFW_KEY_2] = "2";
  key_name[GLFW_KEY_3] = "3";
  key_name[GLFW_KEY_4] = "4";
  key_name[GLFW_KEY_5] = "5";
  key_name[GLFW_KEY_6] = "6";
  key_name[GLFW_KEY_7] = "7";
  key_name[GLFW_KEY_8] = "8";
  key_name[GLFW_KEY_9] = "9";
  key_name[GLFW_KEY_SEMICOLON] = ";:";
  key_name[GLFW_KEY_EQUAL] = "=+";
  key_name[GLFW_KEY_A] = "A";
  key_name[GLFW_KEY_B] = "B";
  key_name[GLFW_KEY_C] = "C";
  key_name[GLFW_KEY_D] = "D";
  key_name[GLFW_KEY_E] = "E";
  key_name[GLFW_KEY_F] = "F";
  key_name[GLFW_KEY_G] = "G";
  key_name[GLFW_KEY_H] = "H";
  key_name[GLFW_KEY_I] = "I";
  key_name[GLFW_KEY_J] = "J";
  key_name[GLFW_KEY_K] = "K";
  key_name[GLFW_KEY_L] = "L";
  key_name[GLFW_KEY_M] = "M";
  key_name[GLFW_KEY_N] = "N";
  key_name[GLFW_KEY_O] = "O";
  key_name[GLFW_KEY_P] = "P";
  key_name[GLFW_KEY_Q] = "Q";
  key_name[GLFW_KEY_R] = "R";
  key_name[GLFW_KEY_S] = "S";
  key_name[GLFW_KEY_T] = "T";
  key_name[GLFW_KEY_U] = "U";
  key_name[GLFW_KEY_V] = "V";
  key_name[GLFW_KEY_W] = "W";
  key_name[GLFW_KEY_X] = "X";
  key_name[GLFW_KEY_Y] = "Y";
  key_name[GLFW_KEY_Z] = "Z";
  key_name[GLFW_KEY_LEFT_BRACKET] = "[{";
  key_name[GLFW_KEY_BACKSLASH] = "\\|";
  key_name[GLFW_KEY_RIGHT_BRACKET] = "]}";
  key_name[GLFW_KEY_GRAVE_ACCENT] = "`~";
  key_name[GLFW_KEY_WORLD_1] = "world 1";
  key_name[GLFW_KEY_WORLD_2] = "world 2";
  key_name[GLFW_KEY_ESCAPE] = "escape";
  key_name[GLFW_KEY_ENTER] = "enter";
  key_name[GLFW_KEY_TAB] = "tab";
  key_name[GLFW_KEY_BACKSPACE] = "backspace";
  key_name[GLFW_KEY_INSERT] = "insert";
  key_name[GLFW_KEY_DELETE] = "delete";
  key_name[GLFW_KEY_RIGHT] = "right arrow";
  key_name[GLFW_KEY_LEFT] = "left arrow";
  key_name[GLFW_KEY_DOWN] = "down arrow";
  key_name[GLFW_KEY_UP] = "up arrow";
  key_name[GLFW_KEY_PAGE_UP] = "page up";
  key_name[GLFW_KEY_PAGE_DOWN] = "page down";
  key_name[GLFW_KEY_HOME] = "home";
  key_name[GLFW_KEY_END] = "end";
  key_name[GLFW_KEY_CAPS_LOCK] = "caps lock";
  key_name[GLFW_KEY_SCROLL_LOCK] = "scroll lock";
  key_name[GLFW_KEY_NUM_LOCK] = "number lock";
  key_name[GLFW_KEY_PRINT_SCREEN] = "print screen";
  key_name[GLFW_KEY_PAUSE] = "pause";
  key_name[GLFW_KEY_F1] = "F1";
  key_name[GLFW_KEY_F2] = "F2";
  key_name[GLFW_KEY_F3] = "F3";
  key_name[GLFW_KEY_F4] = "F4";
  key_name[GLFW_KEY_F5] = "F5";
  key_name[GLFW_KEY_F6] = "F6";
  key_name[GLFW_KEY_F7] = "F7";
  key_name[GLFW_KEY_F8] = "F8";
  key_name[GLFW_KEY_F9] = "F9";
  key_name[GLFW_KEY_F10] = "F10";
  key_name[GLFW_KEY_F11] = "F11";
  key_name[GLFW_KEY_F12] = "F12";
  key_name[GLFW_KEY_F13] = "F13";
  key_name[GLFW_KEY_F14] = "F14";
  key_name[GLFW_KEY_F15] = "F15";
  key_name[GLFW_KEY_F16] = "F16";
  key_name[GLFW_KEY_F17] = "F17";
  key_name[GLFW_KEY_F18] = "F18";
  key_name[GLFW_KEY_F19] = "F19";
  key_name[GLFW_KEY_F20] = "F20";
  key_name[GLFW_KEY_F21] = "F21";
  key_name[GLFW_KEY_F22] = "F22";
  key_name[GLFW_KEY_F23] = "F23";
  key_name[GLFW_KEY_F24] = "F24";
  key_name[GLFW_KEY_F25] = "F25";
  key_name[GLFW_KEY_KP_0] = "keypad 0";
  key_name[GLFW_KEY_KP_1] = "keypad 1";
  key_name[GLFW_KEY_KP_2] = "keypad 2";
  key_name[GLFW_KEY_KP_3] = "keypad 3";
  key_name[GLFW_KEY_KP_4] = "keypad 4";
  key_name[GLFW_KEY_KP_5] = "keypad 5";
  key_name[GLFW_KEY_KP_6] = "keypad 6";
  key_name[GLFW_KEY_KP_7] = "keypad 7";
  key_name[GLFW_KEY_KP_8] = "keypad 8";
  key_name[GLFW_KEY_KP_9] = "keypad 9";
  key_name[GLFW_KEY_KP_DECIMAL] = "keypad .";
  key_name[GLFW_KEY_KP_DIVIDE] = "keypad /";
  key_name[GLFW_KEY_KP_MULTIPLY] = "keypad *";
  key_name[GLFW_KEY_KP_SUBTRACT] = "keypad -";
  key_name[GLFW_KEY_KP_ADD] = "keypad +";
  key_name[GLFW_KEY_KP_ENTER] = "keypad enter";
  key_name[GLFW_KEY_KP_EQUAL] = "keypad =";
  key_name[GLFW_KEY_LEFT_SHIFT] = "left shift";
  key_name[GLFW_KEY_LEFT_CONTROL] = "left control";
  key_name[GLFW_KEY_LEFT_ALT] = "left alt";
  key_name[GLFW_KEY_LEFT_SUPER] = "left super";
  key_name[GLFW_KEY_RIGHT_SHIFT] = "right shift";
  key_name[GLFW_KEY_RIGHT_CONTROL] = "right control";
  key_name[GLFW_KEY_RIGHT_ALT] = "right alt";
  key_name[GLFW_KEY_RIGHT_SUPER] = "right super";
  key_name[GLFW_KEY_MENU] = "menu";
  key_name[CONTROLS_MOUSE_KEY_LEFT] = "left mouse";
  key_name[CONTROLS_MOUSE_KEY_RIGHT] = "right mouse";
  key_name[CONTROLS_MOUSE_KEY_MIDDLE] = "middle mouse";
  key_name[CONTROLS_MOUSE_KEY_BUTTON4] = "mouse 4";
  key_name[CONTROLS_MOUSE_KEY_BUTTON5] = "mouse 5";
  key_name[CONTROLS_MOUSE_KEY_BUTTON6] = "mouse 6";
  key_name[CONTROLS_MOUSE_KEY_BUTTON7] = "mouse 7";
  key_name[CONTROLS_MOUSE_KEY_BUTTON8] = "mouse 8";
};

//--page-split-- controls_key_is_invalid

int controls_key_is_invalid (int key) {
    int a;
    for (a = 0; ; a++) {
        if (invalids[a] == -1) break;
        if (key == invalids[a]) return (1);
    }
    return (0);
}

//--page-split-- controls_key_name

char *controls_key_name (int value) {
    if (value < 0 || value >= CONTROLS_MOUSE_KEY_LIMIT) return ((char *) "unknown key");
    if (key_name[value] == 0) return ((char *)"nothing");
    return (key_name[value]);
}

//--page-split-- controls_get_key

// glfwGetKey replacement
int controls_get_key(int key_id) {
    if (event_key_state[option_key_input[key_id][0]] || event_key_state[option_key_input[key_id][1]]) return (1);
    return (0);
}

//--page-split-- controls_menu_key

// KEY macro wrapper

int controls_menu_key (int key_id) {
    if (KEY == option_key_input[key_id][0] || KEY == option_key_input[key_id][1]) return (1);
    return (0);
}

//--page-split-- controls_menu_pressed

int controls_menu_pressed (int key_id) {
    if (KEY_PRESS_EVENT && (option_key_input[key_id][0] == KEY || option_key_input[key_id][1] == KEY)) return (1);
    return (0);
}

//--page-split-- controls_menu_released

int controls_menu_released (int key_id) {
    if (KEY_RELEASE_EVENT && (option_key_input[key_id][0] == KEY || option_key_input[key_id][1] == KEY)) return (1);
    return (0);
}
