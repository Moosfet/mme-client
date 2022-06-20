#include "everything.h"

//--page-split-- easy_fuck

void easy_fuck(char * message) {
  printf("Oh no!  %s\n", message);
  exit(1);
};

//--page-split-- easy_error

void easy_error(char * message, int error_code) {
  printf("%s: %s\n", message, error_string(error_code));
  exit(1);
};

//--page-split-- easy_time

double easy_time() {
  #ifdef WINDOWS
    return glfwGetTime();
  #else
    // glfwGetTime uses gettimeofday() which is less desriable than
    // CLOCK_MONOTONIC since changing the system time affects
    // gettimeofday() but does not affect CLOCK_MONOTONIC.
    struct timespec bullshit;
    clock_gettime(CLOCK_MONOTONIC, &bullshit);
    return bullshit.tv_sec + bullshit.tv_nsec / 1000000000.0;
  #endif
};

//--page-split-- easy_sleep

void easy_sleep(double seconds) {
  #ifdef WINDOWS
    int milliseconds = round(1000 * seconds);
    if (milliseconds > 0) Sleep(milliseconds);
  #else
    struct timespec bullshit;
    if (seconds > 0) {
      bullshit.tv_sec = floor(seconds);
      bullshit.tv_nsec = round((seconds - bullshit.tv_sec) * 1000000000.0);
      while (nanosleep(&bullshit, &bullshit));
    };
  #endif
};

//--page-split-- easy_seed_random_number_generator

void easy_seed_random_number_generator() {
  static int only_do_it_once = 0;
  if (!only_do_it_once) {
    only_do_it_once = 1;

    int one, two;
    time_t fuck; time(&fuck); one = fuck;

    #ifdef WINDOWS
      two = GetTickCount();
    #else
      two = floor(0x80000000 * fmod(easy_time(), 1.0));
    #endif

    // Since both "one" and "two" are most random in least significant bits,
    // we'll reverse the bits in one of them, so that when the two are then
    // XOR together, the randomness is more consistant.

    int reverse = 0;
    for (int i = 0; i < 31; i++) {
      reverse |= ((two >> i) & 1) << (30 - i);
    };

    int seed = one ^ reverse;

    //printf("one=%.10d  two=%.10d  reverse=%.10d  seed=%.10d\n", one, two, reverse, seed);

    srand(seed);

  };
};

//--page-split-- easy_random

int easy_random(int limit) {

  // creates random number between 0 and limit - 1

  if (limit < 2) easy_fuck("invalid random number limit");

  if (RAND_MAX < limit - 1) easy_fuck("C is so unprepared to accomplish anything.");

  int bits = 1 + floor(log(limit-1) / log(2));
  int mask = (1 << bits) - 1;

  //printf("limit = %d, bits = %d, mask = %d\n", limit, bits, mask);

  int random;
  do {
    random = rand() & mask;
  } while (random >= limit);

  return random;

};

//--page-split-- easy_random_binary_string

void easy_random_binary_string(char *string, int bytes) {
  #ifdef WINDOWS
    easy_seed_random_number_generator();
    if (RAND_MAX < 255) easy_fuck("C is so unprepared to accomplish anything.");
    for (int i = 0; i < bytes; i++) {
      string[i] = rand() & 0xFF;
    };
  #else
    FILE *urandom;
    urandom = fopen("/dev/urandom", "rb");
    if (0 == urandom) easy_fuck("cannot open /dev/urandom");
    if (0 == fread(string, bytes, 1, urandom)) easy_fuck("cannot read /dev/urandom");
    fclose(urandom);
  #endif
};

//--page-split-- easy_random_hex_string

void easy_random_hex_string(char *string, int length) {
  // Generates a text string of random hex characters.
  // If "length" is even, * DOES NOT ADD A NULL BYTE TO END OF STRING *
  // If "length" is odd, last position is filled with a null byte.
  char buffer[length >> 1];
  easy_random_binary_string(buffer, length >> 1);
  easy_binary_to_ascii(string, buffer, length >> 1);
  if (length % 2) string[length - 1] = 0;
};

//--page-split-- easy_binary_to_ascii

void easy_binary_to_ascii(char *output, char *input, int size) {
  const char hex[16] = "0123456789abcdef";
  // Converts binary string to hexadecimal ascii string.  Size is of input.
  for (int i = 0; i < size; i++) {
    *output++ = hex[(*input >> 4) & 15];
    *output++ = hex[*input++ & 15];
  };
};

//--page-split-- easy_strnlen

int easy_strnlen(char *string, int limit) {
  int i;
  for (i = 0; i < limit; i++) {
    if (string[i] == 0) break;
  };
  return i;
};
