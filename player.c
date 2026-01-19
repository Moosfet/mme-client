#include "everything.h"

int player_allow_flying = 0;
int player_allow_speedy = 0;
int player_allow_noclip = 0;

struct double_xyzuv player_view = {};
struct double_xyzuv player_position = {};
struct double_xyzuv player_velocity = {};
int player_fly = 0;
int player_noclip = 0;
int player_stuck = 0;

double player_mouse_x_accumulator = 0;
double player_mouse_y_accumulator = 0;

static int falling = 0;
//static double viscosity;

#define GRAVITY_SCALE 1.0
#define SPEED_SCALE (option_superhuman ? 2.0 : 1.0)

// Values in meters:
#define PLAYER_RADIUS_METERS 0.21
#define STAIRS_HEIGHT_METERS (option_superhuman ? 0.55 : 0.45) // 0.45 at least.

// Values in meters/second:
#define walking_speed 1.38
#define running_speed 4.27

#define fractions 1000

//--page-split-- player_reset

void player_reset() {
  player_position.x = map_data.dimension.x / 2.0;
  player_position.y = map_data.dimension.y / 2.0;
  player_position.z = map_data.dimension.z - 0.001;
  player_velocity.x = 0;
  player_velocity.y = 0;
  player_velocity.z = 0;
  player_fly = 0;
  player_noclip = 0;
  falling = 0;
};

//--page-split-- player_teleport

void player_teleport(struct double_xyzuv position) {
  player_position = position;
  player_velocity.x = 0;
  player_velocity.y = 0;
  player_velocity.z = 0;
};

// Distance traveled to make a footstep sound (in meters)
#define STRIDE_WALK 0.75 // lots of conflicting numbers on the internet
#define STRIDE_RUN  1.5  // just going with what sounds right

//--page-split-- viscosity_test

static double viscosity_test(double px, double py, double pz) {

  struct int_xyz one, two;

  one.x = floor(px - PLAYER_RADIUS_METERS / map_data.resolution.x);
  two.x = floor(px + PLAYER_RADIUS_METERS / map_data.resolution.x);
  one.y = floor(py - PLAYER_RADIUS_METERS / map_data.resolution.y);
  two.y = floor(py + PLAYER_RADIUS_METERS / map_data.resolution.y);

  one.z = floor(pz - (CAMERA_HEIGHT_METERS - STAIRS_HEIGHT_METERS) / map_data.resolution.z);
  two.z = floor(pz - (CAMERA_HEIGHT_METERS - PLAYER_HEIGHT_METERS) / map_data.resolution.z);

  double viscosity = 0;

  if (!player_fly) {

    int vistotal = 0;

    for (int z = one.z; z <= two.z; z++) {
      for (int y = one.y; y <= two.y; y++) {
        for (int x = one.x; x <= two.x; x++) {
          if (block_data[map_get_block_type((struct int_xyz) {x, y, z})].visible) viscosity++;
          vistotal++;
        };
      };
    };

    viscosity /= vistotal;

    viscosity = sqrt(viscosity);

  };

  return viscosity;

};

static struct double_xyz collision_force;

//--page-split-- collision_test

static int collision_test(double px, double py, double pz) {

  int collision = 0;
  collision_force.x = 0;
  collision_force.y = 0;
  collision_force.z = 0;
  struct int_xyz one, two;

  one.x = floor(px - PLAYER_RADIUS_METERS / map_data.resolution.x);
  two.x = floor(px + PLAYER_RADIUS_METERS / map_data.resolution.x);
  one.y = floor(py - PLAYER_RADIUS_METERS / map_data.resolution.y);
  two.y = floor(py + PLAYER_RADIUS_METERS / map_data.resolution.y);

  if (!player_fly) {
    one.z = floor(pz - (CAMERA_HEIGHT_METERS - STAIRS_HEIGHT_METERS) / map_data.resolution.z);
  } else {
    one.z = floor(pz - CAMERA_HEIGHT_METERS / map_data.resolution.z);
  };
  two.z = floor(pz - (CAMERA_HEIGHT_METERS - PLAYER_HEIGHT_METERS) / map_data.resolution.z);

  for (int z = one.z; z <= two.z; z++) {
    for (int y = one.y; y <= two.y; y++) {
      for (int x = one.x; x <= two.x; x++) {
        if (x != one.x && x != two.x && y != one.y && y != two.y && z != one.z && z != two.z) continue;
        if (block_data[map_get_block_type((struct int_xyz) {x, y, z})].impassable) {
          collision = 1;
          if (x == one.x) collision_force.x++;
          if (x == two.x) collision_force.x--;
          if (y == one.y) collision_force.y++;
          if (y == two.y) collision_force.y--;
          if (z == one.z) collision_force.z++;
          if (z == two.z) collision_force.z--;
        };
      };
    };
  };

  math_normalize_vector(&collision_force);

  return collision;

};

