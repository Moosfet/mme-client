#include "everything.h"

#define FRACTIONS 1000.0

#define PROJECTILE_PAPER_AIRPLANE 0
#define PROJECTILE_BOUNCY_BALL 1

static struct projectile {
  int active;
  int type;
  int who;
  struct double_xyzuv position;
  double speed;
  struct double_xyzuv velocity;
  double bounce;
  double gravity;
} *projectile = NULL;
static int projectile_count = 0;

//--page-split-- projectile_reset

void projectile_reset() {
  projectile_count = 0;
  memory_allocate(&projectile, projectile_count);
}

//--page-split-- projectile_render

void projectile_render() {

  glBegin(GL_QUADS);

  for (struct projectile *d = &projectile[0]; d < &projectile[projectile_count]; d++) {

    if (!d->active) continue;

    chat_color(11);
    double the_size = 0.125;
    double lx = d->position.x - the_size;
    double lz = d->position.y - the_size;
    double ly = d->position.z - the_size;
    double hx = d->position.x + the_size;
    double hz = d->position.y + the_size;
    double hy = d->position.z + the_size;

    glVertex3f(lx, lz, hy);
    glVertex3f(hx, lz, hy);
    glVertex3f(hx, hz, hy);
    glVertex3f(lx, hz, hy);

    glVertex3f(lx, lz, ly);
    glVertex3f(lx, hz, ly);
    glVertex3f(hx, hz, ly);
    glVertex3f(hx, lz, ly);

    glVertex3f(lx, hz, ly);
    glVertex3f(lx, hz, hy);
    glVertex3f(hx, hz, hy);
    glVertex3f(hx, hz, ly);

    glVertex3f(lx, lz, ly);
    glVertex3f(hx, lz, ly);
    glVertex3f(hx, lz, hy);
    glVertex3f(lx, lz, hy);

    glVertex3f(hx, lz, ly);
    glVertex3f(hx, hz, ly);
    glVertex3f(hx, hz, hy);
    glVertex3f(hx, lz, hy);

    glVertex3f(lx, lz, ly);
    glVertex3f(lx, lz, hy);
    glVertex3f(lx, hz, hy);
    glVertex3f(lx, hz, ly);

  }

  glEnd();

}

//--page-split-- collision_test

static int collision_test (double x, double y, double z) {

    struct int_xyz block;
    block.x = floor(x);
    block.y = floor(y);
    block.z = floor(z);

    int type = map_get_block_type(block);

    return (block_data[type].impassable);

}
//#define KILL -0.25

//--page-split-- slide

