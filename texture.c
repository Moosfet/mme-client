#include "everything.h"

int texture_list_base;

// Loads an image file into OpenGL, returns OpenGL texture name.

int texture_width;
int texture_height;
int texture_r;
int texture_g;
int texture_b;
int texture_a;

//--page-split-- texture_load

GLuint texture_load(char *file, int size, int flags) {

  // Loads a texture from file or memory.
  // If size == 0, then file is a pointer to a file name.
  // If size > 0, then file is a pointer to the file in memory.

  unsigned char *sort;
  int x, y, n;

  // Load the flippin' file, bitch!

  lag_push(100, "decompressing image data");
  if (size > 0) {
    sort = stbi_load_from_memory(file, size, &x, &y, &n, 4); n = 4;
  } else {
    sort = stbi_load(file, &x, &y, &n, 4); n = 4;
  };
  lag_pop();

  if (sort == NULL) {
    #ifdef TEST
    if (size > 0) {
      printf("Pointer: %p\n", file);
      printf("Size: %d\n", size);
    } else {
      printf("File: %s\n", file);
    };
    #endif
    printf("Image decoding error: %s\n", stbi_failure_reason());
    //easy_fuck("Error while loading texture!");
    return 0;
  };

  unsigned char *data = NULL;
  memory_allocate(&data, n * x * y);

  if (flags & TEXTURE_FLAG_NOFLIP) {
    memmove(data, sort, n * x * y);
  } else {
    for (int j = 0; j < y; j++) {
      memmove(data + j * n * x, sort + (y - j - 1) * n * x, n * x);
    };
  };

  stbi_image_free(sort);

  double r = 0;
  double g = 0;
  double b = 0;
  double a = 0;
  for (int i = 0; i < x * y; i++) {
    r += pow(data[4 * i + 0] / 255.0, 2.2);
    g += pow(data[4 * i + 1] / 255.0, 2.2);
    b += pow(data[4 * i + 2] / 255.0, 2.2);
    a += data[4 * i + 3];
  };
  texture_r = 256.0 * pow(r / (x * y), 0.4545);
  texture_g = 256.0 * pow(g / (x * y), 0.4545);
  texture_b = 256.0 * pow(b / (x * y), 0.4545);
  texture_a = a / (x * y);

  // Generate texture name and select that texture.

  GLuint name;
  glGenTextures(1, &name);
  glBindTexture(GL_TEXTURE_2D, name);

  // anisotropic filtering

  #ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
  #endif
  #ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
  #endif

  if (option_anisotropic_filtering) {
    float max = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max);
    if (max > 4.0f) {
      max = 4.0f;
    };
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max);
  };

  if (flags & TEXTURE_FLAG_PIXELATE) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  };
  if (flags & TEXTURE_FLAG_MIPMAP) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    lag_push(1000, "generating mipmaps");
    if (0 && ((x) & (x-1)) == 0 && ((y) & (y-1)) == 0) {
      printf("Mipmapping %d x %d image!\n", x, y);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      unsigned char *smaller = NULL; int level = 1;
      while (x > 1 || y > 1) {
        if (x > 1 && y > 1) {
          int nx = x >> 1;
          int ny = y >> 1;
          memory_allocate(&smaller, n * nx * ny);
          for (int yy = 0; yy < ny; yy++) {
            for (int xx = 0; xx < nx; xx++) {
              for (int c = 0; c < n; c++) {
                if (c < 3) {
                  int t = round(pow(0.25 * pow(data[((2*yy) * x + (2*xx)) * n + c], 2.2) + 0.25 * pow(data[((2*yy) * x + (2*xx+1)) * n + c], 2.2) + 0.25 * pow(data[((2*yy+1) * x + (2*xx)) * n + c], 2.2) + 0.25 * pow(data[((2*yy+1) * x + (2*xx+1)) * n + c], 2.2), 1/2.2));
                  if (t < 0) t = 0; if (t > 255) t = 255;
                  smaller[(yy * nx + xx) * n + c] = t;
                } else {
                  smaller[(yy * nx + xx) * n + c] = (data[((2*yy) * x + (2*xx)) * n + c] + data[((2*yy) * x + (2*xx+1)) * n + c] + data[((2*yy+1) * x + (2*xx)) * n + c] + data[((2*yy+1) * x + (2*xx+1)) * n + c] + 2) >> 2;
                };
              };
            };
          };
          x = nx;
          y = ny;
          memory_allocate(&data, 0);
          data = smaller; smaller = NULL;
        } else if (x > 1) {
          int nx = x >> 1;
          memory_allocate(&smaller, n * nx * y);
          for (int yy = 0; yy < y; yy++) {
            for (int xx = 0; xx < nx; xx++) {
              for (int c = 0; c < n; c++) {
                if (c < 3) {
                  int t = round(pow(0.5 * pow(data[(yy * x + (2*xx)) * n + c], 2.2) + 0.5 * pow(data[(yy * x + (2*xx+1)) * n + c], 2.2), 1/2.2));
                  if (t < 0) t = 0; if (t > 255) t = 255;
                  smaller[(yy * nx + xx) * n + c] = t;
                } else {
                  smaller[(yy * nx + xx) * n + c] = (data[(yy * x + (2*xx)) * n + c] + data[(yy * x + (2*xx+1)) * n + c]) >> 1;
                };
              };
            };
          };
          x = nx;
          memory_allocate(&data, 0);
          data = smaller; smaller = NULL;
        } else if (y > 1) {
          int ny = y >> 1;
          memory_allocate(&smaller, n * x * ny);
          for (int yy = 0; yy < ny; yy++) {
            for (int xx = 0; xx < x; xx++) {
              for (int c = 0; c < n; c++) {
                if (c < 3) {
                  int t = round(0.5 * pow(pow(data[((2*yy) * x + xx) * n + c], 2.2) + 0.5 * pow(data[((2*yy+1) * x + xx) * n + c], 2.2), 1/2.2));
                  if (t < 0) t = 0; if (t > 255) t = 255;
                  smaller[(yy * x + xx) * n + c] = t;
                } else {
                  int t = data[((2*yy) * x + xx) * n + c] + data[((2*yy+1) * x + xx) * n + c];
                  if (t & 1) {
                    if (easy_random(2)) {
                      t += 1;
                    } else {
                      t -= 1;
                    };
                  };
                  smaller[(yy * x + xx) * n + c] = (data[((2*yy) * x + xx) * n + c] + data[((2*yy+1) * x + xx) * n + c]) >> 1;
                };
              };
            };
          };
          y = ny;
          memory_allocate(&data, 0);
          data = smaller; smaller = NULL;
        };
        glTexImage2D(GL_TEXTURE_2D, level++, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      };
    } else {
      gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, x, y, GL_RGBA, GL_UNSIGNED_BYTE, data);
      //gluBuild2DMipmaps(GL_TEXTURE_2D, GL_SRGB_ALPHA, x, y, GL_RGBA, GL_UNSIGNED_BYTE, data);
    };
    lag_pop();
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  };

  memory_allocate(&data, 0);

  // Store the width and height, in case someone wants to know.

  texture_width = x; texture_height = y;

  // They should read them before this function is called again.

  return name;

};