//--page-split-- gravity_test

static double gravity_test(double px, double py, double pz) {

  // Returns (approximately) distance between feet and ground,
  // but multiplied by 1, and clamped to +/- 1.0, with positive
  // values meaning feet are above ground, and negative meaning
  // feet are in ground.

  int collision = 0;
  struct int_xyz one, two;

  one.x = floor(px - PLAYER_RADIUS_METERS / map_data.resolution.x);
  two.x = floor(px + PLAYER_RADIUS_METERS / map_data.resolution.x);
  one.y = floor(py - PLAYER_RADIUS_METERS / map_data.resolution.y);
  two.y = floor(py + PLAYER_RADIUS_METERS / map_data.resolution.y);
  one.z = floor(pz - (CAMERA_HEIGHT_METERS + 1.0) / map_data.resolution.z);
  two.z = floor(pz - (CAMERA_HEIGHT_METERS - 1.0) / map_data.resolution.z);

  int crunch;
  for (int z = one.z; z <= two.z; z++) {
    for (int y = one.y; y <= two.y; y++) {
      for (int x = one.x; x <= two.x; x++) {
        //if (x != one.x && x != two.x && y != one.y && y != two.y) continue;
        if (block_data[map_get_block_type((struct int_xyz) {x, y, z})].impassable) {
          collision = 1; crunch = z + 1;
        };
      };
    };
  };

  if (collision) {
    double blocks_above_crunch = pz - CAMERA_HEIGHT_METERS / map_data.resolution.z - crunch;
    double height_above_crunch = map_data.resolution.z * blocks_above_crunch;
    double gravity_scale = height_above_crunch;
    if (gravity_scale > +1.0) gravity_scale = +1.0;
    if (gravity_scale < -1.0) gravity_scale = -1.0;
    //printf("Gravity test: %0.3f\n", gravity_scale);
    return gravity_scale;
  } else {
    //printf("Gravity test: %0.3f\n", 1.0);
    return +1.0;
  };

};

//--page-split-- stairs_test

static double stairs_test(double px, double py, double pz) {

  return -gravity_test(px, py, pz);

};

#define KILL (-10.0 / 100.0)

//--page-split-- slide