static void slide(struct projectile *d) {

  struct double_xyzuv old = d->position;
  struct double_xyzuv new = d->position;

  new.x += d->velocity.x / map_data.resolution.x / FRACTIONS;
  new.y += d->velocity.y / map_data.resolution.y / FRACTIONS;
  new.z += d->velocity.z / map_data.resolution.z / FRACTIONS;

  struct int_xyz v;

  if (!collision_test(new.x, new.y, new.z)) {
    //printf("Number zero...\n");

    d->position.x = new.x;
    d->position.y = new.y;
    d->position.z = new.z;

  } else {

    v.x = 0; v.y = 0; v.z = 0;
    // See what happens if we ignore specific components of the move.

    if (!collision_test(old.x, new.y, new.z)) v.x = 1;
    if (!collision_test(new.x, old.y, new.z)) v.y = 1;
    if (!collision_test(new.x, new.y, old.z)) v.z = 1;

    if (v.x + v.y + v.z == 1) {
      //printf("Number one...\n");

      // If ignoring one specific component is OK, then ignore only that one component.
      if (v.x) { d->velocity.x *= d->bounce; d->position.y = new.y; d->position.z = new.z; };
      if (v.y) { d->position.x = new.x; d->velocity.y *= d->bounce; d->position.z = new.z; };
      if (v.z) { d->position.x = new.x; d->position.y = new.y; d->velocity.z *= d->bounce; };

      if (v.z && d->velocity.z > 0) {
        // This is the case when bouncing vertically, like on the ground.
        // Let's make this case subtract from x and y velocity as well,
        // so that a rapidly bouncing ball slows down faster.
        d->velocity.x *= 0.9;
        d->velocity.y *= 0.9;
      };

    } else if (v.x + v.y + v.z == 2) {
      //printf("Number two...\n");

      // If we have two options, choose to ignore the one with least magnitude.

      if (v.x && v.y) {
        if (d->velocity.x < d->velocity.y) {
          d->velocity.x *= d->bounce; d->position.y = new.y; d->position.z = new.z;
        } else {
          d->position.x = new.x; d->velocity.y *= d->bounce; d->position.z = new.z;
        };
      };

      if (v.x && v.z) {
        if (d->velocity.x < d->velocity.z) {
          d->velocity.x *= d->bounce; d->position.y = new.y; d->position.z = new.z;
        } else {
          d->position.x = new.x; d->position.y = new.y; d->velocity.z *= d->bounce;
        };
      };

      if (v.y && v.z) {
        if (d->velocity.y < d->velocity.z) {
          d->position.x = new.x; d->velocity.y *= d->bounce; d->position.z = new.z;
        } else {
          d->position.x = new.x; d->position.y = new.y; d->velocity.z *= d->bounce;
        };
      };

    } else if (v.x + v.y + v.z == 3) {
      //printf("Number three...\n");

      // If we have three options, choose the one with the least magnitude.

      if (d->velocity.z <= d->velocity.x && d->velocity.z <= d->velocity.y) {
        d->position.x = new.x; d->position.y = new.y; d->velocity.z *= d->bounce;
      } else if (d->velocity.y <= d->velocity.x) {
        d->position.x = new.x; d->velocity.y *= d->bounce; d->position.z = new.z;
      } else {
        d->velocity.x *= d->bounce; d->position.y = new.y; d->position.z = new.z;
      };

    } else {

      v.x = 0; v.y = 0; v.z = 0;

      // If ignoring just one component isn't enough, try two...

      if (!collision_test(new.x, old.y, old.z)) v.x = 1;
      if (!collision_test(old.x, new.y, old.z)) v.y = 1;
      if (!collision_test(old.x, old.y, new.z)) v.z = 1;

      if (v.x + v.y + v.z == 1) {
        //printf("Number four...\n");
        if (v.x) { d->position.x = new.x; d->velocity.y *= d->bounce; d->velocity.z *= d->bounce; };
        if (v.y) { d->velocity.x *= d->bounce; d->position.y = new.y; d->velocity.z *= d->bounce; };
        if (v.z) { d->velocity.x *= d->bounce; d->velocity.y *= d->bounce; d->position.z = new.z; };
      } else if (v.x + v.y + v.z == 2) {
        //printf("Number five...\n");
        int xy = sqrt(d->velocity.x * d->velocity.x + d->velocity.y * d->velocity.y);
        int xz = sqrt(d->velocity.x * d->velocity.x + d->velocity.z * d->velocity.z);
        int yz = sqrt(d->velocity.y * d->velocity.y + d->velocity.z * d->velocity.z);
        if (v.x && v.y) {
          if (yz <= xz) {
            d->position.x = new.x; d->velocity.y *= d->bounce; d->velocity.z *= d->bounce;
          } else {
            d->velocity.x *= d->bounce; d->position.y = new.y; d->velocity.z *= d->bounce;
          }
        };
        if (v.x && v.z) {
          if (yz <= xy) {
            d->position.x = new.x; d->velocity.y *= d->bounce; d->velocity.z *= d->bounce;
          } else {
            d->velocity.x *= d->bounce; d->velocity.y *= d->bounce; d->position.z = new.z;
          };
        };
        if (v.y && v.z) {
          if (xz < xy) {
            d->velocity.x *= d->bounce; d->position.y = new.y; d->velocity.z *= d->bounce;
          } else {
            d->velocity.x *= d->bounce; d->velocity.y *= d->bounce; d->position.z = new.z;
          };
        };
      } else if (v.x + v.y + v.z == 3) {
        //printf("Number six...\n");
        int xy = sqrt(d->velocity.x * d->velocity.x + d->velocity.y * d->velocity.y);
        int xz = sqrt(d->velocity.x * d->velocity.x + d->velocity.z * d->velocity.z);
        int yz = sqrt(d->velocity.y * d->velocity.y + d->velocity.z * d->velocity.z);
        if (xy <= xz && xy <= yz) {
          d->velocity.x *= d->bounce; d->velocity.y *= d->bounce; d->position.z = new.z;
        } else if (xz <= yz) {
          d->velocity.x *= d->bounce; d->position.y = new.y; d->velocity.z *= d->bounce;
        } else {
          d->position.x = new.x; d->velocity.y *= d->bounce; d->velocity.z *= d->bounce;
        };
      } else {
        //printf("Lucky seven...\n");
        d->velocity.x *= d->bounce;
        d->velocity.y *= d->bounce;
        d->velocity.z *= d->bounce;
      };

    };

    double speed = sqrt(d->velocity.x * d->velocity.x + d->velocity.y * d->velocity.y + d->velocity.z * d->velocity.z);
    speed *= map_data.resolution.x;
    if (speed < 0.01) { // increased from 0.002 since that was causing some projectiles to never disappear from map, seemingly
      d->active = 0;    // because location isn't changed on bounce, so momentum is never actually lost at such small scales.
    } else {
      //printf("speed = %f, it's at %0.2f %0.2f %0.2f\n", speed, d->position.x, d->position.y, d->position.z);

      //sound_event_bounce();
      struct double_xyz p = {d->position.x, d->position.y, d->position.z};
      //extern short data_bounce_pcm; extern int size_bounce_pcm;
      //mixer_play(&data_bounce_pcm, size_bounce_pcm >> 1, speed * speed, &p);

      sound_play(SOUND_BOUNCE, speed * speed, &p);
    };

  };

};

