#include "everything.h"

static GLuint default_texture;
struct structure_model_player model_player[256] = {};
static struct double_xyzuv smoothicate[256] = {};

#define NOT_REAL(x) ( !( (x) > -1000000000.0 && (x) < +1000000000.0 ) )

//--page-split-- model_open_window

void model_open_window(void) {
//  extern char data_player_png; extern int size_player_png;
//  default_texture = texture_load(&data_player_png, size_player_png, TEXTURE_FLAG_PIXELATE);
  extern char data_boy_png; extern int size_boy_png;
  default_texture = texture_load(&data_boy_png, size_boy_png, 0);
//  extern char data_girl_png; extern int size_girl_png;
//  texture = texture_load(&data_girl_png, size_girl_png, 0);
};

//--page-split-- model_close_window

void model_close_window(void) {
  glDeleteTextures(1, &default_texture);
};

//--page-split-- model_reset

void model_reset(void) {
  for (int i = 0; i < 256; i++) {
    model_player[i].valid = 0;
    model_player[i].position = (struct double_xyzuv) {};
    model_player[i].name[0] = 0;
    model_player[i].color = 0;
    model_player[i].texture = -1;
    smoothicate[i] = (struct double_xyzuv) {};
  };
};

//--page-split-- model_move

void model_move(int player, struct double_xyzuv position) {
  model_player[player].position = position;
  if (!model_player[player].valid) {
    if (NOT_REAL(position.x)) return;
    if (NOT_REAL(position.y)) return;
    if (NOT_REAL(position.z)) return;
    if (NOT_REAL(position.u)) return;
    if (NOT_REAL(position.v)) return;
    smoothicate[player] = position;
  };
};

//--page-split-- model_render

