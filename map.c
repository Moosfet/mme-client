#include "everything.h"

int map_chunk_bits = 5;

#define CHUNK_RING_SIZE 4  // buffered chunks per thread, 4 is all we need

struct structure_chunk_thread_data {
  int kill_switch;
  int read_index;
  int write_index;
  int chunk_index[CHUNK_RING_SIZE];
  struct structure_render_list *render_list[CHUNK_RING_SIZE];
  int list_size[CHUNK_RING_SIZE];
};

struct structure_render_list {
  int index;
  int side;
  struct int_xyz points[4];
  struct double_xy coords[2];
  double light;
};

double map_percentage_in_view = 0.0;
double map_perspective_angle = 45.0;
double map_crosshair_distance = 0.5;
struct structure_map_data map_data = {};
int map_cursor_color = -1;

#define map map_data
#define chunk_bits map_chunk_bits
#define chunk_size (1 << map_chunk_bits)

#define map_block(X, Y, Z) map_data.block[(X) + map_data.dimension.x * ((Y) + map_data.dimension.y * (Z))]
#define chunk_index(X, Y, Z) ((X) + chunk_dimension.x * ((Y) + chunk_dimension.y * (Z)))
#define chunk_map(X, Y, Z) chunk_map[chunk_index(X, Y, Z)]

#define CM_PRIORITY      0x0F  // Priority bits.
#define CM_VISIBLE       0x10  // Is presently visible on-screen.
#define CM_DIRTY         0x20  // Needs to be rebuilt for any reason.
#define CM_COMPLETE      0x40  // Has been built by a thread, is waiting to be compiled to display list.
#define CM_VIRGIN        0x80  // Has never been built.

// == chunk priorities
//
//  7 - contains block modified by user
//  6 - opposite block modified by user
//  5 - contains block modified by server
//  4 - opposite block modified by server
//  3 - lighting modified by user
//  2 - lighting modified by server
//  1 - packet_map_data
//  0 - everything else

int map_initialization_flag = 0;

static struct int_xyz chunk_dimension = {};
static int chunk_limit = 0;
static int chunk_list_base = 0;
static char *chunk_map = NULL;
static int *chunk_counts = NULL;
static int complete_chunks;
static int quad_count;

static struct structure_chunk_thread_data *chunk_thread_data;
volatile static char bullshit[1024];

// MUTEX chunk_map_mutex;
#ifdef WINDOWS
static HANDLE chunk_map_mutex;
#else
static pthread_mutex_t chunk_map_mutex;
#endif

int map_chunk_thread_count = 0;

// Table of specific area sizes to look for when joining individual block
// textures into larger quads.  A day of work went into selecting this list.
// It's a compromise between looking for all possibilities which makes chunk
// building painfully slow, and looking for none which is really fast but
// doesn't do anything to optimize rendering.  It searches only 1/4 of
// possible areas, but finds 4/5 of optimizations, if I remember correctly.
// I think it also does better than 1/4 of the time required to look for all
// possibilities, since some require more computation to look for than others.

#define area_count_a 1
static const struct int_st area_a[] = {{1, 1}};

#define area_count_b 1024
static const struct int_st area_b[] = {{32, 32}, {32, 31}, {31, 32}, {31, 31}, {32, 30}, {30, 32}, {31, 30}, {30, 31}, {32, 29}, {29, 32}, {30, 30}, {31, 29}, {29, 31}, {32, 28}, {28, 32}, {30, 29}, {29, 30}, {31, 28}, {28, 31}, {32, 27}, {27, 32}, {29, 29}, {30, 28}, {28, 30}, {31, 27}, {27, 31}, {32, 26}, {26, 32}, {29, 28}, {28, 29}, {30, 27}, {27, 30}, {31, 26}, {26, 31}, {32, 25}, {25, 32}, {28, 28}, {29, 27}, {27, 29}, {30, 26}, {26, 30}, {31, 25}, {25, 31}, {32, 24}, {24, 32}, {28, 27}, {27, 28}, {29, 26}, {26, 29}, {30, 25}, {25, 30}, {31, 24}, {24, 31}, {32, 23}, {23, 32}, {27, 27}, {28, 26}, {26, 28}, {29, 25}, {25, 29}, {30, 24}, {24, 30}, {31, 23}, {23, 31}, {32, 22}, {22, 32}, {27, 26}, {26, 27}, {28, 25}, {25, 28}, {29, 24}, {24, 29}, {30, 23}, {23, 30}, {31, 22}, {22, 31}, {26, 26}, {27, 25}, {25, 27}, {32, 21}, {28, 24}, {24, 28}, {21, 32}, {29, 23}, {23, 29}, {30, 22}, {22, 30}, {31, 21}, {21, 31}, {26, 25}, {25, 26}, {27, 24}, {24, 27}, {28, 23}, {23, 28}, {32, 20}, {20, 32}, {29, 22}, {22, 29}, {30, 21}, {21, 30}, {25, 25}, {26, 24}, {24, 26}, {27, 23}, {23, 27}, {31, 20}, {20, 31}, {28, 22}, {22, 28}, {29, 21}, {21, 29}, {32, 19}, {19, 32}, {30, 20}, {25, 24}, {24, 25}, {20, 30}, {26, 23}, {23, 26}, {27, 22}, {22, 27}, {31, 19}, {19, 31}, {28, 21}, {21, 28}, {29, 20}, {20, 29}, {32, 18}, {24, 24}, {18, 32}, {25, 23}, {23, 25}, {26, 22}, {22, 26}, {30, 19}, {19, 30}, {27, 21}, {21, 27}, {28, 20}, {20, 28}, {31, 18}, {18, 31}, {24, 23}, {23, 24}, {29, 19}, {19, 29}, {25, 22}, {22, 25}, {26, 21}, {21, 26}, {32, 17}, {17, 32}, {30, 18}, {27, 20}, {20, 27}, {18, 30}, {28, 19}, {19, 28}, {23, 23}, {24, 22}, {22, 24}, {31, 17}, {17, 31}, {25, 21}, {21, 25}, {29, 18}, {18, 29}, {26, 20}, {20, 26}, {27, 19}, {19, 27}, {32, 16}, {16, 32}, {30, 17}, {17, 30}, {23, 22}, {22, 23}, {28, 18}, {24, 21}, {21, 24}, {18, 28}, {25, 20}, {20, 25}, {31, 16}, {16, 31}, {26, 19}, {19, 26}, {29, 17}, {17, 29}, {27, 18}, {18, 27}, {22, 22}, {23, 21}, {21, 23}, {32, 15}, {30, 16}, {24, 20}, {20, 24}, {16, 30}, {15, 32}, {28, 17}, {17, 28}, {25, 19}, {19, 25}, {26, 18}, {18, 26}, {31, 15}, {15, 31}, {29, 16}, {16, 29}, {22, 21}, {21, 22}, {23, 20}, {20, 23}, {27, 17}, {17, 27}, {24, 19}, {19, 24}, {30, 15}, {25, 18}, {18, 25}, {15, 30}, {32, 14}, {28, 16}, {16, 28}, {14, 32}, {26, 17}, {17, 26}, {21, 21}, {22, 20}, {20, 22}, {23, 19}, {19, 23}, {29, 15}, {15, 29}, {31, 14}, {14, 31}, {27, 16}, {24, 18}, {18, 24}, {16, 27}, {25, 17}, {17, 25}, {30, 14}, {28, 15}, {21, 20}, {20, 21}, {15, 28}, {14, 30}, {22, 19}, {19, 22}, {32, 13}, {26, 16}, {16, 26}, {13, 32}, {23, 18}, {18, 23}, {24, 17}, {17, 24}, {29, 14}, {14, 29}, {27, 15}, {15, 27}, {31, 13}, {13, 31}, {25, 16}, {20, 20}, {16, 25}, {21, 19}, {19, 21}, {22, 18}, {18, 22}, {28, 14}, {14, 28}, {23, 17}, {17, 23}, {30, 13}, {26, 15}, {15, 26}, {13, 30}, {32, 12}, {24, 16}, {16, 24}, {12, 32}, {20, 19}, {19, 20}, {27, 14}, {21, 18}, {18, 21}, {14, 27}, {29, 13}, {13, 29}, {25, 15}, {15, 25}, {22, 17}, {17, 22}, {31, 12}, {12, 31}, {23, 16}, {16, 23}, {28, 13}, {26, 14}, {14, 26}, {13, 28}, {19, 19}, {30, 12}, {24, 15}, {20, 18}, {18, 20}, {15, 24}, {12, 30}, {21, 17}, {17, 21}, {32, 11}, {22, 16}, {16, 22}, {11, 32}, {27, 13}, {13, 27}, {25, 14}, {14, 25}, {29, 12}, {12, 29}, {23, 15}, {15, 23}, {19, 18}, {18, 19}, {31, 11}, {11, 31}, {20, 17}, {17, 20}, {26, 13}, {13, 26}, {28, 12}, {24, 14}, {21, 16}, {16, 21}, {14, 24}, {12, 28}, {30, 11}, {22, 15}, {15, 22}, {11, 30}, {25, 13}, {13, 25}, {27, 12}, {18, 18}, {12, 27}, {19, 17}, {17, 19}, {23, 14}, {14, 23}, {32, 10}, {20, 16}, {16, 20}, {10, 32}, {29, 11}, {11, 29}, {21, 15}, {15, 21}, {26, 12}, {24, 13}, {13, 24}, {12, 26}, {31, 10}, {10, 31}, {28, 11}, {22, 14}, {14, 22}, {11, 28}, {18, 17}, {17, 18}, {19, 16}, {16, 19}, {30, 10}, {25, 12}, {20, 15}, {15, 20}, {12, 25}, {10, 30}, {23, 13}, {13, 23}, {27, 11}, {11, 27}, {21, 14}, {14, 21}, {29, 10}, {10, 29}, {17, 17}, {32, 9}, {24, 12}, {18, 16}, {16, 18}, {12, 24}, {9, 32}, {26, 11}, {22, 13}, {13, 22}, {11, 26}, {19, 15}, {15, 19}, {28, 10}, {20, 14}, {14, 20}, {10, 28}, {31, 9}, {9, 31}, {23, 12}, {12, 23}, {25, 11}, {11, 25}, {21, 13}, {13, 21}, {17, 16}, {16, 17}, {30, 9}, {27, 10}, {18, 15}, {15, 18}, {10, 27}, {9, 30}, {19, 14}, {14, 19}, {24, 11}, {22, 12}, {12, 22}, {11, 24}, {29, 9}, {9, 29}, {26, 10}, {20, 13}, {13, 20}, {10, 26}, {32, 8}, {16, 16}, {8, 32}, {17, 15}, {15, 17}, {23, 11}, {11, 23}, {28, 9}, {21, 12}, {18, 14}, {14, 18}, {12, 21}, {9, 28}, {25, 10}, {10, 25}, {31, 8}, {8, 31}, {19, 13}, {13, 19}, {27, 9}, {9, 27}, {22, 11}, {11, 22}, {30, 8}, {24, 10}, {20, 12}, {16, 15}, {15, 16}, {12, 20}, {10, 24}, {8, 30}, {17, 14}, {14, 17}, {26, 9}, {18, 13}, {13, 18}, {9, 26}, {29, 8}, {8, 29}, {21, 11}, {11, 21}, {23, 10}, {10, 23}, {19, 12}, {12, 19}, {25, 9}, {15, 15}, {9, 25}, {32, 7}, {28, 8}, {16, 14}, {14, 16}, {8, 28}, {7, 32}, {17, 13}, {13, 17}, {22, 10}, {20, 11}, {11, 20}, {10, 22}, {31, 7}, {7, 31}, {27, 8}, {24, 9}, {18, 12}, {12, 18}, {9, 24}, {8, 27}, {30, 7}, {21, 10}, {15, 14}, {14, 15}, {10, 21}, {7, 30}, {19, 11}, {11, 19}, {26, 8}, {16, 13}, {13, 16}, {8, 26}, {23, 9}, {9, 23}, {17, 12}, {12, 17}, {29, 7}, {7, 29}, {25, 8}, {20, 10}, {10, 20}, {8, 25}, {22, 9}, {18, 11}, {11, 18}, {9, 22}, {28, 7}, {14, 14}, {7, 28}, {15, 13}, {13, 15}, {32, 6}, {24, 8}, {16, 12}, {12, 16}, {8, 24}, {6, 32}, {19, 10}, {10, 19}, {27, 7}, {21, 9}, {9, 21}, {7, 27}, {17, 11}, {11, 17}, {31, 6}, {6, 31}, {23, 8}, {8, 23}, {26, 7}, {14, 13}, {13, 14}, {7, 26}, {30, 6}, {20, 9}, {18, 10}, {15, 12}, {12, 15}, {10, 18}, {9, 20}, {6, 30}, {22, 8}, {16, 11}, {11, 16}, {8, 22}, {25, 7}, {7, 25}, {29, 6}, {6, 29}, {19, 9}, {9, 19}, {17, 10}, {10, 17}, {13, 13}, {28, 6}, {24, 7}, {21, 8}, {14, 12}, {12, 14}, {8, 21}, {7, 24}, {6, 28}, {15, 11}, {11, 15}, {27, 6}, {18, 9}, {9, 18}, {6, 27}, {23, 7}, {7, 23}, {32, 5}, {20, 8}, {16, 10}, {10, 16}, {8, 20}, {5, 32}, {26, 6}, {13, 12}, {12, 13}, {6, 26}, {31, 5}, {5, 31}, {22, 7}, {14, 11}, {11, 14}, {7, 22}, {17, 9}, {9, 17}, {19, 8}, {8, 19}, {30, 5}, {25, 6}, {15, 10}, {10, 15}, {6, 25}, {5, 30}, {21, 7}, {7, 21}, {29, 5}, {5, 29}, {24, 6}, {18, 8}, {16, 9}, {12, 12}, {9, 16}, {8, 18}, {6, 24}, {13, 11}, {11, 13}, {28, 5}, {20, 7}, {14, 10}, {10, 14}, {7, 20}, {5, 28}, {23, 6}, {6, 23}, {17, 8}, {8, 17}, {27, 5}, {15, 9}, {9, 15}, {5, 27}, {19, 7}, {7, 19}, {22, 6}, {12, 11}, {11, 12}, {6, 22}, {26, 5}, {13, 10}, {10, 13}, {5, 26}, {32, 4}, {16, 8}, {8, 16}, {4, 32}, {21, 6}, {18, 7}, {14, 9}, {9, 14}, {7, 18}, {6, 21}, {25, 5}, {5, 25}, {31, 4}, {4, 31}, {11, 11}, {30, 4}, {24, 5}, {20, 6}, {15, 8}, {12, 10}, {10, 12}, {8, 15}, {6, 20}, {5, 24}, {4, 30}, {17, 7}, {7, 17}, {13, 9}, {9, 13}, {29, 4}, {4, 29}, {23, 5}, {5, 23}, {19, 6}, {6, 19}, {28, 4}, {16, 7}, {14, 8}, {8, 14}, {7, 16}, {4, 28}, {22, 5}, {11, 10}, {10, 11}, {5, 22}, {27, 4}, {18, 6}, {12, 9}, {9, 12}, {6, 18}, {4, 27}, {21, 5}, {15, 7}, {7, 15}, {5, 21}, {26, 4}, {13, 8}, {8, 13}, {4, 26}, {17, 6}, {6, 17}, {25, 4}, {20, 5}, {10, 10}, {5, 20}, {4, 25}, {11, 9}, {9, 11}, {14, 7}, {7, 14}, {32, 3}, {24, 4}, {16, 6}, {12, 8}, {8, 12}, {6, 16}, {4, 24}, {3, 32}, {19, 5}, {5, 19}, {31, 3}, {3, 31}, {23, 4}, {4, 23}, {13, 7}, {7, 13}, {30, 3}, {18, 5}, {15, 6}, {10, 9}, {9, 10}, {6, 15}, {5, 18}, {3, 30}, {22, 4}, {11, 8}, {8, 11}, {4, 22}, {29, 3}, {3, 29}, {17, 5}, {5, 17}, {28, 3}, {21, 4}, {14, 6}, {12, 7}, {7, 12}, {6, 14}, {4, 21}, {3, 28}, {27, 3}, {9, 9}, {3, 27}, {20, 4}, {16, 5}, {10, 8}, {8, 10}, {5, 16}, {4, 20}, {26, 3}, {13, 6}, {6, 13}, {3, 26}, {11, 7}, {7, 11}, {19, 4}, {4, 19}, {25, 3}, {15, 5}, {5, 15}, {3, 25}, {24, 3}, {18, 4}, {12, 6}, {9, 8}, {8, 9}, {6, 12}, {4, 18}, {3, 24}, {14, 5}, {10, 7}, {7, 10}, {5, 14}, {23, 3}, {3, 23}, {17, 4}, {4, 17}, {22, 3}, {11, 6}, {6, 11}, {3, 22}, {13, 5}, {5, 13}, {32, 2}, {16, 4}, {8, 8}, {4, 16}, {2, 32}, {21, 3}, {9, 7}, {7, 9}, {3, 21}, {31, 2}, {2, 31}, {30, 2}, {20, 3}, {15, 4}, {12, 5}, {10, 6}, {6, 10}, {5, 12}, {4, 15}, {3, 20}, {2, 30}, {29, 2}, {2, 29}, {19, 3}, {3, 19}, {28, 2}, {14, 4}, {8, 7}, {7, 8}, {4, 14}, {2, 28}, {11, 5}, {5, 11}, {27, 2}, {18, 3}, {9, 6}, {6, 9}, {3, 18}, {2, 27}, {26, 2}, {13, 4}, {4, 13}, {2, 26}, {17, 3}, {3, 17}, {25, 2}, {10, 5}, {5, 10}, {2, 25}, {7, 7}, {24, 2}, {16, 3}, {12, 4}, {8, 6}, {6, 8}, {4, 12}, {3, 16}, {2, 24}, {23, 2}, {2, 23}, {15, 3}, {9, 5}, {5, 9}, {3, 15}, {22, 2}, {11, 4}, {4, 11}, {2, 22}, {21, 2}, {14, 3}, {7, 6}, {6, 7}, {3, 14}, {2, 21}, {20, 2}, {10, 4}, {8, 5}, {5, 8}, {4, 10}, {2, 20}, {13, 3}, {3, 13}, {19, 2}, {2, 19}, {18, 2}, {12, 3}, {9, 4}, {6, 6}, {4, 9}, {3, 12}, {2, 18}, {7, 5}, {5, 7}, {17, 2}, {2, 17}, {11, 3}, {3, 11}, {32, 1}, {16, 2}, {8, 4}, {4, 8}, {2, 16}, {1, 32}, {31, 1}, {1, 31}, {30, 1}, {15, 2}, {10, 3}, {6, 5}, {5, 6}, {3, 10}, {2, 15}, {1, 30}, {29, 1}, {1, 29}, {28, 1}, {14, 2}, {7, 4}, {4, 7}, {2, 14}, {1, 28}, {27, 1}, {9, 3}, {3, 9}, {1, 27}, {26, 1}, {13, 2}, {2, 13}, {1, 26}, {25, 1}, {5, 5}, {1, 25}, {24, 1}, {12, 2}, {8, 3}, {6, 4}, {4, 6}, {3, 8}, {2, 12}, {1, 24}, {23, 1}, {1, 23}, {22, 1}, {11, 2}, {2, 11}, {1, 22}, {21, 1}, {7, 3}, {3, 7}, {1, 21}, {20, 1}, {10, 2}, {5, 4}, {4, 5}, {2, 10}, {1, 20}, {19, 1}, {1, 19}, {18, 1}, {9, 2}, {6, 3}, {3, 6}, {2, 9}, {1, 18}, {17, 1}, {1, 17}, {16, 1}, {8, 2}, {4, 4}, {2, 8}, {1, 16}, {15, 1}, {5, 3}, {3, 5}, {1, 15}, {14, 1}, {7, 2}, {2, 7}, {1, 14}, {13, 1}, {1, 13}, {12, 1}, {6, 2}, {4, 3}, {3, 4}, {2, 6}, {1, 12}, {11, 1}, {1, 11}, {10, 1}, {5, 2}, {2, 5}, {1, 10}, {9, 1}, {3, 3}, {1, 9}, {8, 1}, {4, 2}, {2, 4}, {1, 8}, {7, 1}, {1, 7}, {6, 1}, {3, 2}, {2, 3}, {1, 6}, {5, 1}, {1, 5}, {4, 1}, {2, 2}, {1, 4}, {3, 1}, {1, 3}, {2, 1}, {1, 2}, {1, 1}};