GLuint texture_full_screen;
GLuint texture_half_screen;

struct structure_texture_data texture_data[TEXTURE_MAX_TEXTURES] = {};

//--page-split-- texture_list

void texture_list(int texture, char *file, int size, int flags) {
  if (texture_data[texture].name != 0) glDeleteTextures(1, &texture_data[texture].name);
  texture_data[texture].name = texture_load(file, size, flags);
  texture_data[texture].r = texture_r / 255.0;
  texture_data[texture].g = texture_g / 255.0;
  texture_data[texture].b = texture_b / 255.0;
  texture_data[texture].a = texture_a / 255.0;
  glNewList(texture_list_base + texture, GL_COMPILE);
  glBindTexture(GL_TEXTURE_2D, texture_data[texture].name);
  glEndList();
};

//--page-split-- texture_initialize

void texture_initialize() {
  extern char data_no_zero_png; extern int size_no_zero_png;
  texture_data[TEXTURE_NO_ZERO].memory = &data_no_zero_png;
  texture_data[TEXTURE_NO_ZERO].size = size_no_zero_png;
  texture_data[TEXTURE_NO_ZERO].flags = TEXTURE_FLAG_MIPMAP;
  texture_data[TEXTURE_NO_ZERO].alpha = 0.0;
  texture_data[TEXTURE_NO_ZERO].x_scale = -1;
  texture_data[TEXTURE_NO_ZERO].y_scale = -1;
  extern char data_unknown_png; extern int size_unknown_png;
  texture_data[TEXTURE_UNKNOWN].memory = &data_unknown_png;
  texture_data[TEXTURE_UNKNOWN].size = size_unknown_png;
  texture_data[TEXTURE_UNKNOWN].flags = TEXTURE_FLAG_MIPMAP;
  texture_data[TEXTURE_UNKNOWN].alpha = 0.0;
  texture_data[TEXTURE_UNKNOWN].x_scale = -1;
  texture_data[TEXTURE_UNKNOWN].y_scale = -1;
  extern char data_loading_png; extern int size_loading_png;
  texture_data[TEXTURE_LOADING].memory = &data_loading_png;
  texture_data[TEXTURE_LOADING].size = size_loading_png;
  texture_data[TEXTURE_LOADING].flags = TEXTURE_FLAG_MIPMAP;
  texture_data[TEXTURE_LOADING].alpha = 0.0;
  texture_data[TEXTURE_LOADING].x_scale = -1;
  texture_data[TEXTURE_LOADING].y_scale = -1;
  extern char data_invalid_png; extern int size_invalid_png;
  texture_data[TEXTURE_INVALID].memory = &data_invalid_png;
  texture_data[TEXTURE_INVALID].size = size_invalid_png;
  texture_data[TEXTURE_INVALID].flags = TEXTURE_FLAG_MIPMAP;
  texture_data[TEXTURE_INVALID].alpha = 0.0;
  texture_data[TEXTURE_INVALID].x_scale = -1;
  texture_data[TEXTURE_INVALID].y_scale = -1;
  extern char data_sweet_u_png; extern int size_sweet_u_png;
  texture_data[TEXTURE_SWEET_U].memory = &data_sweet_u_png;
  texture_data[TEXTURE_SWEET_U].size = size_sweet_u_png;
  texture_data[TEXTURE_SWEET_U].flags = TEXTURE_FLAG_MIPMAP;
  texture_data[TEXTURE_SWEET_U].alpha = 0.0;
  texture_data[TEXTURE_SWEET_U].x_scale = -1;
  texture_data[TEXTURE_SWEET_U].y_scale = -1;
  extern char data_sweet_s_png; extern int size_sweet_s_png;
  texture_data[TEXTURE_SWEET_S].memory = &data_sweet_s_png;
  texture_data[TEXTURE_SWEET_S].size = size_sweet_s_png;
  texture_data[TEXTURE_SWEET_S].flags = TEXTURE_FLAG_MIPMAP;
  texture_data[TEXTURE_SWEET_S].alpha = 0.0;
  texture_data[TEXTURE_SWEET_S].x_scale = -1;
  texture_data[TEXTURE_SWEET_S].y_scale = -1;
  extern char data_sweet_d_png; extern int size_sweet_d_png;
  texture_data[TEXTURE_SWEET_D].memory = &data_sweet_d_png;
  texture_data[TEXTURE_SWEET_D].size = size_sweet_d_png;
  texture_data[TEXTURE_SWEET_D].flags = TEXTURE_FLAG_MIPMAP;
  texture_data[TEXTURE_SWEET_D].alpha = 0.0;
  texture_data[TEXTURE_SWEET_D].x_scale = -1;
  texture_data[TEXTURE_SWEET_D].y_scale = -1;
  extern char data_map_edge_png; extern int size_map_edge_png;
  texture_data[TEXTURE_MAP_EDGE].memory = &data_map_edge_png;
  texture_data[TEXTURE_MAP_EDGE].size = size_map_edge_png;
  texture_data[TEXTURE_MAP_EDGE].flags = TEXTURE_FLAG_MIPMAP;
  texture_data[TEXTURE_MAP_EDGE].alpha = 0.0;
  texture_data[TEXTURE_MAP_EDGE].x_scale = -1;
  texture_data[TEXTURE_MAP_EDGE].y_scale = -1;
};

