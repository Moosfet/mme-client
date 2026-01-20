#include "everything.h"

struct structure_group_data group_data[256] = {};

//--page-split-- group_reset

void group_reset(void) {
  for (int i = 0; i < 256; i++) {
    memory_allocate(&group_data[i].name, 0);
    memory_allocate(&group_data[i].list, 0);
    group_data[i].size = 0;
  };
};