#define area_count_c 64
static const struct int_st area_c[] = {{32, 32}, {32, 8}, {8, 32}, {32, 4}, {32, 3}, {3, 32}, {32, 2}, {2, 32}, {3, 12}, {32, 1}, {1, 32}, {1, 25}, {3, 8}, {5, 4}, {18, 1}, {3, 6}, {17, 1}, {1, 17}, {16, 1}, {8, 2}, {4, 4}, {1, 16}, {15, 1}, {5, 3}, {1, 15}, {14, 1}, {2, 7}, {1, 14}, {13, 1}, {1, 13}, {12, 1}, {6, 2}, {4, 3}, {3, 4}, {1, 12}, {11, 1}, {1, 11}, {10, 1}, {5, 2}, {2, 5}, {1, 10}, {9, 1}, {3, 3}, {1, 9}, {8, 1}, {4, 2}, {2, 4}, {1, 8}, {7, 1}, {1, 7}, {6, 1}, {3, 2}, {2, 3}, {1, 6}, {5, 1}, {1, 5}, {4, 1}, {2, 2}, {1, 4}, {3, 1}, {1, 3}, {2, 1}, {1, 2}, {1, 1}};

#ifdef TEST
static int area_tally[1024] = {};
#endif

static int area_count;
static const struct int_st *area;

//--page-split-- map_initialize

void map_initialize() {
  MUTEX_INIT(chunk_map_mutex);
};

//--page-split-- map_open_window

void map_open_window() {
  map_begin_render();
};

//--page-split-- map_close_window

void map_close_window() {
  map_cease_render();
};

//--page-split-- map_begin_render

void map_begin_render() {
  if (!map_data.block) return;
  if (map_initialization_flag) map_cease_render();
  DEBUG("enter map_initialize()");
  chunk_dimension.x = (map.dimension.x + chunk_size - 1) >> chunk_bits;
  chunk_dimension.y = (map.dimension.y + chunk_size - 1) >> chunk_bits;
  chunk_dimension.z = (map.dimension.z + chunk_size - 1) >> chunk_bits;
  chunk_limit = 2 * chunk_dimension.x * chunk_dimension.y * chunk_dimension.z;
  chunk_list_base = glGenLists(chunk_limit);
  memory_allocate(&chunk_map, sizeof(char) * chunk_limit);
  memset(chunk_map, CM_DIRTY | CM_VIRGIN, sizeof(char) * chunk_limit);
  memory_allocate(&chunk_counts, sizeof(int) * chunk_limit);
  memset(chunk_counts, 0, sizeof(int) * chunk_limit);
  complete_chunks = 0;
  quad_count = 0;

  area = area_c;
  area_count = area_count_c;

  // Start chunk threads...

  if (argument_no_threads) map_chunk_thread_count = 0;
  memory_allocate(&chunk_thread_data, sizeof(struct structure_chunk_thread_data) * map_chunk_thread_count);
  memset(chunk_thread_data, 0, sizeof(struct structure_chunk_thread_data) * map_chunk_thread_count);
  for (int i = 0; i < map_chunk_thread_count; i++) {
    thread_create(map_chunk_thread, (void *) &chunk_thread_data[i]);
  };

  map_initialization_flag = 1;

  DEBUG("leave map_initialize()");
};

//--page-split-- map_cease_render

void map_cease_render() {
  if (!map_initialization_flag) return;
  DEBUG("enter map_terminate()");

  // Stop chunk threads...
  for (int i = 0; i < map_chunk_thread_count; i++) {
    chunk_thread_data[i].kill_switch = 1;
  };
  for (int i = 0; i < map_chunk_thread_count; i++) {
    // Each thread sets kill_switch back to zero.
    while (chunk_thread_data[i].kill_switch) easy_sleep(0.01);
  };
  if (map_chunk_thread_count) while (map_process_a_chunk());
  memory_allocate(&chunk_thread_data, 0);

  glDeleteLists(chunk_list_base, chunk_limit);
  memory_allocate(&chunk_map, 0);
  memory_allocate(&chunk_counts, 0);

  map_initialization_flag = 0;

  #ifdef TEST
  if (0) {
    struct {
      int count;
      int index;
    } sort[1024];
    for (int i = 0; i < area_count; i++) {
      sort[i].index = i;
      sort[i].count = area_tally[i];
    };
    sort_something(sort, 8, area_count);
    for (int i = 0; i < area_count; i++) {
      int j = sort[i].index;
      printf("%d: %d, %d x %d\n", i, sort[i].count, area[sort[i].index].s, area[sort[i].index].t);
    };
  };
  #endif

  DEBUG("leave map_terminate()");
};

//--page-split-- map_invalidate_chunks

void map_invalidate_chunks() {
  if (!map_initialization_flag) return;
  for (int i = 0; i < chunk_limit; i++) chunk_map[i] = CM_DIRTY | CM_VIRGIN;
  complete_chunks = 0;
};

//--page-split-- map_is_active

int map_is_active() {
  return map_initialization_flag;
};

//--page-split-- draw_inside

static void draw_inside(struct int_xyz c, int s) {
  if (s == BLOCK_SIDE_UP) {
    glVertex3f(c.x, c.y + 1, c.z + 1);
    glVertex3f(c.x + 1, c.y + 1, c.z + 1);
    glVertex3f(c.x + 1, c.y, c.z + 1);
    glVertex3f(c.x, c.y, c.z + 1);
  };
  if (s == BLOCK_SIDE_FRONT) {
    glVertex3f(c.x + 1, c.y, c.z);
    glVertex3f(c.x, c.y, c.z);
    glVertex3f(c.x, c.y, c.z + 1);
    glVertex3f(c.x + 1, c.y, c.z + 1);
  };
  if (s == BLOCK_SIDE_RIGHT) {
    glVertex3f(c.x + 1, c.y + 1, c.z);
    glVertex3f(c.x + 1, c.y, c.z);
    glVertex3f(c.x + 1, c.y, c.z + 1);
    glVertex3f(c.x + 1, c.y + 1, c.z + 1);
  };
  if (s == BLOCK_SIDE_LEFT) {
    glVertex3f(c.x, c.y, c.z);
    glVertex3f(c.x, c.y + 1, c.z);
    glVertex3f(c.x, c.y + 1, c.z + 1);
    glVertex3f(c.x, c.y, c.z + 1);
  };
  if (s == BLOCK_SIDE_BACK) {
    glVertex3f(c.x, c.y + 1, c.z);
    glVertex3f(c.x + 1, c.y + 1, c.z);
    glVertex3f(c.x + 1, c.y + 1, c.z + 1);
    glVertex3f(c.x, c.y + 1, c.z + 1);
  };
  if (s == BLOCK_SIDE_DOWN) {
    glVertex3f(c.x, c.y, c.z);
    glVertex3f(c.x + 1, c.y, c.z);
    glVertex3f(c.x + 1, c.y + 1, c.z);
    glVertex3f(c.x, c.y + 1, c.z);
  };
};

//--page-split-- draw_cursor

static void draw_cursor() {

  if (!map_selection.valid || map_cursor_color < 0) return;

  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);

  struct int_xyz block = map_selection.destroy;

  // Draw a colorful highlight, for visibility.
  if (glfw_mouse_capture_flag) {
    chat_color(11);
    //glColor4f(0.0, 0.0, 0.0, 0.8);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(4);
    glBegin(GL_LINE_LOOP);
    draw_inside(block, map_selection.side);
    glEnd();
  };

  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);

};

int map_selection_color = -1;
struct int_xyz map_selection_corner_one;
struct int_xyz map_selection_corner_two;
struct int_xyz map_selection_dimensions;

//--page-split-- draw_selection

static void draw_selection() {

  if (map_selection_color < 0) return;

  struct int_xyz one = map_selection_corner_one;
  struct int_xyz two = map_selection_corner_two;

  // Swap the coordinates so that "one" comes before "two".
  if (one.x > two.x) { int temp = one.x; one.x = two.x; two.x = temp; };
  if (one.y > two.y) { int temp = one.y; one.y = two.y; two.y = temp; };
  if (one.z > two.z) { int temp = one.z; one.z = two.z; two.z = temp; };

  // Draw to the far end of the second block.
  two.x++; two.y++; two.z++;

  struct int_xyz size;
  size.x = two.x - one.x;
  size.y = two.y - one.y;
  size.z = two.z - one.z;
  map_selection_dimensions = size;

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GEQUAL, 0.2);
  glEnable(GL_CULL_FACE);

  chat_color(map_selection_color);

  for (int s = 0; s < 2; s++) {

    if (s == 0) {
      glBindTexture(GL_TEXTURE_2D, texture_half_screen + RENDER_IN_GRAYSCALE);
      glFrontFace(GL_CW);
    } else {
      glBindTexture(GL_TEXTURE_2D, texture_full_screen + RENDER_IN_GRAYSCALE);
      glFrontFace(GL_CCW);
    };

    glBegin(GL_QUADS);

    glTexCoord2f(one.x, one.y); glVertex3f(one.x, one.y, one.z);
    glTexCoord2f(one.x, two.y); glVertex3f(one.x, two.y, one.z);
    glTexCoord2f(two.x, two.y); glVertex3f(two.x, two.y, one.z);
    glTexCoord2f(two.x, one.y); glVertex3f(two.x, one.y, one.z);

    glTexCoord2f(one.x, one.y); glVertex3f(one.x, one.y, two.z);
    glTexCoord2f(two.x, one.y); glVertex3f(two.x, one.y, two.z);
    glTexCoord2f(two.x, two.y); glVertex3f(two.x, two.y, two.z);
    glTexCoord2f(one.x, two.y); glVertex3f(one.x, two.y, two.z);

    glTexCoord2f(one.x, one.z); glVertex3f(one.x, one.y, one.z);
    glTexCoord2f(two.x, one.z); glVertex3f(two.x, one.y, one.z);
    glTexCoord2f(two.x, two.z); glVertex3f(two.x, one.y, two.z);
    glTexCoord2f(one.x, two.z); glVertex3f(one.x, one.y, two.z);

    glTexCoord2f(one.x, one.z); glVertex3f(one.x, two.y, one.z);
    glTexCoord2f(one.x, two.z); glVertex3f(one.x, two.y, two.z);
    glTexCoord2f(two.x, two.z); glVertex3f(two.x, two.y, two.z);
    glTexCoord2f(two.x, one.z); glVertex3f(two.x, two.y, one.z);

    glTexCoord2f(one.y, one.z); glVertex3f(one.x, one.y, one.z);
    glTexCoord2f(one.y, two.z); glVertex3f(one.x, one.y, two.z);
    glTexCoord2f(two.y, two.z); glVertex3f(one.x, two.y, two.z);
    glTexCoord2f(two.y, one.z); glVertex3f(one.x, two.y, one.z);

    glTexCoord2f(one.y, one.z); glVertex3f(two.x, one.y, one.z);
    glTexCoord2f(two.y, one.z); glVertex3f(two.x, two.y, one.z);
    glTexCoord2f(two.y, two.z); glVertex3f(two.x, two.y, two.z);
    glTexCoord2f(one.y, two.z); glVertex3f(two.x, one.y, two.z);

    glEnd();

  };

  glDisable(GL_CULL_FACE);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_TEXTURE_2D);

};

//--page-split-- use_map_coordinates

//  double v_fac = tan(0.5 * map_perspective_angle * M_PI / 180);
//  double h_fac = v_fac * display_window_width / display_window_height;

static void use_map_coordinates(int eye) {

  if (option_perspective_angle < 30) option_perspective_angle = 30;
  if (option_perspective_angle > 120) option_perspective_angle = 120;

  int use_incorrect_perspective = 0;

  if (!option_anaglyph_enable) {
    use_incorrect_perspective = 1;
  } else if (option_anaglyph_pixel_size <= 0) {
    use_incorrect_perspective = 1;
  } else if (option_anaglyph_distance <= 0) {
    use_incorrect_perspective = 1;
  } else {
    map_perspective_angle = 2.0 * atan(0.5 * display_window_height * option_anaglyph_pixel_size / option_anaglyph_distance) * 180 / M_PI;
    //printf("dts=%0.1f  sop=%0.3f  vs=%0.1f  angle=%0.3f\n", option_anaglyph_distance, option_anaglyph_pixel_size, display_window_height * option_anaglyph_pixel_size, map_perspective_angle);
  };

  {
    double v_factor = tan(0.5 * map_perspective_angle * M_PI / 180);
    double h_factor = v_factor * display_window_width / display_window_height;
    //printf("h_factor = %f  v_factor = %f\n", h_factor, v_factor);
    if (h_factor > 10.0 || h_factor < 0.05 || v_factor > 10.0 || h_factor < 0.05) {
      option_anaglyph_enable = 0;
      use_incorrect_perspective = 1;
    };
  };

  if (use_incorrect_perspective) {
    if (display_window_width > display_window_height) {
      map_perspective_angle = option_perspective_angle;
    } else {
      double h_factor = tan(0.5 * option_perspective_angle * M_PI / 180);
      double v_factor = h_factor * display_window_height / display_window_width;
      map_perspective_angle = 2.0 * atan(v_factor) * 180 / M_PI;
    };
  };

  // Set window coordinates to map coordinates.
  glMatrixMode(GL_PROJECTION); glLoadIdentity();

  if (0) {
    // temporal antialiasing
    double px = 1.0 / display_window_width;
    double py = 1.0 / display_window_height;
    double rx = easy_random(256) / 256.0 - 0.5;
    double ry = easy_random(256) / 256.0 - 0.5;
    glTranslated(px * rx, py * ry, 0.0);
  };

  if (option_anaglyph_enable) {
    // The window coordinates here, left to right, is -1.0 to +1.0
    double scale = 0.01;
    if (option_anaglyph_units) scale /= 2.54;
    double pupil_offset = 0.5 * scale * option_pupil_distance;
    double full_offset = 0.5 * scale * option_anaglyph_pixel_size * display_window_width;
    double offset = pupil_offset / full_offset;
    if (eye == 1) {
      glTranslated(-offset, 0.0, 0.0);
    } else if (eye == 2) {
      glTranslated(+offset, 0.0, 0.0);
    };
  };

  gluPerspective(map_perspective_angle, display_window_height != 0 ? (float) display_window_width / display_window_height : display_window_width, 1.0/16, 4096);

  glMatrixMode(GL_MODELVIEW); glLoadIdentity();

  if (option_anaglyph_enable) {
    // here we're in units of meters
    double scale = 0.01;
    if (option_anaglyph_units) scale /= 2.54;
    double offset = 0.5 * scale * option_pupil_distance;
    double center = 0.5 * display_window_width;
    if (eye == 1) {
      if (option_anaglyph_enable == 2) {
        glScissor(0, 0, 0.5 * display_window_width, display_window_height);
        glEnable(GL_SCISSOR_TEST);
      };
      glTranslated(+offset, 0.0, 0.0);
    } else if (eye == 2) {
      if (option_anaglyph_enable == 2) {
        glScissor(0.5 * display_window_width, 0, display_window_width, display_window_height);
        glEnable(GL_SCISSOR_TEST);
      };
      glTranslated(-offset, 0.0, 0.0);
    };
  };

  glRotated(-90, 1, 0, 0); glRotated(90, 0, 0, 1);

  // Rotate camera to player position...
  glRotated(-(player_view.v) * 180 / M_PI, 0, -1, 0);
  glRotated(-(player_view.u) * 180 / M_PI, 0, 0, +1);

  // Scale map so that one unit = one block.
  glScaled(map_data.resolution.x, map_data.resolution.y, map_data.resolution.z);

  // Move camera to player position...
  glTranslated(-player_view.x, -player_view.y, -player_view.z);

};

