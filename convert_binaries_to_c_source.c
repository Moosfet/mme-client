#include "everything.h"

void main (int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Specify input files on command line.\n");
    exit(1);
  };

  for (int i = 1; i < argc; i++) {
    char *name;
    char *p = rindex(argv[i], '/');
    if (p) p++;
    else p = name;
    asprintf(&name, "%s", p);

    // create variable name by replacing bad chars with _
    for (int i = 0; name[i]; i++) {
      if (name[i] < '0') name[i] = '_';
      if (name[i] > '9' && name[i] < 'A') name[i] = '_';
      if (name[i] > 'Z' && name[i] < 'a') name[i] = '_';
      if (name[i] > 'z') name[i] = '_';
    };

    printf("unsigned char data_%s[] = \"", name);

    FILE *file = fopen(argv[i], "rb");
    if (!file) fprintf(stderr, "Failed to open %s: %s\n", argv[i], strerror(errno));
    int size = 0;
    unsigned char buffer[4096];
    int avoid_digit = 0;
    int avoid_hex_digit = 0;
    int count;
    do {
      count = fread(buffer, 1, 4096, file);
      if (count < 0) {
        fprintf(stderr, "Error reading %s: %s\n", argv[i], strerror(errno));
      } else if (count > 0) {
        size += count;
        for (int i = 0; i < count; i++) {
          // represet the byte with the fewest chars possible
          // I may have wasted too much time writing this
          if (buffer[i] == '\a') {
            printf("\\a"); avoid_digit = avoid_hex_digit = 0;
          } else if (buffer[i] == '\b') {
            printf("\\b"); avoid_digit = avoid_hex_digit = 0;
          } else if (buffer[i] == '\t') {
            printf("\\t"); avoid_digit = avoid_hex_digit = 0;
          } else if (buffer[i] == '\n') {
            printf("\\n"); avoid_digit = avoid_hex_digit = 0;
          } else if (buffer[i] == '\v') {
            printf("\\v"); avoid_digit = avoid_hex_digit = 0;
          } else if (buffer[i] == '\f') {
            printf("\\f"); avoid_digit = avoid_hex_digit = 0;
          } else if (buffer[i] == '\r') {
            printf("\\r"); avoid_digit = avoid_hex_digit = 0;
          } else if (buffer[i] == '\e') {
            printf("\\e"); avoid_digit = avoid_hex_digit = 0;
          } else if (buffer[i] == '"') {
            printf("\\\""); avoid_digit = avoid_hex_digit = 0;
          } else if (buffer[i] == '\\') {
            printf("\\\\"); avoid_digit = avoid_hex_digit = 0;
          } else if (buffer[i] < ' ') {
            printf("\\%o", buffer[i]);
            avoid_digit = 1; avoid_hex_digit = 0;
          } else if (buffer[i] > '~') {
            printf("\\x%X", buffer[i]);
            avoid_digit = avoid_hex_digit = 1;
          } else if (avoid_digit && buffer[i] >= '0' && buffer[i] <= '9') {
            printf("\"\"%c", buffer[i]);
            avoid_digit = avoid_hex_digit = 0;
          } else if (avoid_hex_digit && buffer[i] >= 'A' && buffer[i] <= 'F') {
            printf("\"\"%c", buffer[i]);
            avoid_digit = avoid_hex_digit = 0;
          } else if (avoid_hex_digit && buffer[i] >= 'a' && buffer[i] <= 'f') {
            printf("\"\"%c", buffer[i]);
            avoid_digit = avoid_hex_digit = 0;
          } else {
            printf("%c", buffer[i]);
            avoid_digit = avoid_hex_digit = 0;
          };
        };
      };
    } while (count);

    printf("\";\n");
    printf("int size_%s = %d;\n", name, size);

    free(name);
    fclose(file);
  };

};