void model_render(void) {

  struct double_xyzuv pp = player_view;

  static double last_time = 0;
  double this_time = on_frame_time;
  double fraction = 10.0 * (this_time - last_time);
  if (fraction > 1.0) fraction = 1.0;
  last_time = this_time;

  for (int i = 0; i < 256; i++) {
    if (model_player[i].valid) {

      // Skip rendering if position is NAN or INF.
      if (NOT_REAL(model_player[i].position.x)) continue;
      if (NOT_REAL(model_player[i].position.y)) continue;
      if (NOT_REAL(model_player[i].position.z)) continue;
      if (NOT_REAL(model_player[i].position.u)) continue;
      if (NOT_REAL(model_player[i].position.v)) continue;

      if (model_player[i].position.u - smoothicate[i].u >= M_PI) smoothicate[i].u += 2 * M_PI;
      if (model_player[i].position.u - smoothicate[i].u < -M_PI) smoothicate[i].u -= 2 * M_PI;
      if (model_player[i].position.v - smoothicate[i].v >= M_PI) smoothicate[i].v += 2 * M_PI;
      if (model_player[i].position.v - smoothicate[i].v < -M_PI) smoothicate[i].v -= 2 * M_PI;
      smoothicate[i].x += fraction * (model_player[i].position.x - smoothicate[i].x);
      smoothicate[i].y += fraction * (model_player[i].position.y - smoothicate[i].y);
      smoothicate[i].z += fraction * (model_player[i].position.z - smoothicate[i].z);
      smoothicate[i].u += fraction * (model_player[i].position.u - smoothicate[i].u);
      smoothicate[i].v += fraction * (model_player[i].position.v - smoothicate[i].v);

      // If these ever become INF or NAN for any reason, reset them.
      if (NOT_REAL(smoothicate[i].x)) smoothicate[i].x = model_player[i].position.x;
      if (NOT_REAL(smoothicate[i].y)) smoothicate[i].y = model_player[i].position.y;
      if (NOT_REAL(smoothicate[i].z)) smoothicate[i].z = model_player[i].position.z;
      if (NOT_REAL(smoothicate[i].u)) smoothicate[i].u = model_player[i].position.u;
      if (NOT_REAL(smoothicate[i].v)) smoothicate[i].v = model_player[i].position.v;

      struct double_xyzuv mp = smoothicate[i];

      glPushMatrix();
      glTranslated(mp.x, mp.y, mp.z);
      glScaled(1.0 / map_data.resolution.x, 1.0 / map_data.resolution.y, 1.0 / map_data.resolution.z);

      struct double_xyzuv a = mp;
      a.x -= pp.x; a.y -= pp.y; a.z -= pp.z;
      a.u = 0.0; a.v = 0.0;
      math_origin_vector(&a);

      int angle = round(8.0 * (mp.u - a.u) / (2 * M_PI) + 4);
      while (angle < 0) angle += 8; while (angle >= 8) angle -= 8;
      double fraction = 8.0 * (mp.u - a.u) / (2 * M_PI) + 4;
      fraction = 45 * (fraction - round(fraction));
      fraction = 0;

      // select which part of the texture to use
      // depending on player's angle vs. the camera
      double coord[4];
      switch (angle) {
        case 0:
          coord[0] = 0.0;
          coord[1] = 0.5;
          coord[2] = 0.25;
          coord[3] = 1.0;
        break;
        case 1:
          coord[0] = 0.0;
          coord[1] = 0.0;
          coord[2] = 0.25;
          coord[3] = 0.5;
        break;
        case 2:
          coord[0] = 0.25;
          coord[1] = 0.5;
          coord[2] = 0.5;
          coord[3] = 1.0;
        break;
        case 3:
          coord[0] = 0.25;
          coord[1] = 0.0;
          coord[2] = 0.5;
          coord[3] = 0.5;
        break;
        case 4:
          coord[0] = 0.5;
          coord[1] = 0.5;
          coord[2] = 0.75;
          coord[3] = 1.0;
        break;
        case 5:
          coord[0] = 0.5;
          coord[1] = 0.0;
          coord[2] = 0.75;
          coord[3] = 0.5;
        break;
        case 6:
          coord[0] = 0.75;
          coord[1] = 0.5;
          coord[2] = 1.0;
          coord[3] = 1.0;
        break;
        case 7:
          coord[0] = 0.75;
          coord[1] = 0.0;
          coord[2] = 1.0;
          coord[3] = 0.5;
        break;
      };

      glRotated(a.u * 180 / M_PI - 90 + fraction, 0, 0, +1);

      double height = PLAYER_HEIGHT_METERS;
      double below = CAMERA_HEIGHT_METERS;
      double above = height - below;
      double width = height / 2.0;

      glEnable(GL_TEXTURE_2D);
      if (model_player[i].texture >= 0) {
        glCallList(texture_list_base + model_player[i].texture + RENDER_IN_GRAYSCALE);
      } else {
        glBindTexture(GL_TEXTURE_2D, default_texture + RENDER_IN_GRAYSCALE);
      };
      glColor3f(1.0, 1.0, 1.0);
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GEQUAL, 0.5);
      glBegin(GL_QUADS);
      glTexCoord2f(coord[0], coord[1]); glVertex3f(width / -2.0, 0, -below);
      glTexCoord2f(coord[2], coord[1]); glVertex3f(width / +2.0, 0, -below);
      glTexCoord2f(coord[2], coord[3]); glVertex3f(width / +2.0, 0, +above);
      glTexCoord2f(coord[0], coord[3]); glVertex3f(width / -2.0, 0, +above);
      glEnd();
      glDisable(GL_ALPHA_TEST);

      glRotated(a.u * 180 / M_PI - 90 + fraction, 0, 0, -1);
      glRotated(pp.u * 180 / M_PI - 90, 0, 0, +1);
      //glRotated(-fraction, 0, 0, +1);

      //glRotated(player_view.u * 180 / M_PI, 0, 0, +1);
      glRotated(90, +1, 0, 0);
      glRotated(180, 0, +1, 0);
      glRotated(180, 0, 0, +1);
      //glRotated(-90, 0, 0, +1);
      //glRotated(player_view.v * 180 / M_PI - 90, +1, 0, 0);
      glTranslated(0, -0.2, 0);
      glScaled(0.007, 0.007, 0.007);
      glRotated(pp.v * 180 / M_PI, 1, 0, 0);
      glTranslated(0, -24, 0);

      chat_color(model_player[i].color);
      gui_model_name(model_player[i].name);

      glPopMatrix();

    };
  };

  glDisable(GL_TEXTURE_2D);

};