//--page-split-- map_modify_bunch

void map_modify_bunch(struct int_xyz one, struct int_xyz two, int t, int priority) {

  struct double_xyzuv pp = player_view;
  if (map_data.wrap.x && pp.x >= map_data.dimension.x / 2) pp.x -= map_data.dimension.x;
  if (map_data.wrap.y && pp.y >= map_data.dimension.y / 2) pp.y -= map_data.dimension.y;
  if (map_data.wrap.z && pp.z >= map_data.dimension.z / 2) pp.z -= map_data.dimension.z;

  // Arrange second point (which is always near the player) so it is near the player.
  if (map_data.wrap.x && two.x - pp.x >= map_data.dimension.x / 2) two.x -= map_data.dimension.x;
  if (map_data.wrap.y && two.y - pp.y >= map_data.dimension.y / 2) two.y -= map_data.dimension.y;
  if (map_data.wrap.z && two.z - pp.z >= map_data.dimension.z / 2) two.z -= map_data.dimension.z;

  // Wrap the "one" coordinates to come before the "two" coordinates.
  while (map_data.wrap.x && one.x > two.x) one.x -= map_data.dimension.x;
  while (map_data.wrap.y && one.y > two.y) one.y -= map_data.dimension.y;
  while (map_data.wrap.z && one.z > two.z) one.z -= map_data.dimension.z;

  // If resulting box is too large, unwrap the coordinates just once.
  if (map_data.wrap.x && two.x - one.x >= map_data.dimension.x / 2) one.x += map_data.dimension.x;
  if (map_data.wrap.y && two.y - one.y >= map_data.dimension.y / 2) one.y += map_data.dimension.y;
  if (map_data.wrap.z && two.z - one.z >= map_data.dimension.z / 2) one.z += map_data.dimension.z;

  // Then swap the coordinates so that "one" comes before "two".
  if (one.x > two.x) { int temp = one.x; one.x = two.x; two.x = temp; };
  if (one.y > two.y) { int temp = one.y; one.y = two.y; two.y = temp; };
  if (one.z > two.z) { int temp = one.z; one.z = two.z; two.z = temp; };

  struct int_xyz i, j;
  for (i.z = one.z; i.z <= two.z; i.z++) {
    for (i.y = one.y; i.y <= two.y; i.y++) {
      for (i.x = one.x; i.x <= two.x; i.x++) {
        j = i;
        if (j.x < 0) j.x += map_data.dimension.x;
        if (j.y < 0) j.y += map_data.dimension.y;
        if (j.z < 0) j.z += map_data.dimension.z;
        map_modify(j, t, priority);
      };
    };
  };
};

static const int priority_matrix[4][4] = {
  {15, 14, 9, 6},
  {13, 12, 8, 5},
  {11, 10, 7, 4},
  {3, 2, 1, 0},
};

//--page-split-- invalidate_chunk

static void invalidate_chunk(int x, int y, int z, int priority) {
  if (map_data.wrap.x) {
    if (x < 0) x += map_data.dimension.x;
    if (x >= map_data.dimension.x) x -= map_data.dimension.x;
  };
  if (map_data.wrap.y) {
    if (y < 0) y += map_data.dimension.y;
    if (y >= map_data.dimension.y) y -= map_data.dimension.y;
  };
  if (map_data.wrap.z) {
    if (z < 0) z += map_data.dimension.z;
    if (z >= map_data.dimension.z) z -= map_data.dimension.z;
  };
  if (x < 0 || x >= map_data.dimension.x) return;
  if (y < 0 || y >= map_data.dimension.y) return;
  if (z < 0 || z >= map_data.dimension.z) return;
  volatile unsigned int old = chunk_map(x >> chunk_bits, y >> chunk_bits, z >> chunk_bits);
  volatile unsigned int new = old | CM_DIRTY;
  if ((old & CM_PRIORITY) < priority) {
    new &= ~CM_PRIORITY;
    new |= priority;
  };
  //printf("(%d, %d, %d) %02x: %02x to %02x\n", x, y, z, priority, old & 15, new & 15);
  chunk_map(x >> chunk_bits, y >> chunk_bits, z >> chunk_bits) = new;
};

//--page-split-- map_modify

void map_modify(struct int_xyz block, int t, int priority) {
  #define priority(x) priority_matrix[priority][x]
  if (map_data.wrap.x) {
    if (block.x < 0) block.x += map_data.dimension.x;
    if (block.x >= map_data.dimension.x) block.x -= map_data.dimension.x;
  };
  if (map_data.wrap.y) {
    if (block.y < 0) block.y += map_data.dimension.y;
    if (block.y >= map_data.dimension.y) block.y -= map_data.dimension.y;
  };
  if (map_data.wrap.z) {
    if (block.z < 0) block.z += map_data.dimension.z;
    if (block.z >= map_data.dimension.z) block.z -= map_data.dimension.z;
  };
  if (block.x < 0 || block.x >= map_data.dimension.x) return;
  if (block.y < 0 || block.y >= map_data.dimension.y) return;
  if (block.z < 0 || block.z >= map_data.dimension.z) return;
  if (map_block(block.x, block.y, block.z) != t) {
    map_block(block.x, block.y, block.z) = t;
    //printf("\n");
    invalidate_chunk(block.x, block.y, block.z, priority(0));
    invalidate_chunk(block.x, block.y, block.z+1, priority(1));
    invalidate_chunk(block.x, block.y+1, block.z, priority(1));
    invalidate_chunk(block.x+1, block.y, block.z, priority(1));
    invalidate_chunk(block.x, block.y-1, block.z, priority(1));
    invalidate_chunk(block.x-1, block.y, block.z, priority(1));
    invalidate_chunk(block.x, block.y, block.z-1, priority(1));
    if (option_lighting) {
      for (int zi = -32; zi <= 32; zi += 32) {
        for (int yi = -32; yi <= 32; yi += 32) {
          for (int xi = -32; xi <= 32; xi += 32) {
            invalidate_chunk(block.x + xi, block.y + yi, block.z + zi, priority(2));
          };
        };
      };
    };
    if (option_lighting == 2 && map_data.resolution.x < 0.1625) {
      for (int zi = -64; zi <= 64; zi += 32) {
        for (int yi = -64; yi <= 64; yi += 32) {
          for (int xi = -64; xi <= 64; xi += 32) {
            invalidate_chunk(block.x + xi, block.y + yi, block.z + zi, priority(3));
          };
        };
      };
    };
  };
};

//--page-split-- map_get_block_type

int map_get_block_type(struct int_xyz block) {
  if (!map_data.block) return 0;

  // new rules
  if (block.x < 0 || block.x >= map_data.dimension.x || block.y < 0 || block.y >= map_data.dimension.y || block.z < 0 || block.z >= map_data.dimension.z + 1024) {
    if (block.z < map_data.sealevel) {
      return 1;
    } else {
      return 256;
    };
  };

  if (block.z >= map_data.dimension.z) return 255;

  // old rules
  //if (block.z >= map_data.dimension.z && (block.x <= -1 || block.y <= -1 || block.x >= map_data.dimension.x || block.y >= map_data.dimension.y)) return 0;
  //if (block.x <= -1 || block.y <= -1 || block.z <= -1 || block.x >= map_data.dimension.x || block.y >= map_data.dimension.y) return 1;
  //if (block.x <= -10 || block.y <= -10 || block.z <= -20 || block.x >= map_data.dimension.x + 10 || block.y >= map_data.dimension.y + 10 || block.z >= map_data.dimension.z + 1000) return 0;
  //if (block.x <= -1 || block.y <= -1 || block.z <= -1 || block.x >= map_data.dimension.x || block.y >= map_data.dimension.y || block.z >= map_data.dimension.z) return 255;

  return map_block(block.x, block.y, block.z);
};

struct structure_map_selection map_selection;

//--page-split-- other_side

static struct int_xyz other_side(int side, struct int_xyz block) {
  if (side == BLOCK_SIDE_UP) block.z++;
  if (side == BLOCK_SIDE_FRONT) block.y--;
  if (side == BLOCK_SIDE_RIGHT) block.x++;
  if (side == BLOCK_SIDE_LEFT) block.x--;
  if (side == BLOCK_SIDE_BACK) block.y++;
  if (side == BLOCK_SIDE_DOWN) block.z--;
  return block;
};

//--page-split-- map_mouse_test

void map_mouse_test() {

  if (!map_initialization_flag) return;

  if (!glfw_mouse_capture_flag) return;

  DEBUG("enter map_mouse_test()");

  double reach = 2.0;
  if (player_fly) reach *= 2.0;
  if (option_superhuman) reach *= 2.0;
  reach += 0.5 * map_data.resolution.x;
  reach /= map_data.resolution.x;

  // if (!block_data[map_get_block_type(c)].impassable) {
  // if (map_get_block_type(d) && block_data[map_get_block_type(d)].impassable) {

  double best_distance = 2 * reach + 1.0;

  if (option_anaglyph_enable) best_distance = 4096;

  // expand search for auto-perspective's benefit
  //if (best_distance < 20) best_distance = 20;

  double reach_distance = reach + 1.0;
  struct int_xyz best_block;
  int best_side = 0;

  struct double_xyzuv vector = {};
  vector.x = 1;
  vector.u = player_view.u;
  vector.v = player_view.v;
  math_rotate_vector(&vector);

  struct double_xyz player_reach;
  if (player_fly) {
    player_reach.x = player_view.x;
    player_reach.y = player_view.y;
    player_reach.z = player_view.z;
  } else {
    player_reach.x = player_position.x;
    player_reach.y = player_position.y;
    player_reach.z = player_position.z - 0.5 * CAMERA_HEIGHT_METERS / map_data.resolution.x;
  };

  if (vector.x < 0) {
    struct double_xyzuv point = player_view;
    //printf("Player view: x=%f y=%f z=%f, vector = %f, %f, %f\n", point.x, point.y, point.z, vector.x, vector.y, vector.z);
    double advance = player_view.x - floor(player_view.x);
    //printf("advance = %f\n", advance);
    point.x -= advance * vector.x / vector.x;
    point.y -= advance * vector.y / vector.x;
    point.z -= advance * vector.z / vector.x;
    double distance = sqrt((point.x - player_view.x) * (point.x - player_view.x) + (point.y - player_view.y) * (point.y - player_view.y) + (point.z - player_view.z) * (point.z - player_view.z));
    while (distance < best_distance) {
      //printf("x=%f y=%f z=%f d=%f\n", point.x, point.y, point.z, distance);
      struct int_xyz block;
      block.x = floor(point.x - 0.5);
      block.y = floor(point.y);
      block.z = floor(point.z);
      int type = map_get_block_type(block);
      if (type != 0 && block_data[type].impassable) {
        int type = map_get_block_type(other_side(BLOCK_SIDE_RIGHT, block));
        if (type != 0 && !block_data[type].impassable) {
          //printf("This looks good!\n");
          best_distance = distance;
          reach_distance = sqrt((point.x - player_reach.x) * (point.x - player_reach.x) + (point.y - player_reach.y) * (point.y - player_reach.y) + (point.z - player_reach.z) * (point.z - player_reach.z));
          best_block = block;
          best_side = BLOCK_SIDE_RIGHT;
        };
      };
      point.x -= vector.x / vector.x;
      point.y -= vector.y / vector.x;
      point.z -= vector.z / vector.x;
      distance = sqrt((point.x - player_view.x) * (point.x - player_view.x) + (point.y - player_view.y) * (point.y - player_view.y) + (point.z - player_view.z) * (point.z - player_view.z));
    };
  };
  //printf("\n");

  if (vector.y < 0) {
    struct double_xyzuv point = player_view;
    double advance = player_view.y - floor(player_view.y);
    point.x -= advance * vector.x / vector.y;
    point.y -= advance * vector.y / vector.y;
    point.z -= advance * vector.z / vector.y;
    double distance = sqrt((point.x - player_view.x) * (point.x - player_view.x) + (point.y - player_view.y) * (point.y - player_view.y) + (point.z - player_view.z) * (point.z - player_view.z));
    while (distance < best_distance) {
      struct int_xyz block;
      block.x = floor(point.x);
      block.y = floor(point.y - 0.5);
      block.z = floor(point.z);
      int type = map_get_block_type(block);
      if (type != 0 && block_data[type].impassable) {
        int type = map_get_block_type(other_side(BLOCK_SIDE_BACK, block));
        if (type != 0 && !block_data[type].impassable) {
          best_distance = distance;
          reach_distance = sqrt((point.x - player_reach.x) * (point.x - player_reach.x) + (point.y - player_reach.y) * (point.y - player_reach.y) + (point.z - player_reach.z) * (point.z - player_reach.z));
          best_block = block;
          best_side = BLOCK_SIDE_BACK;
        };
      };
      point.x -= vector.x / vector.y;
      point.y -= vector.y / vector.y;
      point.z -= vector.z / vector.y;
      distance = sqrt((point.x - player_view.x) * (point.x - player_view.x) + (point.y - player_view.y) * (point.y - player_view.y) + (point.z - player_view.z) * (point.z - player_view.z));
    };
  };

  if (vector.z < 0) {
    struct double_xyzuv point = player_view;
    double advance = player_view.z - floor(player_view.z);
    point.x -= advance * vector.x / vector.z;
    point.y -= advance * vector.y / vector.z;
    point.z -= advance * vector.z / vector.z;
    double distance = sqrt((point.x - player_view.x) * (point.x - player_view.x) + (point.y - player_view.y) * (point.y - player_view.y) + (point.z - player_view.z) * (point.z - player_view.z));
    while (distance < best_distance) {
      struct int_xyz block;
      block.x = floor(point.x);
      block.y = floor(point.y);
      block.z = floor(point.z - 0.5);
      int type = map_get_block_type(block);
      if (type != 0 && block_data[type].impassable) {
        int type = map_get_block_type(other_side(BLOCK_SIDE_UP, block));
        if (type != 0 && !block_data[type].impassable) {
          best_distance = distance;
          reach_distance = sqrt((point.x - player_reach.x) * (point.x - player_reach.x) + (point.y - player_reach.y) * (point.y - player_reach.y) + (point.z - player_reach.z) * (point.z - player_reach.z));
          best_block = block;
          best_side = BLOCK_SIDE_UP;
        };
      };
      point.x -= vector.x / vector.z;
      point.y -= vector.y / vector.z;
      point.z -= vector.z / vector.z;
      distance = sqrt((point.x - player_view.x) * (point.x - player_view.x) + (point.y - player_view.y) * (point.y - player_view.y) + (point.z - player_view.z) * (point.z - player_view.z));
    };
  };

  if (vector.x > 0) {
    struct double_xyzuv point = player_view;
    //printf("Player view: x=%f y=%f z=%f, vector = %f, %f, %f\n", point.x, point.y, point.z, vector.x, vector.y, vector.z);
    double advance = ceil(player_view.x) - player_view.x;
    //printf("advance = %f\n", advance);
    point.x += advance * vector.x / vector.x;
    point.y += advance * vector.y / vector.x;
    point.z += advance * vector.z / vector.x;
    double distance = sqrt((point.x - player_view.x) * (point.x - player_view.x) + (point.y - player_view.y) * (point.y - player_view.y) + (point.z - player_view.z) * (point.z - player_view.z));
    while (distance < best_distance) {
      //printf("x=%f y=%f z=%f d=%f\n", point.x, point.y, point.z, distance);
      struct int_xyz block;
      block.x = floor(point.x + 0.5);
      block.y = floor(point.y);
      block.z = floor(point.z);
      int type = map_get_block_type(block);
      if (type != 0 && block_data[type].impassable) {
        int type = map_get_block_type(other_side(BLOCK_SIDE_LEFT, block));
        if (type != 0 && !block_data[type].impassable) {
          best_distance = distance;
          reach_distance = sqrt((point.x - player_reach.x) * (point.x - player_reach.x) + (point.y - player_reach.y) * (point.y - player_reach.y) + (point.z - player_reach.z) * (point.z - player_reach.z));
          best_block = block;
          best_side = BLOCK_SIDE_LEFT;
        };
      };
      point.x += vector.x / vector.x;
      point.y += vector.y / vector.x;
      point.z += vector.z / vector.x;
      distance = sqrt((point.x - player_view.x) * (point.x - player_view.x) + (point.y - player_view.y) * (point.y - player_view.y) + (point.z - player_view.z) * (point.z - player_view.z));
    };
  };
  //printf("\n");

  if (vector.y > 0) {
    struct double_xyzuv point = player_view;
    double advance = ceil(player_view.y) - player_view.y;
    point.x += advance * vector.x / vector.y;
    point.y += advance * vector.y / vector.y;
    point.z += advance * vector.z / vector.y;
    double distance = sqrt((point.x - player_view.x) * (point.x - player_view.x) + (point.y - player_view.y) * (point.y - player_view.y) + (point.z - player_view.z) * (point.z - player_view.z));
    while (distance < best_distance) {
      struct int_xyz block;
      block.x = floor(point.x);
      block.y = floor(point.y + 0.5);
      block.z = floor(point.z);
      int type = map_get_block_type(block);
      if (type != 0 && block_data[type].impassable) {
        int type = map_get_block_type(other_side(BLOCK_SIDE_FRONT, block));
        if (type != 0 && !block_data[type].impassable) {
          best_distance = distance;
          reach_distance = sqrt((point.x - player_reach.x) * (point.x - player_reach.x) + (point.y - player_reach.y) * (point.y - player_reach.y) + (point.z - player_reach.z) * (point.z - player_reach.z));
          best_block = block;
          best_side = BLOCK_SIDE_FRONT;
        };
      };
      point.x += vector.x / vector.y;
      point.y += vector.y / vector.y;
      point.z += vector.z / vector.y;
      distance = sqrt((point.x - player_view.x) * (point.x - player_view.x) + (point.y - player_view.y) * (point.y - player_view.y) + (point.z - player_view.z) * (point.z - player_view.z));
    };
  };

  if (vector.z > 0) {
    struct double_xyzuv point = player_view;
    double advance = ceil(player_view.z) - player_view.z;
    point.x += advance * vector.x / vector.z;
    point.y += advance * vector.y / vector.z;
    point.z += advance * vector.z / vector.z;
    double distance = sqrt((point.x - player_view.x) * (point.x - player_view.x) + (point.y - player_view.y) * (point.y - player_view.y) + (point.z - player_view.z) * (point.z - player_view.z));
    while (distance < best_distance) {
      struct int_xyz block;
      block.x = floor(point.x);
      block.y = floor(point.y);
      block.z = floor(point.z + 0.5);
      int type = map_get_block_type(block);
      if (type != 0 && block_data[type].impassable) {
        int type = map_get_block_type(other_side(BLOCK_SIDE_DOWN, block));
        if (type != 0 && !block_data[type].impassable) {
          best_distance = distance;
          reach_distance = sqrt((point.x - player_reach.x) * (point.x - player_reach.x) + (point.y - player_reach.y) * (point.y - player_reach.y) + (point.z - player_reach.z) * (point.z - player_reach.z));
          best_block = block;
          best_side = BLOCK_SIDE_DOWN;
        };
      };
      point.x += vector.x / vector.z;
      point.y += vector.y / vector.z;
      point.z += vector.z / vector.z;
      distance = sqrt((point.x - player_view.x) * (point.x - player_view.x) + (point.y - player_view.y) * (point.y - player_view.y) + (point.z - player_view.z) * (point.z - player_view.z));
    };
  };

  map_crosshair_distance = best_distance * map_data.resolution.x;
  //printf("map_crosshair_distance = %f\n", map_crosshair_distance);

  #if 0
  // What does this do?  Adjust FOV based on distance of targeted block?
  printf("Distance: %0.2f\n", best_distance);
  static double c = 10.0;
  c = 0.98 * c + 0.02 * best_distance;
  double fov = (20.0 / c) * 45.0;
  if (fov < 30) fov = 30;
  if (fov > 120) fov = 120;
  map_perspective_angle = fov;
  #endif

  if (reach_distance <= reach) {
    map_selection.valid = 1;
    map_selection.side = best_side;
    map_selection.create = best_block;
    map_selection.destroy = best_block;
    if (best_side == BLOCK_SIDE_UP) map_selection.create.z++;
    if (best_side == BLOCK_SIDE_FRONT) map_selection.create.y--;
    if (best_side == BLOCK_SIDE_RIGHT) map_selection.create.x++;
    if (best_side == BLOCK_SIDE_LEFT) map_selection.create.x--;
    if (best_side == BLOCK_SIDE_BACK) map_selection.create.y++;
    if (best_side == BLOCK_SIDE_DOWN) map_selection.create.z--;
    //if (map_selection.destroy.x < 0) map_selection.destroy.x += map_data.dimension.x;
    //if (map_selection.destroy.y < 0) map_selection.destroy.y += map_data.dimension.y;
    //if (map_selection.destroy.z < 0) map_selection.destroy.z += map_data.dimension.z;
    //if (map_selection.destroy.x >= map_data.dimension.x) map_selection.destroy.x -= map_data.dimension.x;
    //if (map_selection.destroy.y >= map_data.dimension.y) map_selection.destroy.y -= map_data.dimension.y;
    //if (map_selection.destroy.z >= map_data.dimension.z) map_selection.destroy.z -= map_data.dimension.z;
  } else {
    map_selection.valid = 0;
  };

  DEBUG("leave map_mouse_test()");

};

