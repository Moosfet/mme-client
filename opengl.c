#include "everything.h"

//--page-split-- opengl_extentions

void opengl_extentions() {

  #ifdef TEST

  char *extention = (char *) glGetString(GL_EXTENSIONS);

  printf("Extentions: %s\n", extention);

  char *s = extention;
  char *t = extention;

  int i = 0;

  while (s != NULL) {
    while (*t != ' ' && *t != 0) t++;
    if (*t == ' ') {
      *t = 0;
      printf("OpenGL extention %d: %s\n", i++, s);
      s = t + 1;
    } else {
      printf("OpenGL extention %d: %s\n", i++, s);
      break;
    };
  };

  #endif

};
