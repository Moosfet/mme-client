#include "everything.h"

static struct whatever {
  double time;
  char *string;
  char *colors;
} *array = NULL;

int chat_array_count = 0;

//--page-split-- chat_reset

void chat_reset() {
  for (int i = 0; i < chat_array_count; i++) {
    memory_allocate(&array[i].string, 0);
    memory_allocate(&array[i].colors, 0);
  };
  memory_allocate(&array, 0);
  chat_array_count = 0;
};

//--page-split-- chat_render

void chat_render(int x, int y, int width, int height, int flag, int scroll_offset) {

  // flag indicates full rendering, otherwise only last 10 seconds

  //if (!flag) height++;

  if (width < 1 || height < 1) return;

  if (chat_array_count == 0) return;

  if (chat_array_count > 10000) {
    memory_allocate(&array[0].string, 0);
    memory_allocate(&array[0].colors, 0);
    memmove(array, array + 1, (chat_array_count - 1) * sizeof(*array));
    memory_allocate(&array, (chat_array_count - 1) * sizeof(*array));
    chat_array_count--;
  };

  #define LINE0 (screen)
  #define LINE1 (screen+width+1)
  #define LINE9 (screen+(width+1)*(height-1))
  #define CODE0 (colors)
  #define CODE1 (colors+width+1)
  #define CODE9 (colors+(width+1)*(height-1))
  #define TOTAL ((width+1)*(height))

  unsigned char *screen = NULL;
  memory_allocate(&screen, TOTAL);
  memset(screen, 0, TOTAL);

  unsigned char *colors = NULL;
  memory_allocate(&colors, TOTAL);
  memset(colors, 6, TOTAL);

  double limit = on_frame_time - 10;
  for (int i = 0; i < chat_array_count - scroll_offset; i++) {
    if (flag || array[i].time > limit) {

      unsigned char *p = array[i].string;
      unsigned char *c = array[i].colors;
      while (strlen(p) > width) {
        int j, k;
        for (j = width; j >= 0; j--) {
          if (p[j] == ' ') break;
        };
        if (j > 0) {
          k = j;
          while (p[j+1] == ' ') j++;
          while (p[k-1] == ' ' && k > 0) k--;
          memmove(LINE0, LINE1, (width+1)*(height-1));
          memmove(CODE0, CODE1, (width+1)*(height-1));
          strncpy(LINE9, p, k); *(LINE9 + k) = 0;
          strncpy(CODE9, c, k); *(CODE9 + k) = 0;
          p += j + 1;
          c += j + 1;
        } else {
          memmove(LINE0, LINE1, (width+1)*(height-1));
          memmove(CODE0, CODE1, (width+1)*(height-1));
          strncpy(LINE9, p, width); *(LINE9 + width) = 0;
          strncpy(CODE9, c, width); *(CODE9 + width) = 0;
          p += width;
          c += width;
        };
      };
      if (strlen(p)) {
        memmove(LINE0, LINE1, (width+1)*(height-1));
        memmove(CODE0, CODE1, (width+1)*(height-1));
        strcpy(LINE9, p);
        strcpy(CODE9, c);
      };
    };
  };

  for (int i = 0; i < height; i++) {
    if (strlen(screen + i * (width + 1))) {
      gui_draw_text(x, y + i, screen + i * (width + 1), colors + i * (width + 1), 0);
    };
  };

  memory_allocate(&screen, 0);
  memory_allocate(&colors, 0);

};

//--page-split-- chat_color_decode

extern void chat_color_decode(char **string, char **colors, char *input, int strip_spaces);

void chat_color_decode(char **string, char **colors, char *input, int strip_spaces) {

  int size = strlen(input);

  memory_allocate(string, size + 1);
  memory_allocate(colors, size + 1);

  int color = 9;
  int length = 0;

  char *s = *string;
  char *c = *colors;

  int space_flag = 2;

  while (*input) {
    if (*input == 27) {
      input++;
      if (*input) {
        if (*input < 64) color = *input;
        input++;
      };
    } else {
      if (strip_spaces) {
        if (*input == 32) {
          space_flag++;
        } else {
          space_flag = 0;
        };
        if (space_flag > 2) {
          input++;
          continue;
        };
      };
      *s = *input;
      *c = color;
      s++;
      c++;
      length++;
      input++;
    };
  };

  *s = 0;
  *c = 0;

  memory_allocate(string, length + 1);
  memory_allocate(colors, length + 1);

};

