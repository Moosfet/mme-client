#include "everything.h"

struct structure_sound_data sound_list[SOUND_MAX_SOUNDS];

//--page-split-- sound_reset

void sound_reset () {
  int a;
  for (a = 0; a < SOUND_MAX_SERVER_SOUNDS; a++) {

    memory_allocate(&sound_list[a].file, 0);
    //Don't use memory_free, since memory_allocate didn't allocate this..
    //memory_allocate (sound_list[a].data, 0);
    if (sound_list[a].data != NULL) free (sound_list[a].data);
    sound_list[a].data = NULL;
    sound_list[a].channels = 0;
    sound_list[a].samplerate = 0;
    sound_list[a].size = 0;

  }
}

//--page-split-- sound_initialize

void sound_initialize() {
  mixer_initialize(); // must be before alsa_initialize()
  #ifdef LINUX
  alsa_initialize();
  #endif
  #ifdef WINDOWS
  directsound_initialize();
  #endif

  int a;
  for (a = 0; a < SOUND_MAX_SOUNDS; a++) {
    sound_list[a].file = NULL;
    sound_list[a].data = NULL;
    sound_list[a].channels = 0;
    sound_list[a].samplerate = 0;
    sound_list[a].size = 0;
  };

  extern short data_button_press_ogg; extern int size_button_press_ogg;
  sound_list[SOUND_BUTTON_PRESS].samples = stb_vorbis_decode_memory((void *)&data_button_press_ogg, size_button_press_ogg, &sound_list[SOUND_BUTTON_PRESS].channels, &sound_list[SOUND_BUTTON_PRESS].samplerate, &sound_list[SOUND_BUTTON_PRESS].data);
  sound_list[SOUND_BUTTON_PRESS].size = sound_list[SOUND_BUTTON_PRESS].samples * sound_list[SOUND_BUTTON_PRESS].channels * 2;
  extern short data_chat_ogg; extern int size_chat_ogg;
  sound_list[SOUND_CHAT].samples = stb_vorbis_decode_memory((void *)&data_chat_ogg, size_chat_ogg, &sound_list[SOUND_CHAT].channels, &sound_list[SOUND_CHAT].samplerate, &sound_list[SOUND_CHAT].data);
  sound_list[SOUND_CHAT].size = sound_list[SOUND_CHAT].samples * sound_list[SOUND_CHAT].channels * 2;
  extern short data_collision_ogg; extern int size_collision_ogg;
  sound_list[SOUND_COLLISION].samples = stb_vorbis_decode_memory((void *)&data_collision_ogg, size_collision_ogg, &sound_list[SOUND_COLLISION].channels, &sound_list[SOUND_COLLISION].samplerate, &sound_list[SOUND_COLLISION].data);
  sound_list[SOUND_COLLISION].size = sound_list[SOUND_COLLISION].samples * sound_list[SOUND_COLLISION].channels * 2;
  extern short data_bounce_ogg; extern int size_bounce_ogg;
  sound_list[SOUND_BOUNCE].samples = stb_vorbis_decode_memory((void *)&data_bounce_ogg, size_bounce_ogg, &sound_list[SOUND_BOUNCE].channels, &sound_list[SOUND_BOUNCE].samplerate, &sound_list[SOUND_BOUNCE].data);
  sound_list[SOUND_BOUNCE].size = sound_list[SOUND_BOUNCE].samples * sound_list[SOUND_BOUNCE].channels * 2;
  extern short data_throw_ogg; extern int size_throw_ogg;
  sound_list[SOUND_THROW].samples = stb_vorbis_decode_memory((void *)&data_throw_ogg, size_throw_ogg, &sound_list[SOUND_THROW].channels, &sound_list[SOUND_THROW].samplerate, &sound_list[SOUND_THROW].data);
  sound_list[SOUND_THROW].size = sound_list[SOUND_THROW].samples * sound_list[SOUND_THROW].channels * 2;
  extern short data_block_place_ogg; extern int size_block_place_ogg;
  sound_list[SOUND_BLOCK_PLACE].samples = stb_vorbis_decode_memory((void *)&data_block_place_ogg, size_block_place_ogg, &sound_list[SOUND_BLOCK_PLACE].channels, &sound_list[SOUND_BLOCK_PLACE].samplerate, &sound_list[SOUND_BLOCK_PLACE].data);
  sound_list[SOUND_BLOCK_PLACE].size = sound_list[SOUND_BLOCK_PLACE].samples * sound_list[SOUND_BLOCK_PLACE].channels * 2;
  extern short data_block_remove_ogg; extern int size_block_remove_ogg;
  sound_list[SOUND_BLOCK_REMOVE].samples = stb_vorbis_decode_memory((void *)&data_block_remove_ogg, size_block_remove_ogg, &sound_list[SOUND_BLOCK_REMOVE].channels, &sound_list[SOUND_BLOCK_REMOVE].samplerate, &sound_list[SOUND_BLOCK_REMOVE].data);
  sound_list[SOUND_BLOCK_REMOVE].size = sound_list[SOUND_BLOCK_REMOVE].samples * sound_list[SOUND_BLOCK_REMOVE].channels * 2;
  extern short data_walk_ogg; extern int size_walk_ogg;
  sound_list[SOUND_WALK].samples = stb_vorbis_decode_memory((void *)&data_walk_ogg, size_walk_ogg, &sound_list[SOUND_WALK].channels, &sound_list[SOUND_WALK].samplerate, &sound_list[SOUND_WALK].data);
  sound_list[SOUND_WALK].size = sound_list[SOUND_WALK].samples * sound_list[SOUND_WALK].channels * 2;
};

//--page-split-- sound_terminate

void sound_terminate() {
  #ifdef LINUX
  alsa_terminate();
  #else
  //directsound_terminate();
  #endif
};

//--page-split-- sound_play

void sound_play (int sound_id, float volume, struct double_xyz *location) {
  if (!option_sound) return;
  if (sound_id < 0 || sound_id >= SOUND_MAX_SOUNDS) {
    printf ("sound_play: ID %i is outside the sound array boundries, you bitch!\n", sound_id);
    return;
  }

  if (sound_list[sound_id].data == NULL) {
    printf ("sound_play: ID %i is not allocated, you bitch!\n", sound_id);
    return;
  }

//  printf ("Play sound %i, size %i\n", sound_id, sound_list[sound_id].size);
  mixer_play(sound_list[sound_id].data, sound_list[sound_id].size >> 1, volume, location);
}
