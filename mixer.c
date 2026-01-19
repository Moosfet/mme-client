#include "everything.h"

#define MAX_SOUNDS 64

struct mixer_table {
  short *pcm;
  int count;
  float volume;
  int has_location;
  struct double_xyz location;
  float l, r;
};

static struct mixer_table table[MAX_SOUNDS] = {};

static pthread_mutex_t table_access_mutex;

//--page-split-- mixer_initialize

void mixer_initialize() {
  easy_mutex_init(&table_access_mutex, 0);
  memset(table, 0, sizeof(table));
};

// Note that mixer_generate() is called from the audio thread,
// while other functions are called from the main thread.

//--page-split-- mixer_generate

void mixer_generate(float *buffer, int frames) {

  easy_mutex_lock(&table_access_mutex);

  for (int i = 0; i < MAX_SOUNDS; i++) {
    if (!table[i].count) continue;
    float l, r, v = table[i].volume;
    if (table[i].has_location) {
      float dx, dy, dz, dd;
      dx = (table[i].location.x - player_position.x) * map_data.resolution.x;
      dy = (table[i].location.y - player_position.y) * map_data.resolution.x;
      dz = (table[i].location.z - player_position.z) * map_data.resolution.x;
      dd = dx * dx + dy * dy + dz * dz;
      if (dd < 10.0f) dd = 10.0f;
      v /= dd;
      if (v > 1.0f) v = 1.0f;
      float a = atan2(dy, dx) - player_position.u;
      while (a >= M_PI) a -= 2 * M_PI;
      while (a < -M_PI) a += 2 * M_PI;
      #if 0
      if (a > 0) {
        l = v;
        r = fabsf(cosf(+a)) * v;
      } else {
        l = fabsf(cos(-a)) * v;
        r = v;
      };
      #else
      a /= 0.5 * M_PI;
      if (a < -1) {
        l = -1.0f - a;
        r = 1.0f;
      } else if (a < 0) {
        l = 1.0f + a;
        r = 1.0f;
      } else if (a < 1) {
        l = 1.0f;
        r = 1.0f - a;
      } else {
        l = 1.0f;
        r = a - 1.0f;
      };
      l *= v;
      r *= v;
      #endif
    } else {
      if (v > 1.0f) v = 1.0f;
      l = r = v;
    };
    table[i].l = l;
    table[i].r = r;
  };

  for (int j = 0; j < frames; j++) {

    buffer[2 * j + 0] = 0;
    buffer[2 * j + 1] = 0;

    #if 0
    // create constant tone...
    static float a = 0.0f;
    buffer[2 * j + 0] =
    buffer[2 * j + 1] = 0.01f * sinf(a);
    a += 2 * M_PI * 440.0f / 32768;
    if (a > M_PI) a -= 2 * M_PI;
    #endif

    for (int i = 0; i < MAX_SOUNDS; i++) {
      if (!table[i].count) continue;
      buffer[2 * j + 0] += option_volume * table[i].l * *(table[i].pcm) / 32768.0f;
      buffer[2 * j + 1] += option_volume * table[i].r * *(table[i].pcm) / 32768.0f;
      table[i].pcm++; table[i].count--;
    };

  };

  easy_mutex_unlock(&table_access_mutex);

};

//--page-split-- mixer_play

void mixer_play(short *buffer, int count, float volume, struct double_xyz *location) {

  // calculate the volume of the sound at its present location

  float v = volume;
  if (location != NULL) {
    float dx, dy, dz, dd;
    dx = (location->x - player_position.x) * map_data.resolution.x;
    dy = (location->y - player_position.y) * map_data.resolution.x;
    dz = (location->z - player_position.z) * map_data.resolution.x;
    dd = dx * dx + dy * dy + dz * dz;
    if (dd < 10.0f) dd = 10.0f;
    v /= dd;
  };

  // find an empty slot, or if none are empty,
  // find one that is quieter than this sound.

  int slot = -1;
  float lowest = v;

  easy_mutex_lock(&table_access_mutex);

  for (int i = 0; i < MAX_SOUNDS; i++) {
    if (!table[i].count) {
      slot = i;
      break;
    } else if (table[i].volume < lowest) {
      slot = i;
      lowest = table[i].volume;
    };
  };

  // put the sound into that slot

  if (slot >= 0) {
    table[slot].pcm = buffer;
    table[slot].count = count;
    table[slot].volume = volume;
    if (location == NULL) {
      table[slot].has_location = 0;
    } else {
      table[slot].has_location = 1;
      table[slot].location = *location;
    };
  };

  easy_mutex_unlock(&table_access_mutex);

};