int chat_color_cycle_index = 0;

//--page-split-- chat_message

void chat_message(char *message) {
  memory_allocate(&array, (chat_array_count + 1) * sizeof(*array));
  array[chat_array_count].time = on_frame_time;
  array[chat_array_count].string = NULL;
  array[chat_array_count].colors = NULL;
  chat_color_decode(&array[chat_array_count].string, &array[chat_array_count].colors, message, TRUE);
  chat_array_count++;

  // Look for debugging chat messages, write them to stdout...

  int special_chat = 0;
  int length = strlen(message);
  char plain[length + 1];
  char *i = message;
  char *o = plain;
  while (*i) {
    if (*i == 1) special_chat = 1;
    if (*i == 0x1B) {
      i++;
      if (*i == 0) break;
    } else if (*i >= 32 && *i <= 126) {
      *o = *i;
      o++;
    };
    i++;
  };
  *o = 0;
  if (special_chat) printf("\e[1;33m%s\e[0m\n", plain);
  sound_play(SOUND_CHAT, 0.2f, NULL);
};

//--page-split-- chat_color

void chat_color(int color) {

  if (color == 11) color = floor(2 + 12 * fmod(on_frame_time + chat_color_cycle_index / 12.0, 0.5));

  double c = (chat_color_cycle_index++ % 6) / 6.0;

  double i = 0.5 + 0.5 * cos(2 * M_PI * (on_frame_time + c));
  double j = 0.5 + 0.5 * sin(2 * M_PI * (on_frame_time + c));

  #define glColor4f(red, green, blue, alpha) { r = red; g = green; b = blue; a = alpha; }

  double r, g, b, a;

  switch(color) {

    case 1: glColor4f(0.6, 0.3, 0.2, 1.0); break; // brown
    case 2: glColor4f(1.0, 0.2, 0.2, 1.0); break; // red
    case 3: glColor4f(1.0, 0.5, 0.2, 1.0); break; // orange
    case 4: glColor4f(1.0, 1.0, 0.2, 1.0); break; // yellow
    case 5: glColor4f(0.2, 1.0, 0.2, 1.0); break; // green
    case 6: glColor4f(0.2, 0.6, 1.0, 1.0); break; // blue
    case 7: glColor4f(0.6, 0.2, 1.0, 1.0); break; // purple
    case 8: glColor4f(0.55, 0.55, 0.7, 1.0); break; // gray
    case 9: glColor4f(1.0, 1.0, 1.0, 1.0); break; // white
    case 10: glColor4f(0.2, 0.2, 0.2, 1.0); break; // black

    case 11: { // rainbow
      float r, g, b;
      float a = 4 * fmod(on_frame_time, 1.0);
      if (a < 1) r = 1, g = a, b = 0;
      else if (a < 2) r = 2 - a, g = 1, b = 0;
      else if (a < 2.5) r = 0, g = 1, b = 2 * (a - 2);
      else if (a < 3.0) r = 0, g = 2 * (3 - a), b = 1;
      else if (a < 3.5) r = 2 * (a - 3), g = 0, b = 1;
      else r = 1, g = 0, b = 2 * (4 - a);
      glColor4f(r, g, b, 1.0);
    }; break;

    case 12: glColor4f(1.0, 0.2, 0.6, 1.0); break; // magenta

    case 13: { // silver
      glColor4f(0.85 + 0.15 * i, 0.9 + 0.1 * i, 0.95 + 0.05 * j, 1.0);
    }; break;

    case 14: { // gold
      glColor4f(1.0, 0.85 + 0.15 * i, 0.2 * j, 1.0);
    }; break;

    case 15: { // ruby
      glColor4f(0.9 + 0.1 * j, 0.1 + 0.1 * i, 0.2 + 0.1 * j, 1.0);
    }; break;

    case 16: { // emerald
      glColor4f(0.1 + 0.1 * i, 0.9 + 0.1 * j, 0.2 + 0.1 * j, 1.0);
    }; break;

    default: glColor4f(1.0, 1.0, 1.0, 1.0); break;

  };

  if (option_anaglyph_enable) {
    double v = (r + g + b) / 3.0;
    r = g = b = v;
  };

  #undef glColor4f
  glColor4f(r, g, b, a);

};
