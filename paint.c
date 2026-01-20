#include "everything.h"

#define PAINT_RATE 1000

static struct int_xyz paint_list_one[MAX_PAINT];
static struct int_xyz paint_list_two[MAX_PAINT];
static struct int_xyz (*new_list)[MAX_PAINT];
static int new_list_size;

struct int_xyz (*paint_list)[MAX_PAINT] = &paint_list_one;
int paint_list_size = 0;
int paint_target_type = 0;
int paint_with_type = 0;
double paint_start_time = 0.0;
int paint_total = 0;

//--page-split-- paint_reset

void paint_reset(void) {
  paint_list_size = 0;
};

//--page-split-- consider

static void consider(struct int_xyz c) {
  if (map_get_block_type(c) != paint_target_type) return;
  if (c.x < 0 || c.x >= map_data.dimension.x) return;
  if (c.y < 0 || c.y >= map_data.dimension.y) return;
  if (c.z < 0 || c.z >= map_data.dimension.z) return;

  // Require that at least one side of the block be exposed...

  struct int_xyz side[6] = {
    {-1, 0, 0},
    {+1, 0, 0},
    {0, -1, 0},
    {0, +1, 0},
    {0, 0, -1},
    {0, 0, +1},
  };

  int exposed = 0;
  for (int s = 0; s < 6; s++) {
    int t = map_get_block_type((struct int_xyz) {c.x + side[s].x, c.y + side[s].y, c.z + side[s].z});
    if (!block_data[t].visible) exposed = 1;
    if (block_data[t].transparent) exposed = 1;
  };
  if (!exposed) return;

  map_modify(c, paint_with_type, 0);
  server_modify(c, paint_with_type);
  paint_total++;
  (*new_list)[new_list_size++] = c;
};

//--page-split-- paint_something

void paint_something (void) {

  while (paint_list_size && paint_total < PAINT_RATE * (on_frame_time - paint_start_time)) {

    if (paint_list == &paint_list_one) {
      new_list = &paint_list_two;
    } else {
      new_list = &paint_list_one;
    };
    new_list_size = 0;

    for (int i = 0; i < paint_list_size; i++) {
      struct int_xyz c = (*paint_list)[i];

      c.x += 1; consider(c); c.x -= 2; consider(c); c.x += 1;
      c.y += 1; consider(c); c.y -= 2; consider(c); c.y += 1;
      c.z += 1; consider(c); c.z -= 2; consider(c); c.z += 1;
      if (MAX_PAINT - new_list_size < 6) {
        paint_list_size = 0; return;
      };
    };

    paint_list = new_list;
    paint_list_size = new_list_size;

    static int peak_usage = 0;
    if (new_list_size > peak_usage) {
      peak_usage = new_list_size;
      //printf("Peak paint list size: %d\n", peak_usage);
    };

  };

};
