#include "everything.h"

//--page-split-- menus_fencepost

void menus_fencepost() {

  if (menu_process_event && KEY_PRESS_EVENT && KEY == GLFW_KEY_ESCAPE) menu_switch(menus_escape);
  if (gui_window(32, 23, 3)) menu_switch(menus_play);

  gui_text(-1, 0, "Fence Post Calculator");

  static char s_total[16] = {};
  static char s_post[16] = {};

  gui_text(1, 2, "Total Fence Length: ");
  gui_input(22, 2, 4, 4, s_total, FLAGS);

  gui_text(1, 4, "Fence Post Size: ");
  gui_input(22, 4, 4, 4, s_post, FLAGS);

  static int total;
  static int post;
  static int option[1000];
  static int count = 0;

  int calculate = 0;

  if (gui_link(28, 2, "+")) {
    total = strtod(s_total, NULL);
    if (total < 9999) total++;
    snprintf(s_total, 16, "%d", total); s_total[4] = 0;
    calculate = 1;
  };

  if (gui_link(30, 2, "-")) {
    total = strtod(s_total, NULL);
    if (total > 1) total--;
    snprintf(s_total, 16, "%d", total); s_total[4] = 0;
    calculate = 1;
  };

  if (gui_link(28, 4, "+")) {
    post = strtod(s_post, NULL);
    if (post < 9999) post++;
    snprintf(s_post, 16, "%d", post); s_post[4] = 0;
    calculate = 1;
  };

  if (gui_link(30, 4, "-")) {
    post = strtod(s_post, NULL);
    if (post > 1) post--;
    snprintf(s_post, 16, "%d", post); s_post[4] = 0;
    calculate = 1;
  };

  //if (gui_button(34, 3, -1, "Calculate space between posts!", FLAGS) || calculate) {
    total = strtod(s_total, NULL);
    if (total > 0) {
      snprintf(s_total, 16, "%d", total); s_total[4] = 0;
    } else {
      s_total[0] = 0;
    };
    post = strtod(s_post, NULL);
    if (post > 0) {
      snprintf(s_post, 16, "%d", post); s_post[4] = 0;
    } else {
      s_post[0] = 0;
    };
    if (total > post && post > 0) {
      count = 0;
      for (int i = 0; i < total; i++) {
        if ((total - post) % (i + post) == 0) {
          option[count++] = i;
          if (count == 1000) break;
        };
      };
    } else {
      count = 0;
    };
  //};

  #define MAX 32

  if (count) {
    int line = 6;
    char string[MAX + 1];
    int length = 0;

    //snprintf(string, MAX + 1, " Spacing options for %d length fence with %d wide posts: ", total, post); string[MAX + 1] = 0;
    snprintf(string, MAX + 1, " Spacing options: "); string[MAX + 1] = 0;
    //gui_text(1, line++, string);

    //string[0] = 0;
    length = strlen(string);

    for (int i = 0; i < count; i++) {
      char number[16];
      if (i < count - 1) {
        snprintf(number, 16, " %d,", option[i]); number[15] = 0;
      } else {
        snprintf(number, 16, " %d.", option[i]); number[15] = 0;
      };
      int l = strlen(number);
      if (l + length < MAX) {
        strcpy(string + length, number);
        length += l;
      } else {
        gui_text(0, line++, string);
        strcpy(string, number);
        length = l;
      };
    };
    if (length) {
      gui_text(0, line++, string);
    };
  };

};
