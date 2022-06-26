#include "everything.h"

#define CUBOID_TIME 0.25

int menus_play_box_enable = 1;
int menus_play_box_dimension_limit = 1;
int menus_play_box_volume_limit = 1;
int menus_play_create_type = 1;
int menus_play_destroy_type = 255;
int menus_play_last_click_valid = 0;
struct int_xyz menus_play_last_click;
static int replace_block = 0;
static int mode = 2;
static int step = 0;

//  Modes / Steps
//  2 = click & drag
//  3 = selection
//  4 = paste
//  6 = gun

static struct int_xyz clipboard_offset = {};
static struct int_xyz clipboard_size = {};
static char *clipboard_data = NULL;

static double instruction_hide_time = 0;
static double projectile_last_stamp = 0;

static struct int_xyz box_size = {};

static int saw_chat_menu_key_press[2] = {};
static int saw_server_menu_key_press[2] = {};

static double cuboid_start_time = 0;
static double message_no_fly = 0;
static double message_no_noclip = 0;
static double message_no_gun = 0;

//--page-split-- check_box_size

static int check_box_size(struct int_xyz one, struct int_xyz two) {

  #ifdef TEST
  //return 1; // Disables box size check!
  #endif

  // returns true if box size is below limits, otherwise returns
  // false and spawns a few chat messages to explain why

  int temp;
  if (one.x > two.x) { temp = one.x; one.x = two.x; two.x = temp; };
  if (one.y > two.y) { temp = one.y; one.y = two.y; two.y = temp; };
  if (one.z > two.z) { temp = one.z; one.z = two.z; two.z = temp; };
  if (map_data.wrap.x) {
    if (two.x - one.x >= map_data.dimension.x / 2) {
      two.x -= map_data.dimension.x;
      temp = one.x; one.x = two.x; two.x = temp;
    };
    if (two.x - one.x >= map_data.dimension.x / 4) return 0;
  };
  if (map_data.wrap.y) {
    if (two.y - one.y >= map_data.dimension.y / 2) {
      two.y -= map_data.dimension.y;
      temp = one.y; one.y = two.y; two.y = temp;
    };
    if (two.y - one.y >= map_data.dimension.y / 4) return 0;
  };
  if (map_data.wrap.z) {
    if (two.z - one.z >= map_data.dimension.z / 2) {
      two.z -= map_data.dimension.z;
      temp = one.z; one.z = two.z; two.z = temp;
    };
    if (two.z - one.z >= map_data.dimension.z / 4) return 0;
  };

  box_size.x = two.x - one.x + 1;
  box_size.y = two.y - one.y + 1;
  box_size.z = two.z - one.z + 1;
  int volume = box_size.x * box_size.y * box_size.z;

  if (volume > menus_play_box_volume_limit) return 0;
  if (box_size.x > menus_play_box_dimension_limit) return 0;
  if (box_size.y > menus_play_box_dimension_limit) return 0;
  if (box_size.z > menus_play_box_dimension_limit) return 0;

  return 1;

};

//--page-split-- menus_play_reset

void menus_play_reset() {
  map_selection_color = -1;
  menus_play_create_type = 1;
  mode = 2; step = 0;
  //#ifndef TEST
  clipboard_size = (struct int_xyz) {0, 0, 0};
  memory_allocate(&clipboard_data, 0);
  //#endif
  menus_play_last_click_valid = 0;
};

//--page-split-- menus_play_focus_lost

void menus_play_focus_lost() {
  if (mode == 2 && step == 1) {
    map_selection_color = -1;
    step = 0;
  };
  if (!option_noclip) player_noclip = 0;
  saw_chat_menu_key_press[0] = 0;
  saw_chat_menu_key_press[1] = 0;
  saw_server_menu_key_press[0] = 0;
  saw_server_menu_key_press[1] = 0;
};

//--page-split-- menus_play

#define building_is_allowed (menus_play_box_dimension_limit > 0 && menus_play_box_volume_limit > 0 && (packet_is_sendable(PACKET_MAP_MODIFY) || packet_is_sendable(PACKET_MAP_FILL)))
#define cuboid_is_allowed (menus_play_box_enable && menus_play_box_dimension_limit > 1 && menus_play_box_volume_limit > 1 && packet_is_sendable(PACKET_MAP_MODIFY))