//--page-split-- compile_display_list

static void compile_display_list(GLuint display_list, struct structure_render_list *render_list, int render_list_size) {
  DEBUG("enter compile_display_list()");

  lag_push(10, "compile_display_list()");

  for (int layer = 0; layer < chunk_limit; layer += chunk_dimension.x * chunk_dimension.y * chunk_dimension.z) {

    glNewList(display_list + layer, GL_COMPILE);

    if (option_textures) glEnable(GL_TEXTURE_2D);

    if (layer != 0) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_COLOR);
    };

    for (int t = 0; t <= TEXTURE_MAX_TEXTURES;) {
      int next = TEXTURE_MAX_TEXTURES + 1;
      int quads_begun = 0;
      for (int i = 0; i < render_list_size; i++) {
        if ((texture_data[render_list[i].index].alpha == 1.0 && layer) || (texture_data[render_list[i].index].alpha != 1.0 && !layer)) {
          if (render_list[i].index == t || !option_textures) {
            if (!quads_begun) {
              if (option_textures) glCallList(texture_list_base + t);
              if (texture_data[t].alpha > 0.0 && texture_data[t].alpha < 1.0) {
                glEnable(GL_ALPHA_TEST);
                glAlphaFunc(GL_GEQUAL, texture_data[t].alpha);
              };
              glBegin(GL_QUADS);
              quads_begun = 1;
            };

            double value;
            switch (render_list[i].side) {
              case BLOCK_SIDE_UP: case BLOCK_SIDE_DOWN:
                value = 1.0;
              break;
              case BLOCK_SIDE_FRONT: case BLOCK_SIDE_BACK:
                value = 0.875;
              break;
              case BLOCK_SIDE_LEFT: case BLOCK_SIDE_RIGHT:
                value = 0.75;
              break;
            };
            //if (option_lighting) value = (1.0 + value) / 2.0;
            value *= render_list[i].light;
            if (render_list[i].light > 1.5) value = 1.0;
            if (option_textures) {
              glColor3f(value, value, value);
            } else {
              glColor4f(value * texture_data[render_list[i].index].r, value * texture_data[render_list[i].index].g, value * texture_data[render_list[i].index].b, texture_data[render_list[i].index].a);
            };

//#ifdef TEST
//            if (texture_data[t].alpha > 0.0) {
//              double xo = 0, yo = 0, zo = 0;
//              if (render_list[i].side == BLOCK_SIDE_UP) zo = -0.5;
//              if (render_list[i].side == BLOCK_SIDE_LEFT) xo = +0.5;
//              if (render_list[i].side == BLOCK_SIDE_RIGHT) xo = -0.5;
//              if (render_list[i].side == BLOCK_SIDE_FRONT) yo = +0.5;
//              if (render_list[i].side == BLOCK_SIDE_BACK) yo = -0.5;
//              if (render_list[i].side == BLOCK_SIDE_DOWN) zo = +0.5;
//              glTexCoord2f(render_list[i].coords[0].x, render_list[i].coords[0].y);
//              glVertex3f(render_list[i].points[0].x + xo, render_list[i].points[0].y + yo, render_list[i].points[0].z + zo);
//              glTexCoord2f(render_list[i].coords[1].x, render_list[i].coords[0].y);
//              glVertex3f(render_list[i].points[1].x + xo, render_list[i].points[1].y + yo, render_list[i].points[1].z + zo);
//              glTexCoord2f(render_list[i].coords[1].x, render_list[i].coords[1].y);
//              glVertex3f(render_list[i].points[2].x + xo, render_list[i].points[2].y + yo, render_list[i].points[2].z + zo);
//              glTexCoord2f(render_list[i].coords[0].x, render_list[i].coords[1].y);
//              glVertex3f(render_list[i].points[3].x + xo, render_list[i].points[3].y + yo, render_list[i].points[3].z + zo);
//            } else {
//#endif
              glTexCoord2f(render_list[i].coords[0].x, render_list[i].coords[0].y);
              glVertex3f(render_list[i].points[0].x, render_list[i].points[0].y, render_list[i].points[0].z);
              glTexCoord2f(render_list[i].coords[1].x, render_list[i].coords[0].y);
              glVertex3f(render_list[i].points[1].x, render_list[i].points[1].y, render_list[i].points[1].z);
              glTexCoord2f(render_list[i].coords[1].x, render_list[i].coords[1].y);
              glVertex3f(render_list[i].points[2].x, render_list[i].points[2].y, render_list[i].points[2].z);
              glTexCoord2f(render_list[i].coords[0].x, render_list[i].coords[1].y);
              glVertex3f(render_list[i].points[3].x, render_list[i].points[3].y, render_list[i].points[3].z);
//#ifdef TEST
//            };
//#endif
          } else if (render_list[i].index > t) {
            if (render_list[i].index < next) next = render_list[i].index;
          };
        };
      };
      if (quads_begun) {
        glEnd();
        if (texture_data[t].alpha > 0.0 && texture_data[t].alpha < 1.0) glDisable(GL_ALPHA_TEST);
      };
      t = next;
    };

    if (layer != 0) glDisable(GL_BLEND);

    if (option_textures) glDisable(GL_TEXTURE_2D);

    glEndList();

  };

  lag_pop();

  DEBUG("leave compile_display_list()");
};

// This code is executed inside separate threads.

//--page-split-- compile_chunk

