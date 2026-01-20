#include "everything.h"

struct structure_block_data block_data[BLOCK_MAX_BLOCKS] = {};

//  struct structure_block_data {
//    char *comment;         // Text description of this block type.
//    int visible;           // Whether or not anything needs to be rendered.
//    int reverse;           // Draw reverse sides of transparent textures.
//    int impassable;        // Can the player walk into this block?
//    int transparent;       // For determining where textures render.
//    double transparency;   // For shading of sunlight.
//    int index[6];          // Index into texture_data[] for each side.
//  };

//--page-split-- block_reset

void block_reset(void) {
  for (int i = 0; i < BLOCK_MAX_BLOCKS; i++) {
    memory_allocate(&block_data[i].comment, 0);
    block_data[i].visible = 1;
    block_data[i].reverse = 0;
    block_data[i].impassable = 1;
    block_data[i].transparent = 0;

    block_data[i].emission = 0;
    block_data[i].index[BLOCK_SIDE_UP] = TEXTURE_SWEET_U;
    block_data[i].index[BLOCK_SIDE_FRONT] = TEXTURE_SWEET_S;
    block_data[i].index[BLOCK_SIDE_BACK] = TEXTURE_SWEET_S;
    block_data[i].index[BLOCK_SIDE_LEFT] = TEXTURE_SWEET_S;
    block_data[i].index[BLOCK_SIDE_RIGHT] = TEXTURE_SWEET_S;
    block_data[i].index[BLOCK_SIDE_DOWN] = TEXTURE_SWEET_D;
  };
  block_data[0].visible = 0;
  block_data[256].visible = 0;
};