static void slide() {

  struct double_xyzuv old = player_position;
  struct double_xyzuv new = player_position;

  new.x += player_velocity.x / map_data.resolution.x / fractions;
  new.y += player_velocity.y / map_data.resolution.y / fractions;
  new.z += player_velocity.z / map_data.resolution.z / fractions;

  struct int_xyz v;
  if (!collision_test(new.x, new.y, new.z) || player_noclip || (player_stuck && player_allow_noclip)) {
    //printf("Number zero...\n");

    player_position.x = new.x;
    player_position.y = new.y;
    player_position.z = new.z;

  } else {
    v.x = 0; v.y = 0; v.z = 0;
    // See what happens if we ignore specific components of the move.

    if (!collision_test(old.x, new.y, new.z)) v.x = 1;
    if (!collision_test(new.x, old.y, new.z)) v.y = 1;
    if (!collision_test(new.x, new.y, old.z)) v.z = 1;

    if (v.x + v.y + v.z == 1) {
      //printf("Number one...\n");

      // If ignoring one specific component is OK, then ignore only that one component.
      if (v.x) { player_velocity.x *= KILL; player_position.y = new.y; player_position.z = new.z; };
      if (v.y) { player_position.x = new.x; player_velocity.y *= KILL; player_position.z = new.z; };
      if (v.z) { player_position.x = new.x; player_position.y = new.y; player_velocity.z *= KILL; };

    } else if (v.x + v.y + v.z == 2) {
      //printf("Number two...\n");

      // If we have two options, choose to ignore the one with least magnitude.

      if (v.x && v.y) {
        if (player_velocity.x < player_velocity.y) {
          player_velocity.x *= KILL; player_position.y = new.y; player_position.z = new.z;
        } else {
          player_position.x = new.x; player_velocity.y *= KILL; player_position.z = new.z;
        };
      };

      if (v.x && v.z) {
        if (player_velocity.x < player_velocity.z) {
          player_velocity.x *= KILL; player_position.y = new.y; player_position.z = new.z;
        } else {
          player_position.x = new.x; player_position.y = new.y; player_velocity.z *= KILL;
        };
      };

      if (v.y && v.z) {
        if (player_velocity.y < player_velocity.z) {
          player_position.x = new.x; player_velocity.y *= KILL; player_position.z = new.z;
        } else {
          player_position.x = new.x; player_position.y = new.y; player_velocity.z *= KILL;
        };
      };

    } else if (v.x + v.y + v.z == 3) {
      //printf("Number three...\n");

      // If we have three options, choose the one with the least magnitude.

      if (player_velocity.z <= player_velocity.x && player_velocity.z <= player_velocity.y) {
        player_position.x = new.x; player_position.y = new.y; player_velocity.z *= KILL;
      } else if (player_velocity.y <= player_velocity.x) {
        player_position.x = new.x; player_velocity.y *= KILL; player_position.z = new.z;
      } else {
        player_velocity.x *= KILL; player_position.y = new.y; player_position.z = new.z;
      };

    } else {

      v.x = 0; v.y = 0; v.z = 0;

      // If ignoring just one component isn't enough, try two...

      if (!collision_test(new.x, old.y, old.z)) v.x = 1;
      if (!collision_test(old.x, new.y, old.z)) v.y = 1;
      if (!collision_test(old.x, old.y, new.z)) v.z = 1;

      if (v.x + v.y + v.z == 1) {
        //printf("Number four...\n");
        if (v.x) { player_position.x = new.x; player_velocity.y *= KILL; player_velocity.z *= KILL; };
        if (v.y) { player_velocity.x *= KILL; player_position.y = new.y; player_velocity.z *= KILL; };
        if (v.z) { player_velocity.x *= KILL; player_velocity.y *= KILL; player_position.z = new.z; };
      } else if (v.x + v.y + v.z == 2) {
        //printf("Number five...\n");
        int xy = sqrt(player_velocity.x * player_velocity.x + player_velocity.y * player_velocity.y);
        int xz = sqrt(player_velocity.x * player_velocity.x + player_velocity.z * player_velocity.z);
        int yz = sqrt(player_velocity.y * player_velocity.y + player_velocity.z * player_velocity.z);
        if (v.x && v.y) {
          if (yz <= xz) {
            player_position.x = new.x; player_velocity.y *= KILL; player_velocity.z *= KILL;
          } else {
            player_velocity.x *= KILL; player_position.y = new.y; player_velocity.z *= KILL;
          }
        };
        if (v.x && v.z) {
          if (yz <= xy) {
            player_position.x = new.x; player_velocity.y *= KILL; player_velocity.z *= KILL;
          } else {
            player_velocity.x *= KILL; player_velocity.y *= KILL; player_position.z = new.z;
          };
        };
        if (v.y && v.z) {
          if (xz < xy) {
            player_velocity.x *= KILL; player_position.y = new.y; player_velocity.z *= KILL;
          } else {
            player_velocity.x *= KILL; player_velocity.y *= KILL; player_position.z = new.z;
          };
        };
      } else if (v.x + v.y + v.z == 3) {
        //printf("Number six...\n");
        int xy = sqrt(player_velocity.x * player_velocity.x + player_velocity.y * player_velocity.y);
        int xz = sqrt(player_velocity.x * player_velocity.x + player_velocity.z * player_velocity.z);
        int yz = sqrt(player_velocity.y * player_velocity.y + player_velocity.z * player_velocity.z);
        if (xy <= xz && xy <= yz) {
          player_velocity.x *= KILL; player_velocity.y *= KILL; player_position.z = new.z;
        } else if (xz <= yz) {
          player_velocity.x *= KILL; player_position.y = new.y; player_velocity.z *= KILL;
        } else {
          player_position.x = new.x; player_velocity.y *= KILL; player_velocity.z *= KILL;
        };
      } else {
        //printf("Lucky seven...\n");
        player_velocity.x *= KILL;
        player_velocity.y *= KILL;
        player_velocity.z *= KILL;
      };

    };
  };

};