static void compile_chunk(struct int_xyz chunk, struct structure_render_list **real_list, int *list_size) {

  lag_push(100, "compile_chunk()");

  // Figure out lighting...

  int light_flag = 0;
  if (option_lighting) {
    if (map_data.resolution.x < 0.1625) {
      if (option_lighting == 2) light_flag = 1;
    } else if (map_data.resolution.x < 1.0 / 3.0) {
      light_flag = 2;
    } else {
      light_flag = 3;
    };
  };

  #define LD light_distance
  #define SD short_distance
  #define EC extra_chunks

  #define CC (2 * EC + 1)          // chunk count (one dimension) of light area
  #define BC (CC * chunk_size)     // block count of the same
  #define BL (CC * chunk_size - 1) // one less?  why does this need a #define?

  int light_distance;
  int short_distance;
  int extra_chunks;

  if (light_flag == 1) {
    light_distance = 64;
    short_distance = 32;
    extra_chunks = 2;
  } else if (light_flag == 2) {
    light_distance = 32;
    short_distance = 16;
    extra_chunks = 1;
  } else {
    light_distance = 16;
    short_distance = 8;
    extra_chunks = 1;
  };

  char *b = NULL;
  char *l = NULL;
  #define B(x,y,z) *(b + ((z) * BC + (y)) * BC + (x))
  #define L(x,y,z) *(l + ((z) * BC + (y)) * BC + (x))
  struct int_xyz *this = NULL;
  struct int_xyz *next = NULL;
  struct int_xyz *temp;
  #define LIST_INCREMENT 65536
  int this_size = LIST_INCREMENT;
  int next_size = LIST_INCREMENT;
  int temp_size;
  if (light_flag) {
    memory_allocate(&b, BC*BC*BC);
    memset(b, 0, BC*BC*BC);
    memory_allocate(&l, BC*BC*BC);
    memset(l, 0, BC*BC*BC);
    memory_allocate(&this, this_size * sizeof(struct int_xyz));
    memory_allocate(&next, next_size * sizeof(struct int_xyz));
    int this_i = 0;
    int next_i = 0;
    int ll = LD;
    struct int_xyz a;

    lag_push(1, "* filling array");
    struct int_xyz c;
    for (c.z = -EC; c.z <= +EC; c.z++) {
      for (c.y = -EC; c.y <= +EC; c.y++) {
        for (c.x = -EC; c.x <= +EC; c.x++) {
          if (c.x + chunk.x < 0 || c.x + chunk.x >= chunk_dimension.x) continue;
          if (c.y + chunk.y < 0 || c.y + chunk.y >= chunk_dimension.y) continue;
          if (c.z + chunk.z < 0 || c.z + chunk.z >= chunk_dimension.z) continue;
          char *base = map_data.block +
            ((c.z + chunk.z) * chunk_size * map_data.dimension.y +
            (c.y + chunk.y) * chunk_size) * map_data.dimension.x +
            (c.x + chunk.x) * chunk_size;
          for (int z = 0; z < chunk_size; z++) {
            if ((chunk.z + c.z) * chunk_size + z >= map_data.dimension.z) break;
            for (int y = 0; y < chunk_size; y++) {
              memmove(
                b + ((
                  chunk_size * (c.z + EC) + z)
                  * BC + (chunk_size * (c.y + EC) + y))
                  * BC + chunk_size * (c.x + EC),
                base + (z * map_data.dimension.y + y) * map_data.dimension.x,
                chunk_size
              );
            };
          };
        };
      };
    };
    lag_pop();

    lag_push(1, "* finding sun 1");
    if (option_fog_type & 1) {
      memset(l + BL * BC * BC, LD, BC * BC);
    } else {
      memset(l + BL * BC * BC, 0, BC * BC);
    };

    {
      a.z = BL;
      struct int_xyz b;
      b.z = chunk_size * chunk.z + a.z - chunk_size * EC;
      if (b.z < map_data.dimension.z) {
        int z_limit = chunk_size * chunk.z + 96;
        for (a.y = 0; a.y < BC; a.y++) {
          b.y = chunk_size * chunk.y + a.y - chunk_size * EC;
          if (b.y < 0 || b.y >= map_data.dimension.y) continue;
          for (a.x = 0; a.x < BC; a.x++) {
            b.x = chunk_size * chunk.x + a.x - chunk_size * EC;
            if (b.x < 0 || b.x >= map_data.dimension.x) continue;
            char *base = map_data.block + (b.z * map_data.dimension.y + b.y) * map_data.dimension.x + b.x;
            char *base_limit = map_data.block + (z_limit * map_data.dimension.y + b.y) * map_data.dimension.x + b.x;
            if (base_limit > map_data.block + map_data.limit) base_limit = map_data.block + map_data.limit;
            while (base < base_limit) {
              int t = *base; base += map_data.dimension.x * map_data.dimension.y;
              if (block_data[t].emission == 3) {
                L(a.x, a.y, a.z) = LD;
                break;
              };
              if (block_data[t].visible && !block_data[t].transparent) {
                L(a.x, a.y, a.z) = 0;
              };
            };
          };
        };
      };
    };

    lag_pop();

    lag_push(1, "* sun flow");

    for (int i = BC * BC * BC - 1; i >= BC * BC; i--) {
      if (block_data[b[i]].emission == 3) l[i] = LD;
      if (l[i] == LD) {
        int j = i - BC * BC;
        if (!block_data[b[j]].visible || block_data[b[j]].transparent) {
          l[j] = LD;
        };
      };
    };

    lag_pop();

    lag_push(1, "* finding sun 2");
    #define CONDITION(x, y, z) (L(x,y,z) < ll - 1 && (!block_data[B(x,y,z)].visible || block_data[B(x,y,z)].transparent))
    #define ACTION(x, y, z) L(x,y,z) = ll - 1, next[next_i++] = (struct int_xyz) {x, y, z}
    #define CONDITION_A(x, y, z) (L(x,y,z) <= ll - 1 && (!block_data[B(x,y,z)].visible || block_data[B(x,y,z)].transparent))
    #define CONDITION_B(x, y, z) (L(x,y,z) < ll - 1)
    for (a.z = 0; a.z < BC; a.z++) {
      for (a.y = 0; a.y < BC; a.y++) {
        for (a.x = 0; a.x < BC; a.x++) {
          if (L(a.x,a.y,a.z) == ll || block_data[B(a.x,a.y,a.z)].emission >= 2) {
            if (a.y > 0 && CONDITION_A(a.x, a.y-1, a.z)) {
              if (CONDITION_B(a.x, a.y-1, a.z)) ACTION(a.x, a.y-1, a.z);
              if (a.x > 0 && CONDITION(a.x-1, a.y-1, a.z)) ACTION(a.x-1, a.y-1, a.z);
              if (a.x < BL && CONDITION(a.x+1, a.y-1, a.z)) ACTION(a.x+1, a.y-1, a.z);
              if (a.z > 0 && CONDITION(a.x, a.y-1, a.z-1)) ACTION(a.x, a.y-1, a.z-1);
              if (a.z < BL && CONDITION(a.x, a.y-1, a.z+1)) ACTION(a.x, a.y-1, a.z+1);
            };
            if (a.y < BL && CONDITION_A(a.x, a.y+1, a.z)) {
              if (CONDITION_B(a.x, a.y+1, a.z)) ACTION(a.x, a.y+1, a.z);
              if (a.x > 0 && CONDITION(a.x-1, a.y+1, a.z)) ACTION(a.x-1, a.y+1, a.z);
              if (a.x < BL && CONDITION(a.x+1, a.y+1, a.z)) ACTION(a.x+1, a.y+1, a.z);
              if (a.z > 0 && CONDITION(a.x, a.y+1, a.z-1)) ACTION(a.x, a.y+1, a.z-1);
              if (a.z < BL && CONDITION(a.x, a.y+1, a.z+1)) ACTION(a.x, a.y+1, a.z+1);
            };
            if (a.x > 0 && CONDITION_A(a.x-1, a.y, a.z)) {
              if (CONDITION_B(a.x-1, a.y, a.z)) ACTION(a.x-1, a.y, a.z);
              if (a.y > 0 && CONDITION(a.x-1, a.y-1, a.z)) ACTION(a.x-1, a.y-1, a.z);
              if (a.y < BL && CONDITION(a.x-1, a.y+1, a.z)) ACTION(a.x-1, a.y+1, a.z);
              if (a.z > 0 && CONDITION(a.x-1, a.y, a.z-1)) ACTION(a.x-1, a.y, a.z-1);
              if (a.z < BL && CONDITION(a.x-1, a.y, a.z+1)) ACTION(a.x-1, a.y, a.z+1);
            };
            if (a.x < BL && CONDITION_A(a.x+1, a.y, a.z)) {
              if (CONDITION_B(a.x+1, a.y, a.z)) ACTION(a.x+1, a.y, a.z);
              if (a.y > 0 && CONDITION(a.x+1, a.y-1, a.z)) ACTION(a.x+1, a.y-1, a.z);
              if (a.y < BL && CONDITION(a.x+1, a.y+1, a.z)) ACTION(a.x+1, a.y+1, a.z);
              if (a.z > 0 && CONDITION(a.x+1, a.y, a.z-1)) ACTION(a.x+1, a.y, a.z-1);
              if (a.z < BL && CONDITION(a.x+1, a.y, a.z+1)) ACTION(a.x+1, a.y, a.z+1);
            };
            if (a.z > 0 && CONDITION_A(a.x, a.y, a.z-1)) {
              if (CONDITION_B(a.x, a.y, a.z-1)) ACTION(a.x, a.y, a.z-1);
              if (a.x > 0 && CONDITION(a.x-1, a.y, a.z-1)) ACTION(a.x-1, a.y, a.z-1);
              if (a.x < BL && CONDITION(a.x+1, a.y, a.z-1)) ACTION(a.x+1, a.y, a.z-1);
              if (a.y > 0 && CONDITION(a.x, a.y-1, a.z-1)) ACTION(a.x, a.y-1, a.z-1);
              if (a.y < BL && CONDITION(a.x, a.y+1, a.z-1)) ACTION(a.x, a.y+1, a.z-1);
            };
            if (a.z < BL && CONDITION_A(a.x, a.y, a.z+1)) {
              if (CONDITION_B(a.x, a.y, a.z+1)) ACTION(a.x, a.y, a.z+1);
              if (a.x > 0 && CONDITION(a.x-1, a.y, a.z+1)) ACTION(a.x-1, a.y, a.z+1);
              if (a.x < BL && CONDITION(a.x+1, a.y, a.z+1)) ACTION(a.x+1, a.y, a.z+1);
              if (a.y > 0 && CONDITION(a.x, a.y-1, a.z+1)) ACTION(a.x, a.y-1, a.z+1);
              if (a.y < BL && CONDITION(a.x, a.y+1, a.z+1)) ACTION(a.x, a.y+1, a.z+1);
            };
          };
          if (next_i > next_size - 27) {
            next_size += LIST_INCREMENT;
            memory_allocate(&next, next_size * sizeof(struct int_xyz));
          };
        };
      };
    };
    lag_pop();

    ll--;

    #define DIAGONAL_CONDITION_X 1
    #define DIAGONAL_CONDITION_Y 1
    #define DIAGONAL_CONDITION_Z 1

    lag_push(1, "* light flow");
    while (ll >= 2) {
      if (next_i == 0 && ll > SD) ll = SD;
      if (next_i == 0 && ll < SD) break;
      this_i = next_i; next_i = 0;
      temp = this; this = next; next = temp;
      temp_size = this_size; this_size = next_size; next_size = temp_size;
      if (ll == SD) {
        lag_push(1, "* finding lamps");
        for (a.z = 0; a.z < BC; a.z++) {
          for (a.y = 0; a.y < BC; a.y++) {
            for (a.x = 0; a.x < BC; a.x++) {
              if (L(a.x,a.y,a.z) == ll || block_data[B(a.x,a.y,a.z)].emission == 1) {
                if (a.y > 0 && CONDITION_A(a.x, a.y-1, a.z)) {
                  if (CONDITION_B(a.x, a.y-1, a.z)) ACTION(a.x, a.y-1, a.z);
                  if (a.x > 0 && CONDITION(a.x-1, a.y-1, a.z)) ACTION(a.x-1, a.y-1, a.z);
                  if (a.x < BL && CONDITION(a.x+1, a.y-1, a.z)) ACTION(a.x+1, a.y-1, a.z);
                  if (a.z > 0 && CONDITION(a.x, a.y-1, a.z-1)) ACTION(a.x, a.y-1, a.z-1);
                  if (a.z < BL && CONDITION(a.x, a.y-1, a.z+1)) ACTION(a.x, a.y-1, a.z+1);
                };
                if (a.y < BL && CONDITION_A(a.x, a.y+1, a.z)) {
                  if (CONDITION_B(a.x, a.y+1, a.z)) ACTION(a.x, a.y+1, a.z);
                  if (a.x > 0 && CONDITION(a.x-1, a.y+1, a.z)) ACTION(a.x-1, a.y+1, a.z);
                  if (a.x < BL && CONDITION(a.x+1, a.y+1, a.z)) ACTION(a.x+1, a.y+1, a.z);
                  if (a.z > 0 && CONDITION(a.x, a.y+1, a.z-1)) ACTION(a.x, a.y+1, a.z-1);
                  if (a.z < BL && CONDITION(a.x, a.y+1, a.z+1)) ACTION(a.x, a.y+1, a.z+1);
                };
                if (a.x > 0 && CONDITION_A(a.x-1, a.y, a.z)) {
                  if (CONDITION_B(a.x-1, a.y, a.z)) ACTION(a.x-1, a.y, a.z);
                  if (a.y > 0 && CONDITION(a.x-1, a.y-1, a.z)) ACTION(a.x-1, a.y-1, a.z);
                  if (a.y < BL && CONDITION(a.x-1, a.y+1, a.z)) ACTION(a.x-1, a.y+1, a.z);
                  if (a.z > 0 && CONDITION(a.x-1, a.y, a.z-1)) ACTION(a.x-1, a.y, a.z-1);
                  if (a.z < BL && CONDITION(a.x-1, a.y, a.z+1)) ACTION(a.x-1, a.y, a.z+1);
                };
                if (a.x < BL && CONDITION_A(a.x+1, a.y, a.z)) {
                  if (CONDITION_B(a.x+1, a.y, a.z)) ACTION(a.x+1, a.y, a.z);
                  if (a.y > 0 && CONDITION(a.x+1, a.y-1, a.z)) ACTION(a.x+1, a.y-1, a.z);
                  if (a.y < BL && CONDITION(a.x+1, a.y+1, a.z)) ACTION(a.x+1, a.y+1, a.z);
                  if (a.z > 0 && CONDITION(a.x+1, a.y, a.z-1)) ACTION(a.x+1, a.y, a.z-1);
                  if (a.z < BL && CONDITION(a.x+1, a.y, a.z+1)) ACTION(a.x+1, a.y, a.z+1);
                };
                if (a.z > 0 && CONDITION_A(a.x, a.y, a.z-1)) {
                  if (CONDITION_B(a.x, a.y, a.z-1)) ACTION(a.x, a.y, a.z-1);
                  if (a.x > 0 && CONDITION(a.x-1, a.y, a.z-1)) ACTION(a.x-1, a.y, a.z-1);
                  if (a.x < BL && CONDITION(a.x+1, a.y, a.z-1)) ACTION(a.x+1, a.y, a.z-1);
                  if (a.y > 0 && CONDITION(a.x, a.y-1, a.z-1)) ACTION(a.x, a.y-1, a.z-1);
                  if (a.y < BL && CONDITION(a.x, a.y+1, a.z-1)) ACTION(a.x, a.y+1, a.z-1);
                };
                if (a.z < BL && CONDITION_A(a.x, a.y, a.z+1)) {
                  if (CONDITION_B(a.x, a.y, a.z+1)) ACTION(a.x, a.y, a.z+1);
                  if (a.x > 0 && CONDITION(a.x-1, a.y, a.z+1)) ACTION(a.x-1, a.y, a.z+1);
                  if (a.x < BL && CONDITION(a.x+1, a.y, a.z+1)) ACTION(a.x+1, a.y, a.z+1);
                  if (a.y > 0 && CONDITION(a.x, a.y-1, a.z+1)) ACTION(a.x, a.y-1, a.z+1);
                  if (a.y < BL && CONDITION(a.x, a.y+1, a.z+1)) ACTION(a.x, a.y+1, a.z+1);
                };
              };
              if (next_i > next_size - 27) {
                next_size += LIST_INCREMENT;
                memory_allocate(&next, next_size * sizeof(struct int_xyz));
              };
            };
          };
        };
        lag_pop();
      } else {
        for (int i = 0; i < this_i; i++) {
          a = this[i];
          if (L(a.x,a.y,a.z) == ll) {
            if (a.y > 0 && CONDITION_A(a.x, a.y-1, a.z)) {
              if (CONDITION_B(a.x, a.y-1, a.z)) ACTION(a.x, a.y-1, a.z);
              if (a.x > 0 && CONDITION(a.x-1, a.y-1, a.z)) ACTION(a.x-1, a.y-1, a.z);
              if (a.x < BL && CONDITION(a.x+1, a.y-1, a.z)) ACTION(a.x+1, a.y-1, a.z);
              if (a.z > 0 && CONDITION(a.x, a.y-1, a.z-1)) ACTION(a.x, a.y-1, a.z-1);
              if (a.z < BL && CONDITION(a.x, a.y-1, a.z+1)) ACTION(a.x, a.y-1, a.z+1);
            };
            if (a.y < BL && CONDITION_A(a.x, a.y+1, a.z)) {
              if (CONDITION_B(a.x, a.y+1, a.z)) ACTION(a.x, a.y+1, a.z);
              if (a.x > 0 && CONDITION(a.x-1, a.y+1, a.z)) ACTION(a.x-1, a.y+1, a.z);
              if (a.x < BL && CONDITION(a.x+1, a.y+1, a.z)) ACTION(a.x+1, a.y+1, a.z);
              if (a.z > 0 && CONDITION(a.x, a.y+1, a.z-1)) ACTION(a.x, a.y+1, a.z-1);
              if (a.z < BL && CONDITION(a.x, a.y+1, a.z+1)) ACTION(a.x, a.y+1, a.z+1);
            };
            if (a.x > 0 && CONDITION_A(a.x-1, a.y, a.z)) {
              if (CONDITION_B(a.x-1, a.y, a.z)) ACTION(a.x-1, a.y, a.z);
              if (a.y > 0 && CONDITION(a.x-1, a.y-1, a.z)) ACTION(a.x-1, a.y-1, a.z);
              if (a.y < BL && CONDITION(a.x-1, a.y+1, a.z)) ACTION(a.x-1, a.y+1, a.z);
              if (a.z > 0 && CONDITION(a.x-1, a.y, a.z-1)) ACTION(a.x-1, a.y, a.z-1);
              if (a.z < BL && CONDITION(a.x-1, a.y, a.z+1)) ACTION(a.x-1, a.y, a.z+1);
            };
            if (a.x < BL && CONDITION_A(a.x+1, a.y, a.z)) {
              if (CONDITION_B(a.x+1, a.y, a.z)) ACTION(a.x+1, a.y, a.z);
              if (a.y > 0 && CONDITION(a.x+1, a.y-1, a.z)) ACTION(a.x+1, a.y-1, a.z);
              if (a.y < BL && CONDITION(a.x+1, a.y+1, a.z)) ACTION(a.x+1, a.y+1, a.z);
              if (a.z > 0 && CONDITION(a.x+1, a.y, a.z-1)) ACTION(a.x+1, a.y, a.z-1);
              if (a.z < BL && CONDITION(a.x+1, a.y, a.z+1)) ACTION(a.x+1, a.y, a.z+1);
            };
            if (a.z > 0 && CONDITION_A(a.x, a.y, a.z-1)) {
              if (CONDITION_B(a.x, a.y, a.z-1)) ACTION(a.x, a.y, a.z-1);
              if (a.x > 0 && CONDITION(a.x-1, a.y, a.z-1)) ACTION(a.x-1, a.y, a.z-1);
              if (a.x < BL && CONDITION(a.x+1, a.y, a.z-1)) ACTION(a.x+1, a.y, a.z-1);
              if (a.y > 0 && CONDITION(a.x, a.y-1, a.z-1)) ACTION(a.x, a.y-1, a.z-1);
              if (a.y < BL && CONDITION(a.x, a.y+1, a.z-1)) ACTION(a.x, a.y+1, a.z-1);
            };
            if (a.z < BL && CONDITION_A(a.x, a.y, a.z+1)) {
              if (CONDITION_B(a.x, a.y, a.z+1)) ACTION(a.x, a.y, a.z+1);
              if (a.x > 0 && CONDITION(a.x-1, a.y, a.z+1)) ACTION(a.x-1, a.y, a.z+1);
              if (a.x < BL && CONDITION(a.x+1, a.y, a.z+1)) ACTION(a.x+1, a.y, a.z+1);
              if (a.y > 0 && CONDITION(a.x, a.y-1, a.z+1)) ACTION(a.x, a.y-1, a.z+1);
              if (a.y < BL && CONDITION(a.x, a.y+1, a.z+1)) ACTION(a.x, a.y+1, a.z+1);
            };
          };
          if (next_i > next_size - 27) {
            next_size += LIST_INCREMENT;
            memory_allocate(&next, next_size * sizeof(struct int_xyz));
          };
        };
      };
      ll--;
    };
    lag_pop();

    memory_allocate(&this, 0);
    memory_allocate(&next, 0);
    memory_allocate(&b, 0);
  };

  // Create list of quads from block data...

  int max_list_entries = 6 * chunk_size * chunk_size * chunk_size;
  struct structure_render_list *the_list = NULL;
  struct structure_render_list **render_list = &the_list;
  memory_allocate(render_list, sizeof(struct structure_render_list) * max_list_entries);
  *list_size = 0;

  lag_push(100, "* reducing polygons");

  struct int_xyz c, m, o;
  // c = block coordinate within current chunk
  // m = map coordinate of current block
  // o = map coordinate of opposite block
  int i, j, t;
  int start = 1536;
  int grid[chunk_size][chunk_size];
  int light[chunk_size][chunk_size];
  int grid_count;
  for (int side = 0; side < 6; side++) {
    int first_z = 0;
    if (
      (side == BLOCK_SIDE_UP && chunk.z == 0) || (side == BLOCK_SIDE_DOWN && chunk.z == chunk_dimension.z - 1) ||
      (side == BLOCK_SIDE_BACK && chunk.y == 0) || (side == BLOCK_SIDE_FRONT && chunk.y == chunk_dimension.y - 1) ||
      (side == BLOCK_SIDE_RIGHT && chunk.x == 0) || (side == BLOCK_SIDE_LEFT && chunk.x == chunk_dimension.x - 1)
    ) {
      first_z = -1;
    };
    for (c.z = first_z; c.z < chunk_size; c.z++) {
    //for (c.z = chunk_size - 1; c.z >= 0; c.z--) {
      grid_count = 0;
      for (c.y = 0; c.y < chunk_size; c.y++) {
        for (c.x = 0; c.x < chunk_size; c.x++) {

          switch (side) {
            case BLOCK_SIDE_UP:
              m.x = (chunk.x << map_chunk_bits) + c.x; o.x = m.x;
              m.y = (chunk.y << map_chunk_bits) + c.y; o.y = m.y;
              m.z = (chunk.z << map_chunk_bits) + c.z; o.z = m.z + 1;
            break;
            case BLOCK_SIDE_FRONT:
              m.x = (chunk.x << map_chunk_bits) + c.x; o.x = m.x;
              m.y = (chunk.y << map_chunk_bits) - c.z + (1 << map_chunk_bits) - 1; o.y = m.y - 1;
              m.z = (chunk.z << map_chunk_bits) + c.y; o.z = m.z;
            break;
            case BLOCK_SIDE_RIGHT:
              m.x = (chunk.x << map_chunk_bits) + c.z; o.x = m.x + 1;
              m.y = (chunk.y << map_chunk_bits) + c.x; o.y = m.y;
              m.z = (chunk.z << map_chunk_bits) + c.y; o.z = m.z;
            break;
            case BLOCK_SIDE_LEFT:
              m.x = (chunk.x << map_chunk_bits) - c.z + (1 << map_chunk_bits) - 1; o.x = m.x - 1;
              m.y = (chunk.y << map_chunk_bits) - c.x + (1 << map_chunk_bits) - 1; o.y = m.y;
              m.z = (chunk.z << map_chunk_bits) + c.y; o.z = m.z;
            break;
            case BLOCK_SIDE_BACK:
              m.x = (chunk.x << map_chunk_bits) - c.x + (1 << map_chunk_bits) - 1; o.x = m.x;
              m.y = (chunk.y << map_chunk_bits) + c.z; o.y = m.y + 1;
              m.z = (chunk.z << map_chunk_bits) + c.y; o.z = m.z;
            break;
            case BLOCK_SIDE_DOWN:
              m.x = (chunk.x << map_chunk_bits) + c.x; o.x = m.x;
              m.y = (chunk.y << map_chunk_bits) - c.y + (1 << map_chunk_bits) - 1; o.y = m.y;
              m.z = (chunk.z << map_chunk_bits) - c.z + (1 << map_chunk_bits) - 1; o.z = m.z - 1;
            break;
          };

          i = map_get_block_type(m); // the block we are working on
          j = map_get_block_type(o); // the block opposite this one
          t = -1; // which texture we've decided to render here

          // if the opposite block doesn't obstruct the view of this block
          if (!block_data[j].visible || block_data[j].transparent) {
            // if this block has visible textures
            if (block_data[i].visible) {
              // and it's a different block
              if (i != j || block_data[i].between) {
                t = block_data[i].index[side];
              };
            };
          };

          // if the opposite block has a reverse texture and we haven't chosen
          // to draw a forward texture for this block, then draw the reverse texture
          if (t == -1 && block_data[j].visible) {
            if (block_data[j].reverse) {
              if (i != j) {
                t = block_data[j].index[5-side];
              };
            };
          };

          // Show invalid texture when block type zero is involved.
          if (t >= 0 && (i == 0 || j == 0)) t = TEXTURE_NO_ZERO;

          grid[c.x][c.y] = t;
          if (light_flag) {
            if (block_data[i].emission) {
              light[c.x][c.y] = 255;
            } else {
              int t = L(o.x - chunk_size * chunk.x + chunk_size * EC, o.y - chunk_size * chunk.y + chunk_size * EC, o.z - chunk_size * chunk.z + chunk_size * EC);
              //if (option_quantization && t < 64) t = 3 * ceil(t / 3.0);
              light[c.x][c.y] = t;
            };
          } else {
            light[c.x][c.y] = LD;
          };
          if (t >= 0) grid_count++;

        };
      };

      // Reduce polygons here!

      for (int a = 0; a < area_count && grid_count; a++) {
        if (area[a].s > chunk_size || area[a].t > chunk_size) continue;
        for (c.y = 0; c.y <= chunk_size - area[a].t; c.y++) {
          for (c.x = 0; c.x <= chunk_size - area[a].s; c.x++) {
            int texture = -2; int shade;
            for (int v = c.y; v < c.y + area[a].t; v++) {
              for (int u = c.x; u < c.x + area[a].s; u++) {
                if (grid[u][v] == -1) goto NO_MATCH;
                if (grid[u][v] >= 0) {
                  if (texture == -2) {
                    texture = grid[u][v];
                    shade = light[u][v];
                  };
                  if (grid[u][v] != texture || light[u][v] != shade) goto NO_MATCH;
                };
              };
            };
            if (texture >= 0) {

              #ifdef TEST
              area_tally[a]++;
              #endif

              if (texture < start) start = texture;
              (*render_list)[*list_size].index = texture;
              (*render_list)[*list_size].side = side;
              if (light[c.x][c.y] > LD) {
                (*render_list)[*list_size].light = 2.0;
              } else if (light[c.x][c.y] == LD) {
                (*render_list)[*list_size].light = 1.0;
              } else {
                double l = light[c.x][c.y] / (double) LD;
                l = pow(l, 1.2);
                (*render_list)[*list_size].light = 0.05 + 0.7 * l;
              };

              #ifdef TEST
              //  (*render_list)[*list_size].light = easy_random(255) / 255.0;
              #endif

              switch (side) {
                case BLOCK_SIDE_UP:
                  (*render_list)[*list_size].points[0].x = chunk_size * chunk.x + c.x;
                  (*render_list)[*list_size].points[0].y = chunk_size * chunk.y + c.y;
                  (*render_list)[*list_size].points[0].z = chunk_size * chunk.z + c.z + 1;
                  (*render_list)[*list_size].points[1].x = chunk_size * chunk.x + c.x + area[a].s;
                  (*render_list)[*list_size].points[1].y = chunk_size * chunk.y + c.y;
                  (*render_list)[*list_size].points[1].z = chunk_size * chunk.z + c.z + 1;
                  (*render_list)[*list_size].points[2].x = chunk_size * chunk.x + c.x + area[a].s;
                  (*render_list)[*list_size].points[2].y = chunk_size * chunk.y + c.y + area[a].t;
                  (*render_list)[*list_size].points[2].z = chunk_size * chunk.z + c.z + 1;
                  (*render_list)[*list_size].points[3].x = chunk_size * chunk.x + c.x;
                  (*render_list)[*list_size].points[3].y = chunk_size * chunk.y + c.y + area[a].t;
                  (*render_list)[*list_size].points[3].z = chunk_size * chunk.z + c.z + 1;
                  if (!option_textures) {
                    (*render_list)[*list_size].coords[0].x = +(*render_list)[*list_size].points[0].x;
                    (*render_list)[*list_size].coords[1].x = +(*render_list)[*list_size].points[2].x;
                  } else if (texture_data[texture].x_scale >= 0) {
                    (*render_list)[*list_size].coords[0].x = +(*render_list)[*list_size].points[0].x * texture_data[texture].x_scale * map_data.resolution.x;
                    (*render_list)[*list_size].coords[1].x = +(*render_list)[*list_size].points[2].x * texture_data[texture].x_scale * map_data.resolution.x;
                  } else {
                    (*render_list)[*list_size].coords[0].x = +(*render_list)[*list_size].points[0].x * -texture_data[texture].x_scale;
                    (*render_list)[*list_size].coords[1].x = +(*render_list)[*list_size].points[2].x * -texture_data[texture].x_scale;
                  };
                  if (!option_textures) {
                    (*render_list)[*list_size].coords[0].y = +(*render_list)[*list_size].points[0].y;
                    (*render_list)[*list_size].coords[1].y = +(*render_list)[*list_size].points[2].y;
                  } else if (texture_data[texture].y_scale >= 0) {
                    (*render_list)[*list_size].coords[0].y = +(*render_list)[*list_size].points[0].y * texture_data[texture].y_scale * map_data.resolution.y;
                    (*render_list)[*list_size].coords[1].y = +(*render_list)[*list_size].points[2].y * texture_data[texture].y_scale * map_data.resolution.y;
                  } else {
                    (*render_list)[*list_size].coords[0].y = +(*render_list)[*list_size].points[0].y * -texture_data[texture].y_scale;
                    (*render_list)[*list_size].coords[1].y = +(*render_list)[*list_size].points[2].y * -texture_data[texture].y_scale;
                  };
                break;
                case BLOCK_SIDE_FRONT:
                  (*render_list)[*list_size].points[0].x = chunk_size * chunk.x + c.x;
                  (*render_list)[*list_size].points[0].y = chunk_size * chunk.y + chunk_size - c.z - 1;
                  (*render_list)[*list_size].points[0].z = chunk_size * chunk.z + c.y;
                  (*render_list)[*list_size].points[1].x = chunk_size * chunk.x + c.x + area[a].s;
                  (*render_list)[*list_size].points[1].y = chunk_size * chunk.y + chunk_size - c.z - 1;
                  (*render_list)[*list_size].points[1].z = chunk_size * chunk.z + c.y;
                  (*render_list)[*list_size].points[2].x = chunk_size * chunk.x + c.x + area[a].s;
                  (*render_list)[*list_size].points[2].y = chunk_size * chunk.y + chunk_size - c.z - 1;
                  (*render_list)[*list_size].points[2].z = chunk_size * chunk.z + c.y + area[a].t;
                  (*render_list)[*list_size].points[3].x = chunk_size * chunk.x + c.x;
                  (*render_list)[*list_size].points[3].y = chunk_size * chunk.y + chunk_size - c.z - 1;
                  (*render_list)[*list_size].points[3].z = chunk_size * chunk.z + c.y + area[a].t;
                  if (!option_textures) {
                    (*render_list)[*list_size].coords[0].x = +(*render_list)[*list_size].points[0].x;
                    (*render_list)[*list_size].coords[1].x = +(*render_list)[*list_size].points[2].x;
                  } else if (texture_data[texture].x_scale >= 0) {
                    (*render_list)[*list_size].coords[0].x = +(*render_list)[*list_size].points[0].x * texture_data[texture].x_scale * map_data.resolution.x;
                    (*render_list)[*list_size].coords[1].x = +(*render_list)[*list_size].points[2].x * texture_data[texture].x_scale * map_data.resolution.x;
                  } else {
                    (*render_list)[*list_size].coords[0].x = +(*render_list)[*list_size].points[0].x * -texture_data[texture].x_scale;
                    (*render_list)[*list_size].coords[1].x = +(*render_list)[*list_size].points[2].x * -texture_data[texture].x_scale;
                  };
                  if (!option_textures) {
                    (*render_list)[*list_size].coords[0].y = +(*render_list)[*list_size].points[0].z;
                    (*render_list)[*list_size].coords[1].y = +(*render_list)[*list_size].points[2].z;
                  } else if (texture_data[texture].y_scale >= 0) {
                    (*render_list)[*list_size].coords[0].y = +(*render_list)[*list_size].points[0].z * texture_data[texture].y_scale * map_data.resolution.z;
                    (*render_list)[*list_size].coords[1].y = +(*render_list)[*list_size].points[2].z * texture_data[texture].y_scale * map_data.resolution.z;
                  } else {
                    (*render_list)[*list_size].coords[0].y = +(*render_list)[*list_size].points[0].z * -texture_data[texture].y_scale;
                    (*render_list)[*list_size].coords[1].y = +(*render_list)[*list_size].points[2].z * -texture_data[texture].y_scale;
                  };
                break;
                case BLOCK_SIDE_RIGHT:
                  (*render_list)[*list_size].points[0].x = chunk_size * chunk.x + c.z + 1;
                  (*render_list)[*list_size].points[0].y = chunk_size * chunk.y + c.x;
                  (*render_list)[*list_size].points[0].z = chunk_size * chunk.z + c.y;
                  (*render_list)[*list_size].points[1].x = chunk_size * chunk.x + c.z + 1;
                  (*render_list)[*list_size].points[1].y = chunk_size * chunk.y + c.x + area[a].s;
                  (*render_list)[*list_size].points[1].z = chunk_size * chunk.z + c.y;
                  (*render_list)[*list_size].points[2].x = chunk_size * chunk.x + c.z + 1;
                  (*render_list)[*list_size].points[2].y = chunk_size * chunk.y + c.x + area[a].s;
                  (*render_list)[*list_size].points[2].z = chunk_size * chunk.z + c.y + area[a].t;
                  (*render_list)[*list_size].points[3].x = chunk_size * chunk.x + c.z + 1;
                  (*render_list)[*list_size].points[3].y = chunk_size * chunk.y + c.x;
                  (*render_list)[*list_size].points[3].z = chunk_size * chunk.z + c.y + area[a].t;
                  if (!option_textures) {
                    (*render_list)[*list_size].coords[0].x = +(*render_list)[*list_size].points[0].y;
                    (*render_list)[*list_size].coords[1].x = +(*render_list)[*list_size].points[2].y;
                  } else if (texture_data[texture].x_scale >= 0) {
                    (*render_list)[*list_size].coords[0].x = +(*render_list)[*list_size].points[0].y * texture_data[texture].x_scale * map_data.resolution.y;
                    (*render_list)[*list_size].coords[1].x = +(*render_list)[*list_size].points[2].y * texture_data[texture].x_scale * map_data.resolution.y;
                  } else {
                    (*render_list)[*list_size].coords[0].x = +(*render_list)[*list_size].points[0].y * -texture_data[texture].x_scale;
                    (*render_list)[*list_size].coords[1].x = +(*render_list)[*list_size].points[2].y * -texture_data[texture].x_scale;
                  };
                  if (!option_textures) {
                    (*render_list)[*list_size].coords[0].y = +(*render_list)[*list_size].points[0].z;
                    (*render_list)[*list_size].coords[1].y = +(*render_list)[*list_size].points[2].z;
                  } else if (texture_data[texture].y_scale >= 0) {
                    (*render_list)[*list_size].coords[0].y = +(*render_list)[*list_size].points[0].z * texture_data[texture].y_scale * map_data.resolution.z;
                    (*render_list)[*list_size].coords[1].y = +(*render_list)[*list_size].points[2].z * texture_data[texture].y_scale * map_data.resolution.z;
                  } else {
                    (*render_list)[*list_size].coords[0].y = +(*render_list)[*list_size].points[0].z * -texture_data[texture].y_scale;
                    (*render_list)[*list_size].coords[1].y = +(*render_list)[*list_size].points[2].z * -texture_data[texture].y_scale;
                  };
                break;
                case BLOCK_SIDE_LEFT:
                  (*render_list)[*list_size].points[0].x = chunk_size * chunk.x + chunk_size - c.z - 1;
                  (*render_list)[*list_size].points[0].y = chunk_size * chunk.y + chunk_size - c.x;
                  (*render_list)[*list_size].points[0].z = chunk_size * chunk.z + c.y;
                  (*render_list)[*list_size].points[1].x = chunk_size * chunk.x + chunk_size - c.z - 1;
                  (*render_list)[*list_size].points[1].y = chunk_size * chunk.y + chunk_size - c.x - area[a].s;
                  (*render_list)[*list_size].points[1].z = chunk_size * chunk.z + c.y;
                  (*render_list)[*list_size].points[2].x = chunk_size * chunk.x + chunk_size - c.z - 1;
                  (*render_list)[*list_size].points[2].y = chunk_size * chunk.y + chunk_size - c.x - area[a].s;
                  (*render_list)[*list_size].points[2].z = chunk_size * chunk.z + c.y + area[a].t;
                  (*render_list)[*list_size].points[3].x = chunk_size * chunk.x + chunk_size - c.z - 1;
                  (*render_list)[*list_size].points[3].y = chunk_size * chunk.y + chunk_size - c.x;
                  (*render_list)[*list_size].points[3].z = chunk_size * chunk.z + c.y + area[a].t;
                  if (!option_textures) {
                    (*render_list)[*list_size].coords[0].x = -(*render_list)[*list_size].points[0].y;
                    (*render_list)[*list_size].coords[1].x = -(*render_list)[*list_size].points[2].y;
                  } else if (texture_data[texture].x_scale >= 0) {
                    (*render_list)[*list_size].coords[0].x = -(*render_list)[*list_size].points[0].y * texture_data[texture].x_scale * map_data.resolution.y;
                    (*render_list)[*list_size].coords[1].x = -(*render_list)[*list_size].points[2].y * texture_data[texture].x_scale * map_data.resolution.y;
                  } else {
                    (*render_list)[*list_size].coords[0].x = -(*render_list)[*list_size].points[0].y * -texture_data[texture].x_scale;
                    (*render_list)[*list_size].coords[1].x = -(*render_list)[*list_size].points[2].y * -texture_data[texture].x_scale;
                  };
                  if (!option_textures) {
                    (*render_list)[*list_size].coords[0].y = +(*render_list)[*list_size].points[0].z;
                    (*render_list)[*list_size].coords[1].y = +(*render_list)[*list_size].points[2].z;
                  } else if (texture_data[texture].y_scale >= 0) {
                    (*render_list)[*list_size].coords[0].y = +(*render_list)[*list_size].points[0].z * texture_data[texture].y_scale * map_data.resolution.z;
                    (*render_list)[*list_size].coords[1].y = +(*render_list)[*list_size].points[2].z * texture_data[texture].y_scale * map_data.resolution.z;
                  } else {
                    (*render_list)[*list_size].coords[0].y = +(*render_list)[*list_size].points[0].z * -texture_data[texture].y_scale;
                    (*render_list)[*list_size].coords[1].y = +(*render_list)[*list_size].points[2].z * -texture_data[texture].y_scale;
                  };
                break;
                case BLOCK_SIDE_BACK:
                  (*render_list)[*list_size].points[0].x = chunk_size * chunk.x + chunk_size - c.x;
                  (*render_list)[*list_size].points[0].y = chunk_size * chunk.y + c.z + 1;
                  (*render_list)[*list_size].points[0].z = chunk_size * chunk.z + c.y;
                  (*render_list)[*list_size].points[1].x = chunk_size * chunk.x + chunk_size - c.x - area[a].s;
                  (*render_list)[*list_size].points[1].y = chunk_size * chunk.y + c.z + 1;
                  (*render_list)[*list_size].points[1].z = chunk_size * chunk.z + c.y;
                  (*render_list)[*list_size].points[2].x = chunk_size * chunk.x + chunk_size - c.x - area[a].s;
                  (*render_list)[*list_size].points[2].y = chunk_size * chunk.y + c.z + 1;
                  (*render_list)[*list_size].points[2].z = chunk_size * chunk.z + c.y + area[a].t;
                  (*render_list)[*list_size].points[3].x = chunk_size * chunk.x + chunk_size - c.x;
                  (*render_list)[*list_size].points[3].y = chunk_size * chunk.y + c.z + 1;
                  (*render_list)[*list_size].points[3].z = chunk_size * chunk.z + c.y + area[a].t;
                  if (!option_textures) {
                    (*render_list)[*list_size].coords[0].x = -(*render_list)[*list_size].points[0].x;
                    (*render_list)[*list_size].coords[1].x = -(*render_list)[*list_size].points[2].x;
                  } else if (texture_data[texture].x_scale >= 0) {
                    (*render_list)[*list_size].coords[0].x = -(*render_list)[*list_size].points[0].x * texture_data[texture].x_scale * map_data.resolution.x;
                    (*render_list)[*list_size].coords[1].x = -(*render_list)[*list_size].points[2].x * texture_data[texture].x_scale * map_data.resolution.x;
                  } else {
                    (*render_list)[*list_size].coords[0].x = -(*render_list)[*list_size].points[0].x * -texture_data[texture].x_scale;
                    (*render_list)[*list_size].coords[1].x = -(*render_list)[*list_size].points[2].x * -texture_data[texture].x_scale;
                  };
                  if (!option_textures) {
                    (*render_list)[*list_size].coords[0].y = +(*render_list)[*list_size].points[0].z;
                    (*render_list)[*list_size].coords[1].y = +(*render_list)[*list_size].points[2].z;
                  } else if (texture_data[texture].y_scale >= 0) {
                    (*render_list)[*list_size].coords[0].y = +(*render_list)[*list_size].points[0].z * texture_data[texture].y_scale * map_data.resolution.z;
                    (*render_list)[*list_size].coords[1].y = +(*render_list)[*list_size].points[2].z * texture_data[texture].y_scale * map_data.resolution.z;
                  } else {
                    (*render_list)[*list_size].coords[0].y = +(*render_list)[*list_size].points[0].z * -texture_data[texture].y_scale;
                    (*render_list)[*list_size].coords[1].y = +(*render_list)[*list_size].points[2].z * -texture_data[texture].y_scale;
                  };
                break;
                case BLOCK_SIDE_DOWN:
                  (*render_list)[*list_size].points[0].x = chunk_size * chunk.x + c.x;
                  (*render_list)[*list_size].points[0].y = chunk_size * chunk.y + chunk_size - c.y;
                  (*render_list)[*list_size].points[0].z = chunk_size * chunk.z + chunk_size - c.z - 1;
                  (*render_list)[*list_size].points[1].x = chunk_size * chunk.x + c.x + area[a].s;
                  (*render_list)[*list_size].points[1].y = chunk_size * chunk.y + chunk_size - c.y;
                  (*render_list)[*list_size].points[1].z = chunk_size * chunk.z + chunk_size - c.z - 1;
                  (*render_list)[*list_size].points[2].x = chunk_size * chunk.x + c.x + area[a].s;
                  (*render_list)[*list_size].points[2].y = chunk_size * chunk.y + chunk_size - c.y - area[a].t;
                  (*render_list)[*list_size].points[2].z = chunk_size * chunk.z + chunk_size - c.z - 1;
                  (*render_list)[*list_size].points[3].x = chunk_size * chunk.x + c.x;
                  (*render_list)[*list_size].points[3].y = chunk_size * chunk.y + chunk_size - c.y - area[a].t;
                  (*render_list)[*list_size].points[3].z = chunk_size * chunk.z + chunk_size - c.z - 1;
                  if (!option_textures) {
                    (*render_list)[*list_size].coords[0].x = +(*render_list)[*list_size].points[0].x;
                    (*render_list)[*list_size].coords[1].x = +(*render_list)[*list_size].points[2].x;
                  } else if (texture_data[texture].x_scale >= 0) {
                    (*render_list)[*list_size].coords[0].x = +(*render_list)[*list_size].points[0].x * texture_data[texture].x_scale * map_data.resolution.x;
                    (*render_list)[*list_size].coords[1].x = +(*render_list)[*list_size].points[2].x * texture_data[texture].x_scale * map_data.resolution.x;
                  } else {
                    (*render_list)[*list_size].coords[0].x = +(*render_list)[*list_size].points[0].x * -texture_data[texture].x_scale;
                    (*render_list)[*list_size].coords[1].x = +(*render_list)[*list_size].points[2].x * -texture_data[texture].x_scale;
                  };
                  if (!option_textures) {
                    (*render_list)[*list_size].coords[0].y = -(*render_list)[*list_size].points[0].y;
                    (*render_list)[*list_size].coords[1].y = -(*render_list)[*list_size].points[2].y;
                  } else if (texture_data[texture].y_scale >= 0) {
                    (*render_list)[*list_size].coords[0].y = -(*render_list)[*list_size].points[0].y * texture_data[texture].y_scale * map_data.resolution.y;
                    (*render_list)[*list_size].coords[1].y = -(*render_list)[*list_size].points[2].y * texture_data[texture].y_scale * map_data.resolution.y;
                  } else {
                    (*render_list)[*list_size].coords[0].y = -(*render_list)[*list_size].points[0].y * -texture_data[texture].y_scale;
                    (*render_list)[*list_size].coords[1].y = -(*render_list)[*list_size].points[2].y * -texture_data[texture].y_scale;
                  };
                break;
              };

              if (++*list_size >= max_list_entries) easy_fuck("max_list_entries exceeded!");

              for (int v = c.y; v < c.y + area[a].t; v++) {
                for (int u = c.x; u < c.x + area[a].s; u++) {
                  if (grid[u][v] >= 0) {
                    grid[u][v] = -1;
                    grid_count--;
                  };
                };
              };

            };
            NO_MATCH:;
          };
        };
      };

    };
  };

  lag_pop();

  if (light_flag) {
    memory_allocate(&l, 0);
  };

  memory_allocate(real_list, sizeof(struct structure_render_list) * (*list_size));
  memmove(*real_list, *render_list, sizeof(struct structure_render_list) * (*list_size));
  memory_allocate(render_list, 0);

  lag_pop();

};

