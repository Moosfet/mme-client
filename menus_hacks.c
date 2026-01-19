#include "everything.h"

#ifdef TEST

//--page-split-- menus_hacks

void menus_hacks() {

  if (menu_process_event && KEY_PRESS_EVENT && KEY == GLFW_KEY_ESCAPE) menu_switch(menus_escape);
  if (gui_window(74, 18, 3)) menu_switch(menus_play);

  #if 0
  static int index;
  #define FILE_COUNT 1000
  const char *file_name[FILE_COUNT] = {
    //#include "/home/pj/trash/dynamic.include"
    #include "/home/pj/trash/hd.include"
  };

  static char s_index[16] = {};

  int calculate = 0;

  if (gui_input(2, 5, 4, 4, s_index, FLAGS) == 2) {
    index = strtod(s_index, NULL);
    if (index < 0) index = 0;
    if (index > FILE_COUNT - 1) index = FILE_COUNT - 1;
    snprintf(s_index, 5, "%d", index); s_index[4] = 0;
    calculate = 1;
  };

  if (gui_link(2, 7, "+")) {
    index = strtod(s_index, NULL);
    if (index < FILE_COUNT) index++;
    snprintf(s_index, 5, "%d", index); s_index[4] = 0;
    calculate = 1;
  };

  if (gui_link(4, 7, "-")) {
    index = strtod(s_index, NULL);
    if (index > 0) index--;
    snprintf(s_index, 5, "%d", index); s_index[4] = 0;
    calculate = 1;
  };

  if (index < 0) index = 0;
  if (index > FILE_COUNT - 1) index = FILE_COUNT - 1;

  static char compressed[15360000];
  static char buffer[15360000];

  if (calculate) {
    fprintf(stderr, "Loading \"%s\"...\n", file_name[index]);
    memset(map_data.block, 71, 23 * 2048 * 2048);
    memset(map_data.block + 23 * 2048 * 2048, 61, 1 * 2048 * 2048);
    memset(map_data.block + 24 * 2048 * 2048, 255, 72 * 2048 * 2048);
    FILE *file = fopen(file_name[index], "rb");
    printf("fopen returned %p\n", file);
    int size = fread(compressed, 1, 15360000, file);
    printf("fread returned %d\n", size);
    fclose(file);

        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.next_in = compressed;
        strm.avail_in = size;
        strm.next_out = buffer;
        strm.avail_out = 15360000;
        while (strm.avail_in != 0 && strm.avail_out != 0) {
          inflateInit2(&strm, 47);
          int result = inflate(&strm, Z_FINISH);
          inflateEnd(&strm);
          if (result < 0) {
            if (argument_packets) {
              printf("\e[1;31mzlib returned error %d after deflation\e[0m\n", result);
              printf("(avail_in=%d, avail_out=%d)\n", strm.avail_in, strm.avail_out);
            };
          };
        };
        if (strm.avail_out != 0 || strm.avail_in != 0) {
          if (argument_packets) {
            printf("\e[1;31marea load: Decompressed data size does not match map area size!\e[0m\n");
          };
        } else {
          for (int z = 0; z < 96; z++) {
            for (int y = 0; y < 400; y++) {
              for (int x = 0; x < 400; x++) {
                int s = (z * 400 + y) * 400 + x;
                int d = (z * 2048 + y + 824) * 2048 + x + 824;
                map_data.block[d] = buffer[s];
              };
            };
          };
          map_cease_render();
          map_begin_render();
          //map_invalidate_chunks();
        };

  };
  #endif

  gui_text(-1, 0, "Special Hacks");

  static int threads = -1;
  if (threads < 0) threads = map_chunk_thread_count;

  gui_text(28, 2, "Chunk Threads");
  if (
    gui_radio(30, 4, &threads, 0, "No threads!") +
    gui_radio(30, 5, &threads, 1, "1 thread") +
    gui_radio(30, 6, &threads, 2, "2 threads") +
    gui_radio(30, 7, &threads, 3, "3 threads") +
    gui_radio(30, 8, &threads, 4, "4 threads") +
    gui_radio(30, 9, &threads, 8, "8 threads") +
    gui_radio(30, 10, &threads, 16, "16 threads")
  ) {
    map_cease_render();
    map_chunk_thread_count = threads;
    map_begin_render();
  };

  gui_text(54, 2, "Change Chunk Size");
  char string[64];
  sprintf(string, "(is %d right now)", 1 << map_chunk_bits);
  gui_text(54, 3, string);
  if (gui_link(54, 5, "8 blocks/side")) {
    on_close_window();
    map_chunk_bits = 3;
    on_open_window();
  };
  if (gui_link(54, 6, "16 blocks/side")) {
    on_close_window();
    map_chunk_bits = 4;
    on_open_window();
  };
  if (gui_link(54, 7, "32 blocks/side")) {
    on_close_window();
    map_chunk_bits = 5;
    on_open_window();
  };
  if (gui_link(54, 8, "64 blocks/side")) {
    on_close_window();
    map_chunk_bits = 6;
    on_open_window();
  };

  if (!menu_display_data) {
    if (gui_link(54, 10, "Show Position Data")) menu_display_data = 1;
  } else {
    if (gui_link(54, 10, "Hide Position Data")) menu_display_data = 0;
  };

//  if (gui_check(4, 2, &map_enable_textures, 1, "Enable Textures")) {
//    map_cease_render();
//    map_begin_render();
//  };

//  if (gui_check(4, 3, &option_optimize_chunks, 1, "Optimize Chunks")) {
//    map_cease_render();
//    map_begin_render();
//  };

  #ifdef TEST
  if (argument_lag == 0 && on_activate_lag == 0) {
    if (gui_button(47, 14, -1, "Activate Lag Measurement!", 0)) {
      on_activate_lag = 1;
    };
  };
  #endif

  if (gui_button(63, 16, -1, "Hell yes!", 0)) {
    menu_switch(menus_play);
  };

  gui_text(0, 17, "This is the bottom-left of this menu!");
  gui_text(0, 0, "Top-left of menu!");
  gui_text(0, 1, "Second line...");
  gui_text(59, 17, "Bottom-right: X");

  char one[64] = "Field One";
  char two[64] = "Field Two";
  gui_input(2, 13, 24, 24, one, FLAGS);
  gui_input(2, 15, 24, 24, two, FLAGS);

};

#endif