//--page-split-- projectile_announce_collision

static void projectile_announce_collision (struct projectile *d, int type, int id) {
//  sound_event_collision();

  //double volume = d->velocity.x * d->velocity.x + d->velocity.y * d->velocity.y + d->velocity.z * d->velocity.z;
  // Since it's a glass break sound, we'll assume it's always full volume.

  struct double_xyz p = {d->position.x, d->position.y, d->position.z};
  //extern short data_collision_pcm; extern int size_collision_pcm;
  //mixer_play(&data_collision_pcm, size_collision_pcm >> 1, 4.0f, &p);

  sound_play(SOUND_COLLISION, 4.0f, &p);

  d->active = 0;
  if (d->who == 0)
      packet_send (PACKET_PROJECTILE_COLLISION, d->type, d->position.x,
                    d->position.y, d->position.z, type, id);
}

//--page-split-- projectile_collision_proc

static int projectile_collision_proc (struct projectile *d) {
  struct int_xyz block;
  block.x = floor(d->position.x);
  block.y = floor(d->position.y);
  block.z = floor(d->position.z);

  int type = map_get_block_type(block);
  if (block_data[type].impassable) {
    d->active = 0;
    if (d->who == 0) {
        projectile_announce_collision (d, 0, type);
        return (1);
    }

  } else {

    // Moosfet changed this, as the player's image is probably wider than their width:
    //double player_width = (player_radius_meters) / map_data.resolution.x;
    double player_width = (PLAYER_HEIGHT_METERS * 0.333) / map_data.resolution.x;

    double player_height = PLAYER_HEIGHT_METERS / map_data.resolution.x;
    for (int i = 0; i < 256; i++) {
      if (model_player[i].valid) {
        double model_x_min = model_player[i].position.x - player_width;
        double model_x_max = model_player[i].position.x + player_width;
        double model_y_min = model_player[i].position.y - player_width;
        double model_y_max = model_player[i].position.y + player_width;
        double model_z_min = model_player[i].position.z - (player_height / map_data.resolution.x) ;
        double model_z_max = model_player[i].position.z;// + player_height;
        if (d->position.x >= model_x_min && d->position.x <= model_x_max &&
            d->position.y >= model_y_min && d->position.y <= model_y_max &&
            d->position.z >= model_z_min && d->position.z <= model_z_max) {

            if (d->who == 0) {
                projectile_announce_collision (d, 1, i);
                return (1);
            }
          }
        }
      }
    }
    return (0);
}

//--page-split-- projectile_allocate

static struct projectile *projectile_allocate() {
  if (projectile == NULL) {
    memory_allocate(&projectile, sizeof (struct projectile));
    projectile[0].active = 0;
  }
  int empty = -1;
  int a;
  for (a = 0; a < projectile_count; a++) {
    if (projectile[a].active) continue;
    empty = a;
    break;
  }
  if (a == projectile_count) {
    empty = projectile_count;
    projectile_count++;
    memory_allocate(&projectile, sizeof (struct projectile) * (projectile_count + 1));
  }
//  memset (&projectile[a], 0, sizeof (struct projectile));
  return &projectile[a];
}

//--page-split-- projectile_create

