#include "everything.h"

char *cache_data = NULL;
int cache_size = 0;

//--page-split-- cache_validate

int cache_validate(char *digest) {

  // Checks cache for a file with a name matching the SHA-1 message digest.
  // If file is found, verifies that the file contents match the SHA-1.
  // If both steps are successful, returns 1, and cache_data/cache_size
  // point to the data from the cache.  Otherwise returns 0.

  char ascii[41], pathname[64]; ascii[40] = 0;
  easy_binary_to_ascii(ascii, digest, 20);
  sprintf(pathname, "cache/%s", ascii);

  memory_allocate(&cache_data, 0); cache_size = 0;

  FILE *file = fopen(pathname, "rb");

  if (file == NULL) return 0;

  int return_value;
  do {
    memory_allocate(&cache_data, cache_size + 65536);
    return_value = fread(cache_data + cache_size, 1, 65536, file);
    cache_size += return_value;
  } while (return_value != 0);

  fclose(file);

  memory_allocate(&cache_data, cache_size);

  char verify[20];
  sha1(verify, cache_data, cache_size);

  if (memcmp(digest, verify, 20) == 0) return 1;

  memory_allocate(&cache_data, 0); cache_size = 0;

  // If the data has a different SHA-1 sum, then rather than delete the data
  // (since it is invalid), let's just rename it to the correct file name.

  char correct[41], newname[64]; correct[40] = 0;
  easy_binary_to_ascii(correct, verify, 20);
  sprintf(newname, "cache/%s", correct);
  rename(pathname, newname); remove(pathname);

  return 0;

};
