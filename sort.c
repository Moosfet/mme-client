#include "everything.h"

//--page-split-- sort_something

//  Applies a merge sort algorithm to some data.
//  The first four bytes of each record must be an int,
//  which is the value used to rank each record.

void sort_something(void *data, int record_size, int record_count) {
  if (record_count <= 1) return;
  int half_one = record_count >> 1;
  int half_two = record_count - half_one;
  sort_something(data, record_size, half_one);
  sort_something(data + half_one * record_size, record_size, half_two);
  char *temp = NULL;
  memory_allocate(&temp, record_count * record_size);
  char *pointer_one = data;
  char *pointer_two = data + half_one * record_size;
  char *pointer_three = temp;
  for (int i = 0; i < record_count; i++) {
    if (half_one && half_two) {
      #define RANK(x) (*((int *) (x)))
      if (RANK(pointer_one) < RANK(pointer_two)) {
        memmove(pointer_three, pointer_one, record_size);
        pointer_three += record_size;
        pointer_one += record_size;
        half_one--;
      } else {
        memmove(pointer_three, pointer_two, record_size);
        pointer_three += record_size;
        pointer_two += record_size;
        half_two--;
      };
    } else if (half_one) {
      memmove(pointer_three, pointer_one, record_size);
      pointer_three += record_size;
      pointer_one += record_size;
      half_one--;
    } else {
      memmove(pointer_three, pointer_two, record_size);
      pointer_three += record_size;
      pointer_two += record_size;
      half_two--;
    };
  };
  memmove(data, temp, record_size * record_count);
  memory_allocate(&temp, 0);
};
