#include "everything.h"

int sky_box_flags = 0;
int sky_box_type = 0;
int sky_box_texture[6] = {};

//--page-split-- sky_reset

void sky_reset() {
  sky_box_flags = 0;
  sky_box_type = 0;
};