void menus_play() {

  const int do_in_advance = 1;

  lag_push(1, "menus_play()");

  //if (option_hyper_help && map_is_active()) {
  //  menu_switch(menus_help);
  //  lag_pop();
  //  return;
  //};

  if (menus_server_menu_active) {
    menu_switch(menus_server_menu);
    lag_pop();
    return;
  };

  if (!glfwGetWindowAttrib(glfw_window, GLFW_FOCUSED)) {
    // release mouse on alt-tab and such
    if (glfw_mouse_capture_flag) glfw_release_mouse();
  };

  if (menu_process_event && menu_focus_object == 0) {
    if (CHARACTER_EVENT) {
      if (CHARACTER == '1') mode = 2, step = 0;
      if (CHARACTER == '2') mode = 2, step = 0;
      if (CHARACTER == '3') {
        if (mode == 3 && step == 1 && check_box_size(map_selection_corner_one, map_selection_corner_two)) {
          map_selection_color = 5;
          mode = 3, step = 2;
          instruction_hide_time = on_frame_time;
        } else {
          mode = 3, step = 0;
          instruction_hide_time = on_frame_time;
        };
      };
      if (CHARACTER == '4') {
        if (clipboard_data != NULL
          #ifndef TEST
            && check_box_size((struct int_xyz) {0, 0, 0}, clipboard_size)
          #endif
        ) {
          mode = 4, step = 0;
          instruction_hide_time = on_frame_time;
        } else {
          mode = 3, step = 0;
          instruction_hide_time = on_frame_time;
        };
      };
      #ifdef TEST
      if (CHARACTER == '5') mode = 5, step = 0;
      #endif
      if (CHARACTER == '6') {
        if (packet_is_sendable(PACKET_PROJECTILE_CREATE)) {
          mode = 6; step = 0;
        } else {
            message_no_gun = on_frame_time + 3;
        }
      }
    };
    if (KEY_PRESS_EVENT) {
      if (KEY == option_key_input[CONTROLS_KEY_CHAT][0]) saw_chat_menu_key_press[0] = 1;
      if (KEY == option_key_input[CONTROLS_KEY_CHAT][1]) saw_chat_menu_key_press[1] = 1;
      if (KEY == option_key_input[CONTROLS_KEY_SERVER_MENU][0]) saw_server_menu_key_press[0] = 1;
      if (KEY == option_key_input[CONTROLS_KEY_SERVER_MENU][1]) saw_server_menu_key_press[1] = 1;
      if (KEY == GLFW_KEY_F2) server_menu_request();
      if (controls_menu_key (CONTROLS_KEY_MODE_FLY)) {
        if (!player_allow_flying) {
          message_no_fly = on_frame_time + 3;
        } else {
          player_fly = !player_fly;
              if (option_noclip == 2) player_noclip = player_fly;
        };
      };

      if (KEY == GLFW_KEY_ESCAPE) {
        if (mode > 2 && mode != 6) {
          mode = 2, step = 0;
        } else {
          menu_switch(menus_escape);
        };
      };
      if (controls_menu_key (CONTROLS_KEY_MODE_NOCLIP)) {
        if (!player_allow_noclip) {
          message_no_noclip = on_frame_time + 3;
        } else {
          if (option_noclip == 0) {
            player_noclip = 1;
          } else if (option_noclip == 1) {
            player_noclip = !player_noclip;
          } else if (option_noclip == 2) {
            player_noclip = 1;
          };
        };
      };
      if (controls_menu_key (CONTROLS_KEY_BLOCK_MENU)) {
        menu_switch(menus_group);
        map_selection_color = -1;
        if (mode == 3 || mode == 4 || mode == 6) mode = 2;
        step = 0;
      };
    };
    if (KEY_RELEASE_EVENT) {
      if (KEY == option_key_input[CONTROLS_KEY_CHAT][0] && saw_chat_menu_key_press[0] == 1) menu_switch(menus_chat);
      if (KEY == option_key_input[CONTROLS_KEY_CHAT][1] && saw_chat_menu_key_press[1] == 1) menu_switch(menus_chat);
      if (KEY == option_key_input[CONTROLS_KEY_SERVER_MENU][0] && saw_server_menu_key_press[0] == 1) server_menu_request();
      if (KEY == option_key_input[CONTROLS_KEY_SERVER_MENU][1] && saw_server_menu_key_press[1] == 1) server_menu_request();

      if (controls_menu_key (CONTROLS_KEY_MODE_NOCLIP)) {
        if (option_noclip == 0) {
          player_noclip = 0;
        } else if (option_noclip == 1) {
        } else if (option_noclip == 2) {
          player_noclip = player_fly;
        };
      };
    };
    if (glfw_mouse_capture_flag) {
      int create_press = controls_menu_pressed (CONTROLS_KEY_BLOCK_CREATE);
      int destroy_press = controls_menu_pressed (CONTROLS_KEY_BLOCK_DESTROY);
      int create_release = controls_menu_released (CONTROLS_KEY_BLOCK_CREATE);
      int destroy_release = controls_menu_released (CONTROLS_KEY_BLOCK_DESTROY);
      //if (option_mouse_reverse) {
      //  create_press = RIGHT_PRESS;
      //  destroy_press = LEFT_PRESS;
      //  create_release = RIGHT_RELEASE;
      //  destroy_release = LEFT_RELEASE;
      //};
      if (create_press && map_selection.valid) {
        menus_play_last_click = map_selection.create;
        menus_play_last_click_valid = 1;
      };
      if (destroy_press && map_selection.valid) {
        menus_play_last_click = map_selection.destroy;
        menus_play_last_click_valid = 1;
      };
      int angle;
      if (player_position.u < -0.75 * M_PI || player_position.u > +0.75 * M_PI) {
        angle = 2;
      } else if (player_position.u < -0.25 * M_PI) {
        angle = 3;
      } else if (player_position.u < +0.25 * M_PI) {
        angle = 0;
      } else {
        angle = 1;
      };
      REDO_CLICK:
      if (mode == 1 || (mode == 2 && !cuboid_is_allowed)) {
        step = 0;
        map_selection_color = -1;
        map_cursor_color = 2;
        if (map_selection.valid) {
          if (controls_menu_pressed (CONTROLS_KEY_BLOCK_CLONE)) {
            int t = map_get_block_type(map_selection.destroy);
            if (t != 0) menus_play_create_type = t;
          };
          if (create_press) {
            sound_play(SOUND_BLOCK_PLACE, 0.1f, NULL);

            if (controls_get_key (CONTROLS_KEY_BLOCK_REPLACE)) {
              if (do_in_advance) map_modify(map_selection.destroy, menus_play_create_type, 0);
              server_modify(map_selection.destroy, menus_play_create_type);
              replace_block = 1;
            } else {
              if (do_in_advance) map_modify(map_selection.create, menus_play_create_type, 0);
              server_modify(map_selection.create, menus_play_create_type);
              replace_block = 0;
            }
          };
          if (destroy_press) {
            sound_play(SOUND_BLOCK_REMOVE, 0.1f, NULL);
            if (do_in_advance) map_modify(map_selection.destroy, map_get_block_type(map_selection.create), 0);
            server_modify(map_selection.destroy, map_get_block_type(map_selection.create));
          };
        };
      } else if (mode == 2) {
        map_cursor_color = 3;
        if (step == 0) { // No clicks have occured yet.
          map_selection_color = -1;
          if (map_selection.valid) {
            if (controls_menu_pressed (CONTROLS_KEY_BLOCK_CLONE)) {
              int t = map_get_block_type(map_selection.destroy);
              if (t != 0) menus_play_create_type = t;
            };
            if (create_press) {
              if (controls_get_key (CONTROLS_KEY_BLOCK_REPLACE)) {
                map_selection_corner_one = map_selection.destroy;
                map_selection_corner_two = map_selection.destroy;
                replace_block = 1;
              } else {
                map_selection_corner_one = map_selection.create;
                map_selection_corner_two = map_selection.create;
                replace_block = 0;
              }
              step = 1;
              cuboid_start_time = on_frame_time;
              map_selection_color = 4;
            };
            if (destroy_press) {
              menus_play_destroy_type = map_get_block_type(map_selection.create);
              map_selection_corner_one = map_selection.destroy;
              map_selection_corner_two = map_selection.destroy;
              step = 2;
              cuboid_start_time = on_frame_time;
              map_selection_color = 4;
            };
          };
        } else if (step == 1) { // drawing cuboid with create button
          if (controls_menu_pressed (CONTROLS_KEY_BLOCK_CLONE)) {
            if (map_selection.valid) {
              int t = map_get_block_type(map_selection.destroy);
              if (t != 0) menus_play_create_type = t;
            };
            map_selection_color = -1;
            step = 0;
          } else if (destroy_press) {
            map_selection_color = -1;
            step = 0;
          } else if (on_frame_time - cuboid_start_time < CUBOID_TIME) {
            map_selection_color = 4;
            if (create_release) {
              sound_play(SOUND_BLOCK_PLACE, 0.1f, NULL);
              if (do_in_advance) map_modify(map_selection_corner_one, menus_play_create_type, 0);
              server_modify(map_selection_corner_one, menus_play_create_type);
              map_selection_color = -1;
              step = 0;
            };
          } else if (map_selection.valid) {
            if (replace_block) map_selection_corner_two = map_selection.destroy;
            else map_selection_corner_two = map_selection.create;
            if (check_box_size(map_selection_corner_one, map_selection_corner_two)) {
              map_selection_color = 4;
              if (create_release) {
                sound_play(SOUND_BLOCK_PLACE, 0.1f, NULL);
                if (do_in_advance) map_modify_bunch(map_selection_corner_one, map_selection_corner_two, menus_play_create_type, 0);
                lag_push(1, "server_modify_bunch()");
                server_modify_bunch(map_selection_corner_one, map_selection_corner_two, menus_play_create_type);
                lag_pop();
                map_selection_color = -1;
                step = 0;
              };
            } else {
              map_selection_color = 2;
            };
          } else {
            map_selection_color = -1;
          };
          if (create_release) {
            map_selection_color = -1;
            step = 0;
          };
        } else if (step == 2) { // drawing cuboid with destroy button
          if (controls_menu_pressed (CONTROLS_KEY_BLOCK_CLONE)) {
            if (map_selection.valid) {
              int t = map_get_block_type(map_selection.destroy);
              if (t != 0) menus_play_create_type = t;
            };
            map_selection_color = -1;
            step = 0;
          } else if (create_press) {
            map_selection_color = -1;
            step = 0;
          } else if (on_frame_time - cuboid_start_time < CUBOID_TIME) {
            map_selection_color = 4;
            if (destroy_release) {
              sound_play(SOUND_BLOCK_REMOVE, 0.1f, NULL);
              if (do_in_advance) map_modify(map_selection_corner_one, menus_play_destroy_type, 0);
              server_modify(map_selection_corner_one, menus_play_destroy_type);
              map_selection_color = -1;
              step = 0;
            };
          } else if (map_selection.valid) {
            map_selection_corner_two = map_selection.destroy;
            if (check_box_size(map_selection_corner_one, map_selection_corner_two)) {
              map_selection_color = 4;
              if (destroy_release) {
                sound_play(SOUND_BLOCK_REMOVE, 0.1f, NULL);
                if (do_in_advance) map_modify_bunch(map_selection_corner_one, map_selection_corner_two, menus_play_destroy_type, 0);
                lag_push(1, "server_modify_bunch()");
                server_modify_bunch(map_selection_corner_one, map_selection_corner_two, menus_play_destroy_type);
                lag_pop();
                map_selection_color = -1;
                step = 0;
              };
            } else {
              map_selection_color = 2;
            };
          } else {
            map_selection_color = -1;
          };
          if (destroy_release) {
            map_selection_color = -1;
            step = 0;
          };
        };
      } else if (mode == 3) {
        map_cursor_color = 4;
        if (step == 0) { // no blocks have been selected yet
          map_selection_color = -1;
          if (controls_menu_pressed (CONTROLS_KEY_BLOCK_CLONE)) {
            if (map_selection.valid) {
              int t = map_get_block_type(map_selection.destroy);
              if (t != 0) menus_play_create_type = t;
            };
            map_selection_color = -1;
            mode = 2; step = 0;
          } else {
            map_selection_color = -1;
            if (map_selection.valid) {
              if (create_press) {
                map_selection_corner_one = map_selection.create;
                map_selection_corner_two = map_selection.create;
                map_selection_color = 4;
                step = 1;
              };
              if (destroy_press) {
                map_selection_corner_one = map_selection.destroy;
                map_selection_corner_two = map_selection.destroy;
                map_selection_color = 4;
                step = 1;
              };
            };
          };
        } else if (step == 1) { // some blocks have been selected
          if (controls_menu_pressed (CONTROLS_KEY_BLOCK_CLONE)) {
            if (map_selection.valid) {
              int t = map_get_block_type(map_selection.destroy);
              if (t != 0) menus_play_create_type = t;
            };
            map_selection_color = -1;
            mode = 2; step = 0;
          } else if (map_selection.valid) {
            if (create_press) {
              if (map_selection.create.x < map_selection_corner_one.x) map_selection_corner_one.x = map_selection.create.x;
              if (map_selection.create.y < map_selection_corner_one.y) map_selection_corner_one.y = map_selection.create.y;
              if (map_selection.create.z < map_selection_corner_one.z) map_selection_corner_one.z = map_selection.create.z;
              if (map_selection.create.x > map_selection_corner_two.x) map_selection_corner_two.x = map_selection.create.x;
              if (map_selection.create.y > map_selection_corner_two.y) map_selection_corner_two.y = map_selection.create.y;
              if (map_selection.create.z > map_selection_corner_two.z) map_selection_corner_two.z = map_selection.create.z;
            };
            if (destroy_press) {
              if (map_selection.destroy.x < map_selection_corner_one.x) map_selection_corner_one.x = map_selection.destroy.x;
              if (map_selection.destroy.y < map_selection_corner_one.y) map_selection_corner_one.y = map_selection.destroy.y;
              if (map_selection.destroy.z < map_selection_corner_one.z) map_selection_corner_one.z = map_selection.destroy.z;
              if (map_selection.destroy.x > map_selection_corner_two.x) map_selection_corner_two.x = map_selection.destroy.x;
              if (map_selection.destroy.y > map_selection_corner_two.y) map_selection_corner_two.y = map_selection.destroy.y;
              if (map_selection.destroy.z > map_selection_corner_two.z) map_selection_corner_two.z = map_selection.destroy.z;
            };
            if (check_box_size(map_selection_corner_one, map_selection_corner_two)) {
              map_selection_color = 4;
            } else {
              map_selection_color = 2;
              if (create_press || destroy_press) {
                instruction_hide_time = on_frame_time;
              };
            };
          };
        } else if (step == 2) { // selection is finalized, waiting for reference block
          if (controls_menu_pressed (CONTROLS_KEY_BLOCK_CLONE)) {
            if (map_selection.valid) {
              int t = map_get_block_type(map_selection.destroy);
              if (t != 0) menus_play_create_type = t;
            };
            map_selection_color = -1;
            mode = 2; step = 0;
          } else if ((create_press || destroy_press) && map_selection.valid) {
            struct int_xyz reference;
            if (create_press) {
              reference = map_selection.create;
            } else {
              reference = map_selection.destroy;
            };
            if (angle == 0) {
              clipboard_offset = (struct int_xyz) {
                map_selection_corner_one.x - reference.x,
                map_selection_corner_one.y - reference.y,
                map_selection_corner_one.z - reference.z,
              };
              clipboard_size = (struct int_xyz) {
                map_selection_corner_two.x - map_selection_corner_one.x + 1,
                map_selection_corner_two.y - map_selection_corner_one.y + 1,
                map_selection_corner_two.z - map_selection_corner_one.z + 1,
              };
            } else if (angle == 1) {
              clipboard_offset = (struct int_xyz) {
                map_selection_corner_one.y - reference.y,
                reference.x - map_selection_corner_two.x,
                map_selection_corner_one.z - reference.z,
              };
              clipboard_size = (struct int_xyz) {
                map_selection_corner_two.y - map_selection_corner_one.y + 1,
                map_selection_corner_two.x - map_selection_corner_one.x + 1,
                map_selection_corner_two.z - map_selection_corner_one.z + 1,
              };
            } else if (angle == 2) {
              clipboard_offset = (struct int_xyz) {
                reference.x - map_selection_corner_two.x,
                reference.y - map_selection_corner_two.y,
                map_selection_corner_one.z - reference.z,
              };
              clipboard_size = (struct int_xyz) {
                map_selection_corner_two.x - map_selection_corner_one.x + 1,
                map_selection_corner_two.y - map_selection_corner_one.y + 1,
                map_selection_corner_two.z - map_selection_corner_one.z + 1,
              };
            } else {
              clipboard_offset = (struct int_xyz) {
                reference.y - map_selection_corner_two.y,
                map_selection_corner_one.x - reference.x,
                map_selection_corner_one.z - reference.z,
              };
              clipboard_size = (struct int_xyz) {
                map_selection_corner_two.y - map_selection_corner_one.y + 1,
                map_selection_corner_two.x - map_selection_corner_one.x + 1,
                map_selection_corner_two.z - map_selection_corner_one.z + 1,
              };
            };
            int size = (map_selection_corner_two.x - map_selection_corner_one.x + 1) * (map_selection_corner_two.y - map_selection_corner_one.y + 1) * (map_selection_corner_two.z - map_selection_corner_one.z + 1);
            memory_allocate(&clipboard_data, size);
            char *pointer = clipboard_data;
            for (int z = 0; z < clipboard_size.z; z++) {
              for (int y = 0; y < clipboard_size.y; y++) {
                for (int x = 0; x < clipboard_size.x; x++) {
                  struct int_xyz map_coord;
                  if (angle == 0) {
                    map_coord.x = reference.x + (clipboard_offset.x + x);
                    map_coord.y = reference.y + (clipboard_offset.y + y);
                    map_coord.z = reference.z + clipboard_offset.z + z;
                  } else if (angle == 1) {
                    map_coord.x = reference.x - (clipboard_offset.y + y);
                    map_coord.y = reference.y + (clipboard_offset.x + x);
                    map_coord.z = reference.z + clipboard_offset.z + z;
                  } else if (angle == 2) {
                    map_coord.x = reference.x - (clipboard_offset.x + x);
                    map_coord.y = reference.y - (clipboard_offset.y + y);
                    map_coord.z = reference.z + clipboard_offset.z + z;
                  } else {
                    map_coord.x = reference.x + (clipboard_offset.y + y);
                    map_coord.y = reference.y - (clipboard_offset.x + x);
                    map_coord.z = reference.z + clipboard_offset.z + z;
                  };
                  *pointer++ = map_get_block_type(map_coord);
                };
              };
            };
            map_selection_color = -1;
            mode = 4; step = 0;
            instruction_hide_time = on_frame_time;
          };
        };
      } else if (mode == 4) {
        map_selection_color = -1;
        map_cursor_color = 5;
        if (clipboard_data == NULL || clipboard_size.x == 0 || clipboard_size.y == 0 || clipboard_size.z == 0) {
          mode = 2; step = 0;
        } else if (controls_menu_pressed (CONTROLS_KEY_BLOCK_CLONE)) {
          if (map_selection.valid) {
            int t = map_get_block_type(map_selection.destroy);
            if (t != 0) menus_play_create_type = t;
          };
          map_selection_color = -1;
          mode = 2; step = 0;
        } else if (destroy_press) {
          map_selection_color = -1;
          mode = 2; step = 0;
          goto REDO_CLICK;
        } else {
          if (create_press && map_selection.valid) {
            struct int_xyz reference;
            if (create_press) {
              reference = map_selection.create;
            } else {
              reference = map_selection.destroy;
            };
            char *pointer = clipboard_data;
            char *paste_data = NULL;
            memory_allocate(&paste_data, clipboard_size.x * clipboard_size.y * clipboard_size.z);
            memset(paste_data, 0, clipboard_size.x * clipboard_size.y * clipboard_size.z);
            struct int_xyz min_coord;
            struct int_xyz dimension;
            min_coord.z = reference.z + clipboard_offset.z;
            dimension.z = clipboard_size.z;
            if (angle == 0) {
              min_coord.x = reference.x + clipboard_offset.x;
              min_coord.y = reference.y + clipboard_offset.y;
              dimension.x = clipboard_size.x;
              dimension.y = clipboard_size.y;
            } else if (angle == 1) {
              min_coord.x = reference.x - clipboard_offset.y - clipboard_size.y + 1;
              min_coord.y = reference.y + clipboard_offset.x;
              dimension.x = clipboard_size.y;
              dimension.y = clipboard_size.x;
            } else if (angle == 2) {
              min_coord.x = reference.x - clipboard_offset.x - clipboard_size.x + 1;
              min_coord.y = reference.y - clipboard_offset.y - clipboard_size.y + 1;
              dimension.x = clipboard_size.x;
              dimension.y = clipboard_size.y;
            } else {
              min_coord.x = reference.x + clipboard_offset.y;
              min_coord.y = reference.y - clipboard_offset.x - clipboard_size.x + 1;
              dimension.x = clipboard_size.y;
              dimension.y = clipboard_size.x;
            };
            for (int z = 0; z < clipboard_size.z; z++) {
              for (int y = 0; y < clipboard_size.y; y++) {
                for (int x = 0; x < clipboard_size.x; x++) {
                  int block = *pointer++;
                  struct int_xyz map_coord;
                  map_coord.z = reference.z + clipboard_offset.z + z;
                  if (angle == 0) {
                    map_coord.x = reference.x + (clipboard_offset.x + x);
                    map_coord.y = reference.y + (clipboard_offset.y + y);
                  } else if (angle == 1) {
                    map_coord.x = reference.x - (clipboard_offset.y + y);
                    map_coord.y = reference.y + (clipboard_offset.x + x);
                  } else if (angle == 2) {
                    map_coord.x = reference.x - (clipboard_offset.x + x);
                    map_coord.y = reference.y - (clipboard_offset.y + y);
                  } else {
                    map_coord.x = reference.x + (clipboard_offset.y + y);
                    map_coord.y = reference.y - (clipboard_offset.x + x);
                  };
                  if (map_coord.x < 0 || map_coord.x >= map_data.dimension.x) continue;
                  if (map_coord.y < 0 || map_coord.y >= map_data.dimension.y) continue;
                  if (map_coord.z < 0 || map_coord.z >= map_data.dimension.z) continue;
                  int dx = map_coord.x - min_coord.x;
                  int dy = map_coord.y - min_coord.y;
                  int dz = map_coord.z - min_coord.z;
                  int o = (dz * dimension.y + dy) * dimension.x + dx;
                  //printf("x=%d y=%d z=%d dx=%d dy=%d dz=%d o=%d dimensions %d %d %d\n", x, y, z, dx, dy, dz, o, dimension.x, dimension.y, dimension.z);
                  //if (o < 0 || o >= clipboard_size.x * clipboard_size.y * clipboard_size.z) {
                  //  easy_fuck("o is out of range");
                  //};
                  if (map_get_block_type(map_coord) != block) {
                    paste_data[o] = block;
                    //if (do_in_advance) map_modify(map_coord, block, 0);
                    //server_modify(map_coord, block);
                    //printf("Building type %d at (%d, %d, %d)\n", block, map_coord.x, map_coord.y, map_coord.z);
                  };
                };
              };
            };
            if (server_paste(min_coord, dimension, paste_data)) {
              sound_play(SOUND_BLOCK_PLACE, 0.1f, NULL);
              for (int z = 0; z < dimension.z; z++) {
                for (int y = 0; y < dimension.y; y++) {
                  for (int x = 0; x < dimension.x; x++) {
                    struct int_xyz map_coord;
                    map_coord.x = min_coord.x + x;
                    map_coord.y = min_coord.y + y;
                    map_coord.z = min_coord.z + z;
                    int o = (z * dimension.y + y) * dimension.x + x;
                    if (paste_data[o] != 0) {
                      if (do_in_advance) map_modify(map_coord, paste_data[o], 0);
                    };
                  };
                };
              };
            };
            memory_allocate(&paste_data, 0);
            if (!option_multiple_paste) {
              map_selection_color = -1;
              mode = 2; step = 0;
            };
          };
        };
      } else if (mode == 5) {
        if (controls_menu_pressed (CONTROLS_KEY_BLOCK_CLONE)) {
          if (map_selection.valid) {
            int t = map_get_block_type(map_selection.destroy);
            if (t != 0) menus_play_create_type = t;
          };
        } else if (destroy_press) {
          map_selection_color = -1;
          mode = 2; step = 0;
          goto REDO_CLICK;
        } else if (create_press && map_selection.valid) {
          if (menus_play_create_type != map_get_block_type(map_selection.destroy)) {
            paint_target_type = map_get_block_type(map_selection.destroy);
            paint_with_type = menus_play_create_type;
            if (do_in_advance) map_modify(map_selection.destroy, menus_play_create_type, 0);
            server_modify(map_selection.destroy, menus_play_create_type);
            (*paint_list)[0] = map_selection.destroy;
            paint_list_size = 1;
            paint_start_time = on_frame_time;
            paint_total = 0;
          };
        };
      } else if (mode == 6) {
        map_cursor_color = -1;
        #if 0
          static double last_fire_time = 0;
          static int autofire = -1;
          if (create_press) autofire = 0;
          if (destroy_press) autofire = 1;
          if (create_release || destroy_release) autofire = -1;
          if (autofire >= 0) {
            if (last_fire_time < on_frame_time - 0.1) {
              projectile_create(0, autofire, player_position);
              last_fire_time = on_frame_time;
            };
          };
        #else
        if (create_press || destroy_press) {
          if (on_frame_time >= projectile_last_stamp) {
            projectile_last_stamp = on_frame_time + 0.05;
            int type = 0;
            if (create_press) type = 0;
            else if (destroy_press) type = 1;
            projectile_create(0, type, player_position);
          }
        }
        #endif
      };
      if (!building_is_allowed) map_cursor_color = -1;
    } else {
      if (LEFT_PRESS) glfw_capture_mouse();
    };
  };

  menu_object_index++; // Let the game rendering be the first object...
  if (!menu_process_event) {

//    glPushMatrix();
//    glTranslatef(0, menu_window_height - 24 * gui_text_lines - 8, 0);
//    chat_render(1, 1, gui_text_columns - 2, gui_text_lines - 1, 0);
//    glPopMatrix();

    if (option_show_f1_help && map_is_active() && !option_f3_display) {
      //if (option_hyper_help) {
      //  chat_color(11);
      //} else {
      //  chat_color(13);
      //};
      gui_text(0, 0, "\e\x0B" "Press F1 for help!");
    };

    // Copy & Paste Instructions

    char *instruction = NULL;
    char string[256];
    {
      if (mode == 3 && (step == 0 | step == 1)) {
        if (step == 0 || check_box_size(map_selection_corner_one, map_selection_corner_two)) {
          instruction = "Click blocks to define a selection.  When finished, press 3 again.";
        } else {
          instruction = "Selection is too large.  Press 3 to start over or press 2 to cancel.";
        };
      };
      if (mode == 3 && step == 2) {
        instruction = "Click the block you want to become the block you build when pasting.";
      };
      if (mode == 4 && (option_key_input[CONTROLS_KEY_BLOCK_CREATE][0] != 0 || option_key_input[CONTROLS_KEY_BLOCK_CREATE][1] != 0)) {
//        char *click = NULL;
        char *repeat = NULL;
//        if (option_mouse_reverse) {
//          click = "Right";
//        } else {
//          click = "Left";
//        };
        if (option_multiple_paste) {
          repeat = "2 to return to building normally";
        } else {
          repeat = "4 to paste the same thing again";
        };
        char keys[64] = {0};
        int a = option_key_input[CONTROLS_KEY_BLOCK_CREATE][0] != 0;
        int b = option_key_input[CONTROLS_KEY_BLOCK_CREATE][1] != 0;
        if (a && b) {
          snprintf(keys, 64, "%s or %s", controls_key_name(option_key_input[CONTROLS_KEY_BLOCK_CREATE][0]), controls_key_name(option_key_input[CONTROLS_KEY_BLOCK_CREATE][1]));
        } else if (a) {
          snprintf(keys, 64, "%s", controls_key_name(option_key_input[CONTROLS_KEY_BLOCK_CREATE][0]));
        } else if (b) {
          snprintf(keys, 64, "%s", controls_key_name(option_key_input[CONTROLS_KEY_BLOCK_CREATE][1]));
        } else {
          snprintf(keys, 64, "none");
        };

        snprintf(string, 256, "Use %s to paste.  Then press %s.", keys, repeat);
        instruction = string;
      };
      if (instruction && on_frame_time > instruction_hide_time + 0.25) {
        int length = strlen(instruction);
        int offset = (gui_text_columns - length) / 2;
        double alpha = 0.8;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        chat_color(14);
        glBegin(GL_QUADS);
        glVertex2f(12 * (offset - 1) + gui_menu_x_offset, 38 + gui_menu_y_offset);
        glVertex2f(12 * (offset + length + 1) + gui_menu_x_offset, 38 + gui_menu_y_offset);
        glVertex2f(12 * (offset + length + 1) + gui_menu_x_offset, 110 + gui_menu_y_offset);
        glVertex2f(12 * (offset - 1) + gui_menu_x_offset, 110 + gui_menu_y_offset);
        glEnd();
        glDisable(GL_BLEND);
        gui_text(offset, 2, instruction);
        instruction = "...or press 2 to cancel and return to normal building.";
        length = strlen(instruction);
        offset = (gui_text_columns - length) / 2;
        gui_text(offset, 3, instruction);
      };
    };

    if (map_selection_color >= 0 && instruction == NULL) {
      check_box_size(map_selection_corner_one, map_selection_corner_two);
      if (box_size.x > 1 || box_size.y > 1 || box_size.z > 1) {

        char block[64];
        sprintf(block, "Size: %d \x10 %d \x10 %d", box_size.x, box_size.y, box_size.z);

        struct double_xyz meters;
        struct int_xyz feet;
        struct int_xyz inches;
        meters.x = box_size.x * map_data.resolution.x;
        meters.y = box_size.y * map_data.resolution.y;
        meters.z = box_size.z * map_data.resolution.z;
        char meter[64];
        sprintf(meter, "%0.2f \x10 %0.2f \x10 %0.2f", meters.x, meters.y, meters.z);
        inches.x = round(12 * (meters.x / 0.3)); // Set so that 4" == 10 cm
        inches.y = round(12 * (meters.y / 0.3)); // Set so that 4" == 10 cm
        inches.z = round(12 * (meters.z / 0.3)); // Set so that 4" == 10 cm
        char inch[64];
        sprintf(inch, "%d\" \x10 %d\" \x10 %d\"", inches.x, inches.y, inches.z);
        feet.x = floor(inches.x / 12);
        feet.y = floor(inches.y / 12);
        feet.z = floor(inches.z / 12);
        inches.x -= 12 * feet.x;
        inches.y -= 12 * feet.y;
        inches.z -= 12 * feet.z;
        char foot[64];
        sprintf(foot, "%d'%d\" \x10 %d'%d\" \x10 %d'%d\"", feet.x, inches.x, feet.y, inches.y, feet.z, inches.z);

        int length, longest = 0;
        length = strlen(block); if (longest < length) longest = length;
        length = strlen(meter); if (longest < length) longest = length;
        length = strlen(foot); if (longest < length) longest = length;
        length = strlen(inch); if (longest < length) longest = length;

        double alpha = 0.8;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0, 0.0, 1.0, alpha);
        glBegin(GL_QUADS);
        glVertex2f(gui_menu_x_offset + 36 + 12 * longest + 24, gui_menu_y_offset + 36);
        glVertex2f(gui_menu_x_offset + 36, gui_menu_y_offset + 36);
        glVertex2f(gui_menu_x_offset + 36, gui_menu_y_offset + 156);
        glVertex2f(gui_menu_x_offset + 36 + 12 * longest + 24, gui_menu_y_offset + 156);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(1.0, 1.0, 1.0);
        gui_draw_text(4, 2, block, NULL, 0);
        gui_draw_text(4, 3, meter, NULL, 0);
        gui_draw_text(4, 4, inch, NULL, 0);
        gui_draw_text(4, 5, foot, NULL, 0);

      };
    };

    if (message_no_fly > on_frame_time || message_no_noclip > on_frame_time || message_no_gun > on_frame_time) {
      char *instruction;
      if (message_no_fly > message_no_noclip) {
        instruction = "This server does not allow flying.";
      } else if (message_no_noclip > message_no_gun) {
        instruction = "This server does not allow noclip.";
      } else {
        instruction = "This server does not allow guns.";
      };
      int length = strlen(instruction);
      int offset = (gui_text_columns - length) / 2;
      int y = gui_text_lines - 3;
      int y1 = y * 24 - 12;
      int y2 = y * 24 + 36;
      double alpha = 0.8;
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glColor4f(0.0, 0.0, 0.0, 0.8);
      glBegin(GL_QUADS);
      glVertex2f(12 * (offset - 1) + gui_menu_x_offset, y1 + gui_menu_y_offset);
      glVertex2f(12 * (offset + length + 1) + gui_menu_x_offset, y1 + gui_menu_y_offset);
      glVertex2f(12 * (offset + length + 1) + gui_menu_x_offset, y2 + gui_menu_y_offset);
      glVertex2f(12 * (offset - 1) + gui_menu_x_offset, y2 + gui_menu_y_offset);
      glEnd();
      glDisable(GL_BLEND);
      gui_text(offset, y, instruction);
    };

    if (glfw_mouse_capture_flag) {
      glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
      glEnable(GL_LINE_SMOOTH);
      glEnable(GL_BLEND);
      for (int eye = (option_anaglyph_enable ? 1 : 0); eye < (option_anaglyph_enable ? 3 : 1); eye++) {
        int r_mask = 1, g_mask = 1, b_mask = 1;
        if (eye) {
          r_mask = ((int []) {1, 1, 0, 0, 0, 1})[option_anaglyph_filter[eye - 1]];
          g_mask = ((int []) {0, 1, 1, 1, 0, 0})[option_anaglyph_filter[eye - 1]];
          b_mask = ((int []) {0, 0, 0, 1, 1, 1})[option_anaglyph_filter[eye - 1]];
          double scale = 0.01;
          if (option_anaglyph_units) scale /= 2.54;
          double pupil_offset_meters = 0.5 * scale * option_pupil_distance;
          double pupil_offset_pixels = pupil_offset_meters / (scale * option_anaglyph_pixel_size);
          double offset = pupil_offset_pixels * (scale * option_anaglyph_distance) / map_crosshair_distance;
          //printf("pom = %f, cdm = %f, sdm = %f, pop = %f, offset = %f\n", pupil_offset_meters, map_crosshair_distance, option_anaglyph_distance * scale, pupil_offset_pixels, offset);
          glPushMatrix();
          if (eye == 1) glTranslated(-pupil_offset_pixels + offset, 0, 0);
          if (eye == 2) glTranslated(+pupil_offset_pixels - offset, 0, 0);
        };
        glColorMask(r_mask ? GL_TRUE : GL_FALSE, g_mask ? GL_TRUE : GL_FALSE, b_mask ? GL_TRUE : GL_FALSE, GL_TRUE);
        if (building_is_allowed) {
          if (mode == 6) {
            if (on_frame_time - projectile_last_stamp < 0.001) {
              chat_color(3);
            } else {
              chat_color(15);
            };
          } else {
            glColor3f(1.0, 1.0, 1.0);
          };
          glLineWidth(1.5);
          glBegin(GL_LINES);
          glVertex2f(display_window_width / 2.0 - 12, display_window_height / 2.0);
          glVertex2f(display_window_width / 2.0 + 12, display_window_height / 2.0);
          glVertex2f(display_window_width / 2.0, display_window_height / 2.0 - 12);
          glVertex2f(display_window_width / 2.0, display_window_height / 2.0 + 12);
          glEnd();
          if (mode == 4) {
            glPushMatrix();
            chat_color(11);
            glTranslated(display_window_width / 2.0 - 30 - gui_menu_x_offset, display_window_height / 2.0 - 48 - gui_menu_y_offset, 0);
            gui_draw_text(0, 0, "PASTE", NULL, FLAGS);
            glTranslated(6, 0, 0);
            gui_draw_text(0, 3, "MODE", NULL, FLAGS);
            glPopMatrix();
          };
        } else {
          glColor3f(1.0, 0.2, 0.1);
          glLineWidth(1.5);
          glBegin(GL_LINES);
          glVertex2f(display_window_width / 2.0 - 12, display_window_height / 2.0 - 12);
          glVertex2f(display_window_width / 2.0 + 12, display_window_height / 2.0 + 12);
          glVertex2f(display_window_width / 2.0 - 12, display_window_height / 2.0 + 12);
          glVertex2f(display_window_width / 2.0 + 12, display_window_height / 2.0 - 12);
          glEnd();
        };
        if (eye) glPopMatrix();
      };
      glDisable(GL_BLEND);
    };

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // The "server may no longer be connected" warning...

    double lag = easy_time() - network_last_response;

    double alpha = (lag - 15) / 75.0;
    if (alpha > 1.0) alpha = 1.0;

    if (alpha > 0.0) {

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glColor4f(1.0, 0.0, 0.0, alpha);
      glBegin(GL_QUADS);
      glVertex2f(gui_menu_x_offset + 420, gui_menu_y_offset + 36);
      glVertex2f(gui_menu_x_offset + 36, gui_menu_y_offset + 36);
      glVertex2f(gui_menu_x_offset + 36, gui_menu_y_offset + 132);
      glVertex2f(gui_menu_x_offset + 420, gui_menu_y_offset + 132);
      glEnd();

      char buffer[64];
      glColor4f(1.0, 1.0, 1.0, alpha);
      gui_draw_text(4, 2, "Warning:  You may no longer be", NULL, 0);
      gui_draw_text(4, 3, "connected to the server.  Last", NULL, 0);
      sprintf(buffer, "data received %0.0f seconds ago.", floor(lag));
      gui_draw_text(4, 4, buffer, NULL, 0);

      glDisable(GL_BLEND);

    };

  };

  lag_pop();
};