//--page-split-- map_process_a_chunk

int map_process_a_chunk() {
  if (!map_initialization_flag) return 0;

  DEBUG("enter map_process_a_chunk()");

  int unprocessed_chunks = 0;
  int processed_a_chunk = 0;

  static int thread_index = 0;
  if (thread_index >= map_chunk_thread_count) thread_index = 0;

  if (map_chunk_thread_count) {

    int stop_index = thread_index;
    for (int j = thread_index;;) {

      int chunks = chunk_thread_data[j].write_index - chunk_thread_data[j].read_index;
      if (chunks < 0) chunks += CHUNK_RING_SIZE;
      unprocessed_chunks += chunks;

      if (!processed_a_chunk && chunks != 0) {

        chunk_map[chunk_thread_data[j].chunk_index[chunk_thread_data[j].read_index]] &= ~CM_COMPLETE;

        struct structure_render_list *render_list;
        int render_list_size;

        render_list = chunk_thread_data[j].render_list[chunk_thread_data[j].read_index];
        render_list_size = chunk_thread_data[j].list_size[chunk_thread_data[j].read_index];

        compile_display_list(chunk_list_base + chunk_thread_data[j].chunk_index[chunk_thread_data[j].read_index], render_list, render_list_size);

        DEBUG("enter easy_free(render_list)");
        memory_allocate(&chunk_thread_data[j].render_list[chunk_thread_data[j].read_index], 0);
        DEBUG("leave easy_free(render_list)");

        quad_count += render_list_size - chunk_counts[chunk_thread_data[j].chunk_index[chunk_thread_data[j].read_index]];
        chunk_counts[chunk_thread_data[j].chunk_index[chunk_thread_data[j].read_index]] = render_list_size;
        statistics_count_quads(quad_count);
        statistics_count_chunk();

        chunk_thread_data[j].read_index = (chunk_thread_data[j].read_index + 1) & (CHUNK_RING_SIZE - 1);

        processed_a_chunk = 1;
        unprocessed_chunks--;

        if (++j >= map_chunk_thread_count) j = 0;
        thread_index = j;

      } else {

        if (++j >= map_chunk_thread_count) j = 0;

      };

      if (j == stop_index) break;

    };

  } else {

    // If no threads, we'll have to do this ourselves...

    int distance, closest, priority;
    struct int_xyz chunk;

    closest = 1048576; priority = 0;
    for (int z = 0; z < chunk_dimension.z; z++) {
      for (int y = 0; y < chunk_dimension.y; y++) {
        for (int x = 0; x < chunk_dimension.x; x++) {
          if (chunk_map(x, y, z) & CM_DIRTY && (chunk_map(x, y, z) & (CM_PRIORITY | CM_VISIBLE)) >= priority) {
            unprocessed_chunks++;
            double xx = fabs(x * chunk_size + chunk_size / 2 - player_view.x);
            double yy = fabs(y * chunk_size + chunk_size / 2 - player_view.y);
            double zz = fabs(z * chunk_size + chunk_size / 2 - player_view.z);
            if (map_data.wrap.x && xx >= map_data.dimension.x / 2) xx -= map_data.dimension.x;
            if (map_data.wrap.y && yy >= map_data.dimension.y / 2) yy -= map_data.dimension.y;
            if (map_data.wrap.z && zz >= map_data.dimension.z / 2) zz -= map_data.dimension.z;
            distance = sqrt(pow(xx, 2) + pow(yy, 2) + pow(zz, 2));
            if ((chunk_map(x, y, z) & (CM_PRIORITY | CM_VISIBLE)) > priority || distance < closest) {
              closest = distance;
              priority = chunk_map(x, y, z) & (CM_PRIORITY | CM_VISIBLE);
              chunk.x = x;
              chunk.y = y;
              chunk.z = z;
            };
          };
        };
      };
    };

    if (unprocessed_chunks) {

      if (chunk_map(chunk.x, chunk.y, chunk.z) & CM_VIRGIN) complete_chunks++;
      chunk_map(chunk.x, chunk.y, chunk.z) &= ~(CM_DIRTY | CM_VIRGIN | CM_COMPLETE);
      statistics_complete_chunks(complete_chunks, chunk_limit >> 1);

      struct structure_render_list *render_list = NULL;
      int render_list_size;

      compile_chunk(chunk, &render_list, &render_list_size);

      compile_display_list(chunk_list_base + chunk_index(chunk.x, chunk.y, chunk.z), render_list, render_list_size);

      memory_allocate(&render_list, 0);

      quad_count += render_list_size - chunk_counts[chunk_index(chunk.x, chunk.y, chunk.z)];
      chunk_counts[chunk_index(chunk.x, chunk.y, chunk.z)] = render_list_size;
      statistics_count_quads(quad_count);
      statistics_count_chunk();

      processed_a_chunk = 1;
      unprocessed_chunks--;

    };

  };

  DEBUG("leave map_process_a_chunk()");
  return unprocessed_chunks;

};

