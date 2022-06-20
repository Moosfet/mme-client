#include "everything.h"

//--page-split-- math_rotate_vector

void math_rotate_vector(struct double_xyzuv *vector) {
  struct double_xyz t;
  t.x = vector->x * cos(vector->v) - vector->z * sin(vector->v);
  t.z = vector->x * sin(vector->v) + vector->z * cos(vector->v);
  vector->x = t.x; vector->z = t.z; vector->v = 0;
  t.x = vector->x * cos(vector->u) - vector->y * sin(vector->u);
  t.y = vector->x * sin(vector->u) + vector->y * cos(vector->u);
  vector->x = t.x; vector->y = t.y; vector->u = 0;
};

//--page-split-- math_reverse_rotate_vector

void math_reverse_rotate_vector(struct double_xyzuv *vector) {
  struct double_xyz t;
  t.x = vector->x * cos(-vector->u) - vector->y * sin(-vector->u);
  t.y = vector->x * sin(-vector->u) + vector->y * cos(-vector->u);
  vector->x = t.x; vector->y = t.y; vector->u = 0;
  t.x = vector->x * cos(-vector->v) - vector->z * sin(-vector->v);
  t.z = vector->x * sin(-vector->v) + vector->z * cos(-vector->v);
  vector->x = t.x; vector->z = t.z; vector->v = 0;
};

//--page-split-- math_origin_vector

void math_origin_vector(struct double_xyzuv *vector) {
  if (vector->u != 0.0 || vector->v != 0.0) math_rotate_vector(vector);
  struct double_xyz t;
  vector->u = atan2(vector->y, vector->x);
  t.x = vector->x * cos(-vector->u) - vector->y * sin(-vector->u);
  t.y = vector->x * sin(-vector->u) + vector->y * cos(-vector->u);
  vector->x = t.x; vector->y = t.y;
  vector->v = atan2(vector->z, vector->x);
  t.x = vector->x * cos(-vector->v) - vector->z * sin(-vector->v);
  t.z = vector->x * sin(-vector->v) + vector->z * cos(-vector->v);
  vector->x = t.x; vector->z = t.z;
};

//--page-split-- math_normalize_vector

void math_normalize_vector(struct double_xyz *vector) {
  double length = sqrt(vector->x * vector->x + vector->y * vector->y + vector->z * vector->z);
  if (length >= 0.000001) {
    vector->x /= length;
    vector->y /= length;
    vector->z /= length;
  } else {
    vector->x = 0;
    vector->y = 0;
    vector->z = 0;
  };
};