void projectile_create(int who, int type, struct double_xyzuv start_position) {

  struct projectile *d = projectile_allocate();
  d->type = type;
  d->active = 1;
  d->who = who;

  if (type == PROJECTILE_PAPER_AIRPLANE) {
    d->speed = 20; //projectile speed in m/s
    d->bounce = 0.0;
    d->gravity = 9.8;

  } else {
    d->speed = 20; //projectile speed in m/s
    d->bounce = -0.80;
    d->gravity = 9.8;
  }

  // Set initial velocity to match player's aim.

  d->velocity.x = d->speed;
  d->velocity.y = 0.0;
  d->velocity.z = 0.0;
  d->velocity.u = start_position.u;
  d->velocity.v = start_position.v;

  if (d->who == 0) {
    // Let's try to choose an upward angle that actually hits the target
    // the player is pointing at.  To do that, we first need to know
    // what they are pointing at, so we'll follow a straight line vector
    // until it hits something...

    struct double_xyzuv p, v;
    p = start_position;
    v.x = 0.1;
    v.y = 0.0;
    v.z = 0.0;
    v.u = start_position.u;
    v.v = start_position.v;
    math_rotate_vector(&v);

    struct int_xyz c;
    while (1) {
      c.x = floor(p.x); c.y = floor(p.y); c.z = floor(p.z);
      if (block_data[map_get_block_type(c)].impassable) break;
      p.x += v.x; p.y += v.y; p.z += v.z;
    };

    //printf("Targeting block %d, %d, %d\n", c.x, c.y, c.z);

    // Now we know the coordinate of what we hit, so we need to find
    // the ground distance to that point, and the change in elevation
    // between us and it.

    struct double_xyzuv target;
    target.x = c.x + 0.5;
    target.y = c.y + 0.5;
    target.z = c.z + 0.5;
    target.x -= start_position.x;
    target.y -= start_position.y;
    target.z -= start_position.z;
    target.u = -atan2(target.y, target.x);
    target.v = 0;
    math_rotate_vector(&target);

    //printf("Target is %0.1f blocks away and %0.1f blocks high.\n", target.x, target.z);

    // convert to meters...
    target.x *= map_data.resolution.x;
    target.z *= map_data.resolution.x;

    // Then we just do some math...

    {
      double a = (-d->gravity * target.x * target.x) / (2 * d->speed * d->speed);
      double b = target.x;
      double c = a - target.z;
      double e = sqrt(b * b - 4 * a * c);
      double s1 = (-b + e) / (2 * a);
      double s2 = (-b - e) / (2 * a);
      //printf("a=%f, b=%f, c=%f, s1=%f, s2=%f\n", a, b, c, s1, s2);
      double angle = atan(s1);

      // If the result isn't NAN, then use it:
      if (angle > -4 && angle < +4) d->velocity.v = angle;
      //printf("Launching at angle %0.1f degrees.\n\n", d->velocity.v * 180 / M_PI);

    };

  };

  math_rotate_vector(&d->velocity);

  d->velocity.x += player_velocity.x;
  d->velocity.y += player_velocity.y;
  d->velocity.z += player_velocity.z;

  d->position.x = 0;
  d->position.y = -0.1 / map_data.resolution.x;
  d->position.z = -0.1 / map_data.resolution.x;
  d->position.u = start_position.u;
  d->position.v = start_position.v;
  math_rotate_vector(&d->position);
  d->position.x += start_position.x;
  d->position.y += start_position.y;
  d->position.z += start_position.z;

  //Don't announce projectiles created by other clients
  if (d->who == 0) {
    packet_send (PACKET_PROJECTILE_CREATE, d->type, (float)start_position.x,
        (float)start_position.y, (float)start_position.z, (float)start_position.u,
        (float)start_position.v);
  }

  sound_play (SOUND_THROW, 0.2f, NULL);
  return;
}

//--page-split-- projectile_movement

void projectile_movement() {

  // Figure out how many 1/1000th of a second has passed
  // between each frame, but do so in a way that rounding
  // errors don't cause it to be 1001 or only 999...

  static double start_time = 0;
  static int fraction_count;

  if (start_time == 0) start_time = on_frame_time;

  int next_fraction_count = floor(FRACTIONS * (on_frame_time - start_time));
  int milliseconds = next_fraction_count - fraction_count;
  fraction_count = next_fraction_count;

  for (int i = 0; i < milliseconds; i++) {
    struct projectile *d;
    for (d = &projectile[0]; d < &projectile[projectile_count]; d++) {

      if (!d->active) continue;

      if (d->bounce) {
        slide(d);
      } else {
        d->position.x += d->velocity.x / map_data.resolution.x / 1000.0;
        d->position.y += d->velocity.y / map_data.resolution.x / 1000.0;
        d->position.z += d->velocity.z / map_data.resolution.x / 1000.0;
      }
      d->velocity.z -= d->gravity / 1000.0;

      projectile_collision_proc (d);

      // added a new speed check into slide() function
      // so this one isn't needed:

      //double speed = sqrt (d->velocity.x * d->velocity.x + d->velocity.y * d->velocity.y + d->velocity.z * d->velocity.z);
      //if (speed < 10) {
      //  projectile_announce_collision (d, 0, 0);
      //  d->active = 0;
      //}

    };
  };
};