//--page-split-- map_chunk_thread

void *map_chunk_thread(void *parameter) {

  thread_priority_background();

  int thread_index = (int) (long long) ((char *) parameter - (char *) chunk_thread_data) / sizeof(struct structure_chunk_thread_data);
  //printf("Thread %d starting!\n", thread_index + 1);

  struct structure_chunk_thread_data *my_data = parameter;

  while (!my_data->kill_switch) {

    // Wait until the ring buffer of render_list pointers has free space...

    while (!my_data->kill_switch && ((my_data->write_index + 1) & (CHUNK_RING_SIZE - 1)) == my_data->read_index) {
      if (my_data->kill_switch) break;
      easy_sleep(0.015);
    };
    if (my_data->kill_switch) break;

    // Then look for a chunk that requires compilation...

    int distance, closest, priority;
    struct int_xyz chunk;

    MUTEX_LOCK(chunk_map_mutex);

    closest = 1048576; priority = 0;
    for (int z = 0; z < chunk_dimension.z; z++) {
      for (int y = 0; y < chunk_dimension.y; y++) {
        for (int x = 0; x < chunk_dimension.x; x++) {
          if ((chunk_map(x, y, z) & (CM_DIRTY | CM_COMPLETE)) == CM_DIRTY && (chunk_map(x, y, z) & (CM_PRIORITY | CM_VISIBLE)) >= priority) {
            double xx = fabs(x * chunk_size + chunk_size / 2 - player_view.x);
            double yy = fabs(y * chunk_size + chunk_size / 2 - player_view.y);
            double zz = fabs(z * chunk_size + chunk_size / 2 - player_view.z);
            if (map_data.wrap.x && xx >= map_data.dimension.x / 2) xx -= map_data.dimension.x;
            if (map_data.wrap.y && yy >= map_data.dimension.y / 2) yy -= map_data.dimension.y;
            if (map_data.wrap.z && zz >= map_data.dimension.z / 2) zz -= map_data.dimension.z;
            distance = sqrt(pow(xx, 2) + pow(yy, 2) + pow(zz, 2));
            if ((chunk_map(x, y, z) & (CM_PRIORITY | CM_VISIBLE)) > priority || distance < closest) {
              priority = chunk_map(x, y, z) & (CM_PRIORITY | CM_VISIBLE);
              closest = distance;
              chunk.x = x;
              chunk.y = y;
              chunk.z = z;
            };
          };
        };
      };
    };

    if (closest == 1048576) {
      MUTEX_UNLOCK(chunk_map_mutex);
      easy_sleep(0.015);
      continue;
    };

    //printf("Found priority %d\n", priority);

    if (chunk_map(chunk.x, chunk.y, chunk.z) & CM_VIRGIN) complete_chunks++;
    statistics_complete_chunks(complete_chunks, chunk_limit >> 1);
    chunk_map(chunk.x, chunk.y, chunk.z) |= CM_COMPLETE;
    chunk_map(chunk.x, chunk.y, chunk.z) &= ~(CM_DIRTY | CM_VIRGIN | CM_PRIORITY);

    MUTEX_UNLOCK(chunk_map_mutex);

    my_data->chunk_index[my_data->write_index] = chunk_index(chunk.x, chunk.y, chunk.z);
    //printf("Calculating chunk %d in slot %d...\n", my_data->chunk_index[my_data->write_index], my_data->write_index);
    compile_chunk(chunk, &my_data->render_list[my_data->write_index], &my_data->list_size[my_data->write_index]);
    //printf("Rendered %d polygons to memory %u!\n", my_data->list_size[my_data->write_index], (unsigned int) my_data->render_list[my_data->write_index]);
    my_data->write_index = (my_data->write_index + 1) & (CHUNK_RING_SIZE - 1);

  };

  //printf("Thread %d terminating!\n", thread_index + 1);

  my_data->kill_switch = 0;

};

//--page-split-- map_render