//--page-split-- player_get_vector

int player_get_vector(struct int_xyz *v) {
  struct int_xyz t;
  if (v == NULL) v = &t;
  v->x = 0; v->y = 0; v->z = 0;
  if (controls_get_key (CONTROLS_KEY_FORWARD)) v->x++;
  if (controls_get_key (CONTROLS_KEY_BACK)) v->x--;
  if (controls_get_key (CONTROLS_KEY_LEFT)) v->y++;
  if (controls_get_key (CONTROLS_KEY_RIGHT)) v->y--;
  if (controls_get_key (CONTROLS_KEY_FLY_UP)) v->z++;
  if (controls_get_key (CONTROLS_KEY_FLY_DOWN)) v->z--;

  return (v->x || v->y || v->z);
};

//--page-split-- player_movement

void player_movement() {
  DEBUG("enter player_movement()");
  static double last_movment_stamp = 0;
  static int start_time = 0;
  if (start_time == 0) start_time = on_frame_time;

  static int fraction_count;
  int next_fraction_count = floor(fractions * (on_frame_time - start_time));
  int milliseconds = next_fraction_count - fraction_count;
  fraction_count = next_fraction_count;

  if (!map_data.block) return;
  if (menu_function_pointer == menus_server_loading) return;

  if (!player_allow_flying) player_fly = 0;
  if (!player_allow_noclip) player_noclip = 0;

  double angle;
  if (option_anaglyph_enable) {
    angle = 2 * atan(0.5 * display_window_height * option_anaglyph_pixel_size / option_anaglyph_distance) / (display_window_height / 2);
  } else {
    angle = (map_perspective_angle * M_PI / 180) / display_window_height;
  };
  //angle = 0.1 * M_PI / 180.0;

  struct int_xyz keyboard = {};

  if (glfw_mouse_capture_flag) {

    player_get_vector(&keyboard);

    struct double_xy mouse;
    glfwGetCursorPos(glfw_window, &mouse.x, &mouse.y);
    mouse.x = player_mouse_x_accumulator;
    mouse.y = player_mouse_y_accumulator;
    player_mouse_x_accumulator = 0;
    player_mouse_y_accumulator = 0;

    player_position.u -= mouse.x * angle;
    if (option_mouse_invert) {
      player_position.v += mouse.y * angle;
    } else {
      player_position.v -= mouse.y * angle;
    };

    while (player_position.u < -M_PI) player_position.u += 2 * M_PI;
    while (player_position.u > +M_PI) player_position.u -= 2 * M_PI;
    if (player_position.v < -M_PI / 2) player_position.v = -M_PI / 2;
    if (player_position.v > +M_PI / 2) player_position.v = +M_PI / 2;

  };

  double speed, acceleration, deacceleration;

  #define MPH / 2.23693629

  #define WALKING_SPEED (walking_speed * SPEED_SCALE)
  #define RUNNING_SPEED (running_speed * SPEED_SCALE)
  #define SNEAKING_SPEED (walking_speed * SPEED_SCALE / 2.0)

  #define ACCELERATION (3.0 * SPEED_SCALE * SPEED_SCALE)

  #define FLYING_SPEED (running_speed * 2.0)
  #define FLYING_ACCELERATION (3.0 * 2.0 * 2.0)

  //#define FLYING_MULTIPLIER (RUNNING_SPEED / WALKING_SPEED)
  #define FAST_FLYING_MULTIPLIER 10
  #define FAST_FLYING_SPEED (FAST_FLYING_MULTIPLIER * FLYING_SPEED)
  #define FAST_FLYING_ACCELERATION (FAST_FLYING_MULTIPLIER * FLYING_SPEED)

  if (!player_fly) {
    if (controls_get_key (CONTROLS_KEY_RUN) == option_always_run) {
      speed = WALKING_SPEED;
      acceleration = ACCELERATION;
      deacceleration = ACCELERATION;
    } else {
      speed = RUNNING_SPEED;
      acceleration = ACCELERATION;
      deacceleration = ACCELERATION;
    };
    //if (glfwGetKey(GLFW_KEY_LCTRL) || glfwGetKey(GLFW_KEY_RCTRL)) {
    if (controls_get_key (CONTROLS_KEY_SNEAK)) {
      speed = SNEAKING_SPEED;
      acceleration = ACCELERATION;
      deacceleration = ACCELERATION;
    }
  } else {
    //if ((glfwGetKey(GLFW_KEY_LSHIFT) || glfwGetKey(GLFW_KEY_RSHIFT)) == option_fly_fast) {
    if (controls_get_key (CONTROLS_KEY_RUN) == option_fly_fast) {
      speed = FLYING_SPEED;
      acceleration = FLYING_ACCELERATION;
      deacceleration = FLYING_ACCELERATION;

      // add extra deceleration if moving quickly
      if (player_allow_speedy) {
        struct double_xyzuv orthogonal = {};
        orthogonal.x = player_velocity.x;
        orthogonal.y = player_velocity.y;
        if (player_fly) {
          orthogonal.z = player_velocity.z;
        } else {
          orthogonal.z = 0;
        };
        math_origin_vector(&orthogonal);
        if (orthogonal.x > speed) {
          deacceleration += (FAST_FLYING_ACCELERATION - deacceleration) * pow((orthogonal.x - speed) / (FAST_FLYING_SPEED - speed), 1.0 / 3.0);
        };
      };

    } else {
      if (!player_allow_flying || !player_allow_speedy) {
        speed = FLYING_SPEED;
        acceleration = FLYING_ACCELERATION;
        deacceleration = FLYING_ACCELERATION;
      } else if (!player_allow_speedy) {
        speed = 2.0 * FLYING_SPEED;
        acceleration = 2.0 * FLYING_ACCELERATION;
        deacceleration = 2.0 * FLYING_ACCELERATION;
      } else {
        speed = FAST_FLYING_SPEED;
        acceleration = FAST_FLYING_ACCELERATION;
        deacceleration = FAST_FLYING_ACCELERATION;
      };
    };
  };

  //printf("speed = %0.1f  accel = %0.1f  deaccel = %0.1f\n", speed, acceleration, deacceleration);

  for (int i = 0; i < milliseconds; i++) {

    //player_position.z = 60;

    double viscosity = viscosity_test(player_position.x, player_position.y, player_position.z);

    struct double_xyzuv new;
    new.x = keyboard.x;
    new.y = keyboard.y;
    new.z = keyboard.z;

    if (!player_fly) {
      if (option_superhuman) {
        new.z *= viscosity > 0;
      } else {
        new.z *= viscosity;
      };
      if (gravity_test(player_position.x, player_position.y, player_position.z) > 0.05) {
        if (option_superhuman) {
          new.x *= viscosity > 0;
          new.y *= viscosity > 0;
        } else {
          new.x *= viscosity;
          new.y *= viscosity;
        };
      };
    };

    // If the vector has a significant length, the player wants to move.
    double length = sqrt(new.x * new.x + new.y * new.y + new.z * new.z);

    if (length > 0.5) {

      // rotate the vector by the angle the player is turned to
      new.u = player_position.u; new.v = 0; math_rotate_vector(&new);

      // find the angles of the vector
      math_origin_vector(&new);

      // translate the current velocity to match the angle of the player's
      // acceleration, in order to determine the player's speed in that
      // direction, so that we can be sure not to accelerate beyond the
      // player's maximum speed, yet still allow acceleration in other
      // directions so that max speed doesn't inhibit making turns

      player_velocity.u = new.u; player_velocity.v = new.v;
      math_reverse_rotate_vector(&player_velocity);

      double power = pow(viscosity, 2.0);

      // decelerate in directions orthogonal to player acceleration
      struct double_xyzuv orthogonal = {};
      orthogonal.x = player_velocity.y;
      if (player_fly) {
        orthogonal.y = player_velocity.z;
      } else {
        orthogonal.y = 0;
      };
      math_origin_vector(&orthogonal);
      orthogonal.x -= deacceleration / fractions;
      if (orthogonal.x < 0) orthogonal.x = 0;
      math_rotate_vector(&orthogonal);
      player_velocity.y = orthogonal.x;
      if (player_fly) {
        player_velocity.z = orthogonal.y;
      };
      power = 1.0;

      // accelerate up to maximum speed
      if (player_velocity.x < speed) {
        //player_velocity.x += power * acceleration / fractions;
        if (player_velocity.x < -speed) {
          player_velocity.x += deacceleration / fractions;
        } else {
          player_velocity.x += acceleration / fractions;
        };
        if (player_velocity.x > speed) player_velocity.x = speed;
      } else {
        player_velocity.x -= deacceleration / fractions;
      };

      // translate velocity back to its normal angle
      player_velocity.u = new.u; player_velocity.v = new.v;
      math_rotate_vector(&player_velocity);

    } else if (player_fly || gravity_test(player_position.x, player_position.y, player_position.z) <= 0.05) {

      // decelerate in all directions
      struct double_xyzuv orthogonal = {};
      orthogonal.x = player_velocity.x;
      orthogonal.y = player_velocity.y;
      if (player_fly) {
        orthogonal.z = player_velocity.z;
      } else {
        orthogonal.z = 0;
      };
      math_origin_vector(&orthogonal);
      orthogonal.x -= deacceleration / fractions;
      if (orthogonal.x < 0) orthogonal.x = 0;
      math_rotate_vector(&orthogonal);
      player_velocity.x = orthogonal.x;
      player_velocity.y = orthogonal.y;
      if (player_fly) {
        player_velocity.z = orthogonal.z;
      };

    };

    // Add air/water friction.
    struct double_xyzuv orthogonal = {};
    orthogonal.x = player_velocity.x;
    orthogonal.y = player_velocity.y;
    orthogonal.z = player_velocity.z;
    math_origin_vector(&orthogonal);
    orthogonal.x -= (0.004 + 7.0 * viscosity) * pow(orthogonal.x, 1.0) / fractions;
    if (orthogonal.x < 0) orthogonal.x = 0;
    math_rotate_vector(&orthogonal);
    player_velocity.x = orthogonal.x;
    player_velocity.y = orthogonal.y;
    player_velocity.z = orthogonal.z;

    player_stuck = collision_test(player_position.x, player_position.y, player_position.z);

    if (!player_noclip && player_stuck && player_allow_noclip) {
      player_position.x += collision_force.x / map_data.resolution.x / fractions;
      player_position.y += collision_force.y / map_data.resolution.y / fractions;
      player_position.z += collision_force.z / map_data.resolution.z / fractions;
      if (player_allow_noclip) slide();
    } else {

      slide();

      if (0) {
        static double last_support_time;
        static int last_support_test;
        static double highest_point;
        int this_support_test = gravity_test(player_position.x, player_position.y, player_position.z) <= 0.0;
        if (last_support_test != this_support_test) {
          if (last_support_test) {
            last_support_time = on_frame_time;
            highest_point = -1000;
          } else {
            double fall_time = on_frame_time - last_support_time;
            if (fall_time > 0.1) {
              printf("Fall of %0.3f meters took %0.3f seconds!\n", (highest_point - player_position.z) * map_data.resolution.z, fall_time);
            };
          };
        };
        last_support_test = this_support_test;
        if (highest_point < player_position.z) highest_point = player_position.z;
      };

      double nz = player_position.z + walking_speed / map_data.resolution.z / fractions;

      int jump_press = controls_get_key (CONTROLS_KEY_JUMP);

      static int jump_release; int jump;
      jump = jump_press && jump_release && glfw_mouse_capture_flag;
      jump_release = !jump_press;

      if (jump && gravity_test(player_position.x, player_position.y, player_position.z) < 0.1) {
        player_velocity.z = 2.6 * GRAVITY_SCALE * (option_superhuman ? 2.0 : 1.0);
        falling = 0.2 * fractions;
      };

      if (!player_fly && (!player_noclip || !player_stuck)) {

        double acceleration_due_to_gravity = 9.81 * GRAVITY_SCALE;
        #define GRAVITY acceleration_due_to_gravity

        double gravity = gravity_test(player_position.x, player_position.y, player_position.z);

        if (gravity >= 0.5) falling = 0.2 * fractions;
        //if (falling && gravity <= 0.01 && player_velocity.z >= -0.01) falling--;
        if (falling && gravity <= 0.05) falling--;

        if (!falling) {
          if (gravity > 0.0 && gravity < 0.5) {
            player_position.z -= 3.0 / map_data.resolution.z / fractions;
            //GRAVITY *= 2.0;
          };
        };

        // This adds the downward acceleration due to gravity.
        player_velocity.z -= (1.0 - 0.95 * pow(viscosity, 2.0)) * GRAVITY / fractions;

        // I think this checks how far into the ground the player is.

        // This shit calculates how much upward force to apply to get the player out of the ground.
        #if 0
        double scale = pow(10, -10 * gravity);
        if (scale > 2.0) scale = 2.0;
        double force = scale * GRAVITY / fractions;
        //printf("v = %0.3f  force = %0.3f\n", player_velocity.z, force);
        if (player_velocity.z < -GRAVITY * gravity) {
          player_velocity.z += force;
        };
        #endif

        if (gravity < -0.01) {
          player_position.z += 3.0 / map_data.resolution.z / fractions;
        };
        if (gravity < 0.00) {
          if (player_velocity.z < 0) player_velocity.z = 0.0;
        };

        #if 0
        double length = sqrt(player_velocity.x * player_velocity.x + player_velocity.y * player_velocity.y);
        if (rise > 1.25 && length > walking_speed / 3.0) {
          double shorter = length - scale * rise * GRAVITY / fractions;
          //if (shorter < 0) shorter = 0;
          player_velocity.x *= shorter / length;
          player_velocity.y *= shorter / length;
        };
        #endif
      };

    };

    if (gravity_test(player_position.x, player_position.y, player_position.z) < 0.05) {
      static struct double_xyzuv foot_position;
      struct double_xyz diff;
      diff.x = (player_position.x - foot_position.x) * map_data.resolution.x;
      diff.y = (player_position.y - foot_position.y) * map_data.resolution.y;
      diff.z = (player_position.z - foot_position.z) * map_data.resolution.z;

      double distance = sqrt(diff.x * diff.x + diff.y * diff.y);
      double speed = sqrt(player_velocity.x * player_velocity.x + player_velocity.y * player_velocity.y);
      double threshold = STRIDE_WALK + (STRIDE_RUN - STRIDE_WALK) * (speed - walking_speed) / (running_speed - walking_speed);

      if (distance >= threshold) {
        sound_play (SOUND_WALK, 0.2f, NULL);
        foot_position = player_position;
      };
    };

  };

  if (map_data.block) {
    if (map_data.wrap.x) {
      while (player_position.x < 0) player_position.x += map_data.dimension.x;
      while (player_position.x >= map_data.dimension.x) player_position.x -= map_data.dimension.x;
    } else {
      #if 0
      if (player_position.x < 0.5) {
        player_position.x = 0.5;
        if (player_velocity.x < 0) player_velocity.x = 0;
      };
      if (player_position.x >= map_data.dimension.x - 0.5) {
        player_position.x = map_data.dimension.x - 0.5;
        if (player_velocity.x > 0) player_velocity.x = 0;
      };
      #endif
    };
    if (map_data.wrap.y) {
      while (player_position.y < 0) player_position.y += map_data.dimension.y;
      while (player_position.y >= map_data.dimension.y) player_position.y -= map_data.dimension.y;
    } else {
      #if 0
      if (player_position.y < 0.5) {
        player_position.y = 0.5;
        if (player_velocity.y < 0) player_velocity.y = 0;
      };
      if (player_position.y >= map_data.dimension.y - 0.5) {
        player_position.y = map_data.dimension.y - 0.5;
        if (player_velocity.y > 0) player_velocity.y = 0;
      };
      #endif
    };
    if (map_data.wrap.z) {
      while (player_position.z < 0) player_position.z += map_data.dimension.z;
      while (player_position.z >= map_data.dimension.z) player_position.z -= map_data.dimension.z;
    } else {
      #if 0
      if (player_position.z < 0.5) {
        player_position.z = 0.5;
        if (player_velocity.z < 0) player_velocity.z = 0;
      };
      if (player_position.z >= map_data.dimension.z - 0.5) {
        player_position.z = map_data.dimension.z - 0.5;
        if (player_velocity.z > 0) player_velocity.z = 0;
      };
      #endif
    };
  };

  // Adjust view for head and neck offset...

  struct double_xyzuv offset;
  offset.x = 0.06;
  offset.y = 0.00;
  offset.z = 0.06;
  offset.u = player_position.u;
  offset.v = player_position.v;
  math_rotate_vector(&offset);

  player_view.x = player_position.x + offset.x / map_data.resolution.x;
  player_view.y = player_position.y + offset.y / map_data.resolution.y;
  player_view.z = player_position.z + offset.z / map_data.resolution.z;
  player_view.u = player_position.u;
  player_view.v = player_position.v;

  player_view = player_position;

  DEBUG("leave player_movement()");
};