//--page-split-- texture_open_window

void texture_open_window() {
  texture_list_base = glGenLists(TEXTURE_MAX_TEXTURES);
  extern char data_full_screen_png; extern int size_full_screen_png;
  texture_full_screen = texture_load(&data_full_screen_png, size_full_screen_png, TEXTURE_FLAG_PIXELATE);
  extern char data_half_screen_png; extern int size_half_screen_png;
  texture_half_screen = texture_load(&data_half_screen_png, size_half_screen_png, TEXTURE_FLAG_PIXELATE);
  extern char data_invalid_0_png; extern int size_invalid_0_png;
  for (int i = 0; i < TEXTURE_MAX_TEXTURES; i++) {
    if (texture_data[i].name == 0) {
      if (texture_data[i].memory != NULL) {
        texture_data[i].name = texture_load(texture_data[i].memory, texture_data[i].size, texture_data[i].flags);
      } else if (texture_data[i].file != NULL) {
        texture_data[i].name = texture_load(texture_data[i].file, 0, texture_data[i].flags);
      };
    };
    glNewList(texture_list_base + i, GL_COMPILE);
    glBindTexture(GL_TEXTURE_2D, texture_data[i].name);
    glEndList();
  };
};

//--page-split-- texture_close_window

void texture_close_window() {
  for (int i = 0; i < TEXTURE_MAX_TEXTURES; i++) {
    memory_allocate(&texture_data[i].digest, 0);
    if (texture_data[i].name != 0) {
      glDeleteTextures(1, &texture_data[i].name);
      texture_data[i].name = 0;
    };
  };
  glDeleteLists(texture_list_base, TEXTURE_MAX_TEXTURES);
};

//--page-split-- texture_reset

void texture_reset() {
  for (int i = 0; i < TEXTURE_MAX_SERVER_TEXTURES; i++) {
    memory_allocate(&texture_data[i].digest, 0);
    memory_allocate(&texture_data[i].file, 0);
    if (texture_data[i].name != 0) {
      glDeleteTextures(1, &texture_data[i].name);
      texture_data[i].name = 0;
    };
    texture_data[i].memory = NULL;
    texture_data[i].size = 0;
  };
};