void map_render() {

  int do_anaglyph = option_anaglyph_enable && option_pupil_distance > 0.0;

  int r_mask = 1, g_mask = 1, b_mask = 1;

  if (do_anaglyph) {
    if (RENDER_IN_GRAYSCALE) {
      r_mask = g_mask = b_mask = 0;
      r_mask |= ((int []) {1, 1, 0, 0, 0, 1})[option_anaglyph_filter[0]];
      g_mask |= ((int []) {0, 1, 1, 1, 0, 0})[option_anaglyph_filter[0]];
      b_mask |= ((int []) {0, 0, 0, 1, 1, 1})[option_anaglyph_filter[0]];
      r_mask |= ((int []) {1, 1, 0, 0, 0, 1})[option_anaglyph_filter[1]];
      g_mask |= ((int []) {0, 1, 1, 1, 0, 0})[option_anaglyph_filter[1]];
      b_mask |= ((int []) {0, 0, 0, 1, 1, 1})[option_anaglyph_filter[1]];
    };
  };

  if ((option_fog_type & 1) == 0) {
    glClearColor(0.05 * r_mask, 0.05 * g_mask, 0.05 * b_mask, 1.0);
    glFogfv(GL_FOG_COLOR, (GLfloat[]) {0.05 * r_mask, 0.05 * g_mask, 0.05 * b_mask, 1.0});
  } else if (RENDER_IN_GRAYSCALE) {
    glClearColor(0.8 * r_mask, 0.8 * g_mask, 0.8 * b_mask, 1.0);
    glFogfv(GL_FOG_COLOR, (GLfloat[]) {0.8 * r_mask, 0.8 * g_mask, 0.8 * b_mask, 1.0});
  } else if (option_fog_type == 1) {
    glClearColor(0.6 * r_mask, 0.8 * g_mask, 1.0 * b_mask, 1.0);
    glFogfv(GL_FOG_COLOR, (GLfloat[]) {0.6 * r_mask, 0.8 * g_mask, 1.0 * b_mask, 1.0});
  } else {
    glClearColor(0.75 * r_mask, 0.8 * g_mask, 0.85 * b_mask, 1.0);
    glFogfv(GL_FOG_COLOR, (GLfloat[]) {0.75 * r_mask, 0.8 * g_mask, 0.85 * b_mask, 1.0});
  };

  glClear(GL_COLOR_BUFFER_BIT);

  int sucky_fog = 0;
  if (option_fog_type & 2) {
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    if (option_fog_type & 4) {
      glFogf(GL_FOG_START, 0.5 * statistics_fog_distance);
    } else {
      glFogf(GL_FOG_START, 0.0 * statistics_fog_distance);
    };
    glFogf(GL_FOG_END, 1.0 * statistics_fog_distance);
    glHint(GL_FOG_HINT, GL_NICEST);
    glFogi(0x855A, 0x855B); // get better fog rendering on NVIDIA cards
    sucky_fog = glGetError(); // ignore GL errors if it isn't available
  } else {
    glDisable(GL_FOG);
  };

  for (int eye = (do_anaglyph ? 1 : 0); eye < (do_anaglyph ? 3 : 1); eye++) {
    //int r_mask = 1, g_mask = 1, b_mask = 1;
    if (eye && option_anaglyph_enable == 1) {
      r_mask = ((int []) {1, 1, 0, 0, 0, 1})[option_anaglyph_filter[eye - 1]];
      g_mask = ((int []) {0, 1, 1, 1, 0, 0})[option_anaglyph_filter[eye - 1]];
      b_mask = ((int []) {0, 0, 0, 1, 1, 1})[option_anaglyph_filter[eye - 1]];
    };

    glColorMask(r_mask ? GL_TRUE : GL_FALSE, g_mask ? GL_TRUE : GL_FALSE, b_mask ? GL_TRUE : GL_FALSE, GL_TRUE);

    glClear(GL_DEPTH_BUFFER_BIT);
    use_map_coordinates(eye); CHECK_GL_ERROR;

    if ((option_fog_type & 3) == 2) stars_render(); CHECK_GL_ERROR;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    struct double_xyzuv pp = player_view;
    if (map_data.wrap.x && pp.x >= map_data.dimension.x / 2) pp.x -= map_data.dimension.x;
    if (map_data.wrap.y && pp.y >= map_data.dimension.y / 2) pp.y -= map_data.dimension.y;
    if (map_data.wrap.z && pp.z >= map_data.dimension.z / 2) pp.z -= map_data.dimension.z;

    if (option_fog_type == 1) {
      glDepthMask(GL_FALSE);
      glEnable(GL_TEXTURE_2D);
      if (sky_box_type == 1 || sky_box_type == 2) {
        double x1, x2, y1, y2, z1, z2;
        if (sky_box_flags & 0x01) {
          x1 = 0.0;
          y1 = 0.0;
          z1 = map_data.sealevel;
          x2 = map_data.dimension.x;
          y2 = map_data.dimension.y;
          z2 = map_data.dimension.z;
        } else {
          x1 = pp.x - 1024;
          x2 = pp.x + 1024;
          y1 = pp.y - 1024;
          y2 = pp.y + 1024;
          z1 = pp.z + 0;
          z2 = pp.z + 1024;
        };
        glColor3f(1.0, 1.0, 1.0);
        glCallList(texture_list_base + sky_box_texture[0]);
        glBegin(GL_QUADS);
        // north
        glTexCoord2f(0.25, 0.00); glVertex3f(x1, y2, z1);
        glTexCoord2f(0.75, 0.00); glVertex3f(x2, y2, z1);
        glTexCoord2f(0.75, 0.25); glVertex3f(x2, y2, z2);
        glTexCoord2f(0.25, 0.25); glVertex3f(x1, y2, z2);
        // up
        glTexCoord2f(0.25, 0.25); glVertex3f(x1, y2, z2);
        glTexCoord2f(0.75, 0.25); glVertex3f(x2, y2, z2);
        glTexCoord2f(0.75, 0.75); glVertex3f(x2, y1, z2);
        glTexCoord2f(0.25, 0.75); glVertex3f(x1, y1, z2);
        // south
        glTexCoord2f(0.75, 1.00); glVertex3f(x2, y1, z1);
        glTexCoord2f(0.25, 1.00); glVertex3f(x1, y1, z1);
        glTexCoord2f(0.25, 0.75); glVertex3f(x1, y1, z2);
        glTexCoord2f(0.75, 0.75); glVertex3f(x2, y1, z2);
        // west
        glTexCoord2f(0.00, 0.75); glVertex3f(x1, y1, z1);
        glTexCoord2f(0.00, 0.25); glVertex3f(x1, y2, z1);
        glTexCoord2f(0.25, 0.25); glVertex3f(x1, y2, z2);
        glTexCoord2f(0.25, 0.75); glVertex3f(x1, y1, z2);
        // east
        glTexCoord2f(1.00, 0.25); glVertex3f(x2, y2, z1);
        glTexCoord2f(1.00, 0.75); glVertex3f(x2, y1, z1);
        glTexCoord2f(0.75, 0.75); glVertex3f(x2, y1, z2);
        glTexCoord2f(0.75, 0.25); glVertex3f(x2, y2, z2);
        glEnd();
      } else if (sky_box_type == 5 || sky_box_type == 6) {
        double x1, x2, y1, y2, z1, z2;
        if (sky_box_flags & 0x01) {
          x1 = 0.0;
          y1 = 0.0;
          z1 = 0.0;
          x2 = map_data.dimension.x;
          y2 = map_data.dimension.y;
          z2 = map_data.dimension.z;
        } else {
          x1 = pp.x - 1024;
          x2 = pp.x + 1024;
          y1 = pp.y - 1024;
          y2 = pp.y + 1024;
          z1 = pp.z - 1024;
          z2 = pp.z + 1024;
        };
        glColor3f(1.0, 1.0, 1.0);
        // west
        glCallList(texture_list_base + sky_box_texture[0]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex3f(x1, y1, z1);
        glTexCoord2f(1.0, 0.0); glVertex3f(x1, y2, z1);
        glTexCoord2f(1.0, 1.0); glVertex3f(x1, y2, z2);
        glTexCoord2f(0.0, 1.0); glVertex3f(x1, y1, z2);
        glEnd();
        // south
        glCallList(texture_list_base + sky_box_texture[1]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex3f(x2, y1, z1);
        glTexCoord2f(1.0, 0.0); glVertex3f(x1, y1, z1);
        glTexCoord2f(1.0, 1.0); glVertex3f(x1, y1, z2);
        glTexCoord2f(0.0, 1.0); glVertex3f(x2, y1, z2);
        glEnd();
        if (sky_box_type == 6) {
          // down
          glCallList(texture_list_base + sky_box_texture[2]);
          glBegin(GL_QUADS);
          glTexCoord2f(0.0, 0.0); glVertex3f(x1, y1, z1);
          glTexCoord2f(1.0, 0.0); glVertex3f(x2, y1, z1);
          glTexCoord2f(1.0, 1.0); glVertex3f(x2, y2, z1);
          glTexCoord2f(0.0, 1.0); glVertex3f(x1, y2, z1);
          glEnd();
        };
        // east
        glCallList(texture_list_base + sky_box_texture[3]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex3f(x2, y2, z1);
        glTexCoord2f(1.0, 0.0); glVertex3f(x2, y1, z1);
        glTexCoord2f(1.0, 1.0); glVertex3f(x2, y1, z2);
        glTexCoord2f(0.0, 1.0); glVertex3f(x2, y2, z2);
        glEnd();
        // north
        glCallList(texture_list_base + sky_box_texture[4]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex3f(x1, y2, z1);
        glTexCoord2f(1.0, 0.0); glVertex3f(x2, y2, z1);
        glTexCoord2f(1.0, 1.0); glVertex3f(x2, y2, z2);
        glTexCoord2f(0.0, 1.0); glVertex3f(x1, y2, z2);
        glEnd();
        // up
        glCallList(texture_list_base + sky_box_texture[5]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex3f(x1, y2, z2);
        glTexCoord2f(1.0, 0.0); glVertex3f(x2, y2, z2);
        glTexCoord2f(1.0, 1.0); glVertex3f(x2, y1, z2);
        glTexCoord2f(0.0, 1.0); glVertex3f(x1, y1, z2);
        glEnd();
      };
      glDisable(GL_TEXTURE_2D);
      glDepthMask(GL_TRUE);
    };

    // Draw "edge of map" texture.

    #if 0
    if (option_fog_type == 1 && sky_box_type != 0 && sky_box_flags & 0x01 == 0) {
      int x1 = 0.0;
      int y1 = 0.0;
      int z1 = 0.0;
      int x2 = map_data.dimension.x;
      int y2 = map_data.dimension.y;
      int z2 = map_data.sealevel;
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, texture_data[TEXTURE_MAP_EDGE].name);
      glColor3f(1.0, 1.0, 1.0);
      glBegin(GL_QUADS);
      glTexCoord2f(x1, y1); glVertex3f(x1, y1, z1);
      glTexCoord2f(x2, y1); glVertex3f(x2, y1, z1);
      glTexCoord2f(x2, y2); glVertex3f(x2, y2, z1);
      glTexCoord2f(x1, y2); glVertex3f(x1, y2, z1);
      glTexCoord2f(x1, z1); glVertex3f(x1, y2, z1);
      glTexCoord2f(x2, z1); glVertex3f(x2, y2, z1);
      glTexCoord2f(x2, z2); glVertex3f(x2, y2, z2);
      glTexCoord2f(x1, z2); glVertex3f(x1, y2, z2);
      glTexCoord2f(y1, z1); glVertex3f(x1, y1, z1);
      glTexCoord2f(y2, z1); glVertex3f(x1, y2, z1);
      glTexCoord2f(y2, z2); glVertex3f(x1, y2, z2);
      glTexCoord2f(y1, z2); glVertex3f(x1, y1, z2);
      glTexCoord2f(x1, z1); glVertex3f(x2, y1, z1);
      glTexCoord2f(x2, z1); glVertex3f(x1, y1, z1);
      glTexCoord2f(x2, z2); glVertex3f(x1, y1, z2);
      glTexCoord2f(x1, z2); glVertex3f(x2, y1, z2);
      glTexCoord2f(y1, z1); glVertex3f(x2, y2, z1);
      glTexCoord2f(y2, z1); glVertex3f(x2, y1, z1);
      glTexCoord2f(y2, z2); glVertex3f(x2, y1, z2);
      glTexCoord2f(y1, z2); glVertex3f(x2, y2, z2);
      glEnd();
      glDisable(GL_TEXTURE_2D);
    };
    #endif

    struct int_xyz pc;
    pc.x = ((int) floor(pp.x)) >> chunk_bits;
    pc.y = ((int) floor(pp.y)) >> chunk_bits;
    pc.z = ((int) floor(pp.z)) >> chunk_bits;

    // Determine where all chunk corners are, in respect to visibility on screen.
    // This is used later to determine which chunks to render.

    lag_push(1, "corner positions");
    int corner[chunk_dimension.x + 1][chunk_dimension.y + 1][chunk_dimension.z + 1];
    memset(corner, 0, sizeof(int) * (chunk_dimension.x + 1) * (chunk_dimension.y + 1) * (chunk_dimension.z + 1));
    double cos_u = cos(-pp.u); double sin_u = sin(-pp.u);
    double cos_v = cos(-pp.v); double sin_v = sin(-pp.v);
    double v_fac = tan(0.5 * map_perspective_angle * M_PI / 180);
    double h_fac = v_fac * display_window_width / display_window_height;
    for (int cz = 0; cz <= map_data.dimension.z; cz += chunk_size) {
      for (int cy = 0; cy <= map_data.dimension.y; cy += chunk_size) {
        for (int cx = 0; cx <= map_data.dimension.x; cx += chunk_size) {
          struct double_xyz c = {cx, cy, cz}; struct double_xyz t;
          if (map_data.wrap.x && c.x - pp.x >= map_data.dimension.x / 2) c.x -= map_data.dimension.x;
          if (map_data.wrap.y && c.y - pp.y >= map_data.dimension.y / 2) c.y -= map_data.dimension.y;
          if (map_data.wrap.z && c.z - pp.z >= map_data.dimension.z / 2) c.z -= map_data.dimension.z;
          c.x -= pp.x; c.x *= map_data.resolution.x;
          c.y -= pp.y; c.y *= map_data.resolution.y;
          c.z -= pp.z; c.z *= map_data.resolution.z;
          t.x = c.x * cos_u - c.y * sin_u; c.y = c.x * sin_u + c.y * cos_u; c.x = t.x;
          t.x = c.x * cos_v - c.z * sin_v; c.z = c.x * sin_v + c.z * cos_v; c.x = t.x;
          if (c.y > h_fac * c.x) corner[cx >> chunk_bits][cy >> chunk_bits][cz >> chunk_bits] |= 1;
          if (c.y < h_fac * -c.x) corner[cx >> chunk_bits][cy >> chunk_bits][cz >> chunk_bits] |= 2;
          if (c.z > v_fac * c.x) corner[cx >> chunk_bits][cy >> chunk_bits][cz >> chunk_bits] |= 4;
          if (c.z < v_fac * -c.x) corner[cx >> chunk_bits][cy >> chunk_bits][cz >> chunk_bits] |= 8;
          //if ((option_fog_type & 3)) {
            if (sucky_fog) {
              if (c.x > statistics_fog_distance) corner[cx >> chunk_bits][cy >> chunk_bits][cz >> chunk_bits] |= 16;
            } else {
              if (sqrt(c.x * c.x + c.y * c.y + c.z * c.z) > statistics_fog_distance) corner[cx >> chunk_bits][cy >> chunk_bits][cz >> chunk_bits] |= 16;
            };
          //};
        };
      };
    };
    lag_pop();

    // Make an array to track which chunks are actually rendered,
    // in order to test that ones out of view are not rendered.

    #if 0
    int fudge[chunk_dimension.x][chunk_dimension.y];
    memset(fudge, 0, sizeof(int) * chunk_dimension.x * chunk_dimension.y);
    int rendered_chunks = 0, total_chunks = 0;
    #endif

    CHECK_GL_ERROR;

    #if 0
    struct int_xyz block;
    block.x = floor(player_position.x);
    block.y = floor(player_position.y);
    block.z = floor(player_position.z);
    int type = map_get_block_type(block);
    if (type != 0 && block_data[type].impassable) {
      glFrontFace(GL_CW);
    } else {
      glFrontFace(GL_CCW);
    };
    #endif

    lag_push(1, "chunk visibility");
    glEnable(GL_MULTISAMPLE);
    int chunks_in_view = 0;
    for (int layer = 0; layer < chunk_limit; layer += chunk_dimension.x * chunk_dimension.y * chunk_dimension.z) {
      if (layer) glDepthMask(GL_FALSE);
      for (int z = 0; z < chunk_dimension.z; z++) {
        for (int y = 0; y < chunk_dimension.y; y++) {
          for (int x = 0; x < chunk_dimension.x; x++) {
            struct int_xyz cc = {x, y, z};
            // Let's see if any of the eight corners of the chunk should be visible,
            // and if not, then let's not render that chunk since it isn't visible.
            int invisible = -1;
            invisible &= corner[x+0][y+0][z+0];
            invisible &= corner[x+1][y+0][z+0];
            invisible &= corner[x+0][y+1][z+0];
            invisible &= corner[x+1][y+1][z+0];
            invisible &= corner[x+0][y+0][z+1];
            invisible &= corner[x+1][y+0][z+1];
            invisible &= corner[x+0][y+1][z+1];
            invisible &= corner[x+1][y+1][z+1];
            if (invisible) {
              chunk_map(cc.x, cc.y, cc.z) &= ~CM_VISIBLE;
            } else {
              chunk_map(cc.x, cc.y, cc.z) |= CM_VISIBLE;
              //glPushMatrix();
              //struct int_xyz translate = {};
              //if (cc.x < 0) {
              //  translate.x = -map_data.dimension.x;
              //  cc.x += chunk_dimension.x;
              //};
              //if (cc.y < 0) {
              //  translate.y = -map_data.dimension.y;
              //  cc.y += chunk_dimension.y;
              //};
              //if (cc.z < 0) {
              //  translate.z = -map_data.dimension.z;
              //  cc.z += chunk_dimension.z;
              //};
              chunks_in_view++;
              //glTranslated(translate.x, translate.y, translate.z);
              glCallList(chunk_list_base + layer + chunk_index(cc.x, cc.y, cc.z));
              if (glGetError()) printf("Error after chunk (%d, %d, %d)\n", cc.x, cc.y, cc.z);
              //glPopMatrix();
              //rendered_chunks++;
            };
            //total_chunks++;
          };
        };
      };
      if (layer) glDepthMask(GL_TRUE);
      else {
        CHECK_GL_ERROR; // after rendering map
        lag_push(1, "model_render()");
        model_render(); CHECK_GL_ERROR;
        lag_pop();
        lag_push(1, "projectile_render()");
        projectile_render(); CHECK_GL_ERROR;
        lag_pop();
      };
    };
    map_percentage_in_view = 100.0 * chunks_in_view / chunk_limit;
    glDisable(GL_MULTISAMPLE);
    lag_pop();

    CHECK_GL_ERROR;

    #if 0
    char buffer[1048576];
    char *pointer = buffer;
    pointer += sprintf(pointer, "\e[H");
    for (int y = chunk_dimension.y - 1; y >= 0; y--) {
      for (int x = 0; x < chunk_dimension.x; x++) {
        if (fudge[x][y]) {
          pointer += sprintf(pointer, "[]");
        } else {
          pointer += sprintf(pointer, "  ");
        };
      };
      pointer += sprintf(pointer, "\e[K\n");
    };
    int percent = round(100.0 * rendered_chunks / total_chunks);
    pointer += sprintf(pointer, "Rendered %d%% of chunks.\e[K\n", percent);
    pointer += sprintf(pointer, "map_perspective_angle = %0.1f.\e[K\n", map_perspective_angle);
    printf("%s", buffer);
    #endif

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    #if 0
    glPushMatrix();

    // Slightly back away from the scene, so that we can draw on top of it.
    struct double_xyz v, t;
    v.x = 1; v.y = 0; v.z = 0;
    t.x = v.x * cos(player_view.v) - v.z * sin(player_view.v);
    t.z = v.x * sin(player_view.v) + v.z * cos(player_view.v);
    v.x = t.x; v.z = t.z;
    t.x = v.x * cos(player_view.u) - v.y * sin(player_view.u);
    t.y = v.x * sin(player_view.u) + v.y * cos(player_view.u);
    v.x = t.x; v.y = t.y;
    glTranslated(-1.0/64 * v.x, -1.0/64 * v.y, -1.0/64 * v.z);
    #endif

    draw_selection();

    draw_cursor();

    //glPopMatrix();

  };
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_FOG);

};
