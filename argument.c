#include "everything.h"

int argument_packets = 0;
int argument_server = 0;
int argument_modes = 0;
int argument_no_threads = 0;
int argument_fullscreen = 0;

#ifdef TEST
int argument_lag = 0;
int argument_hacks = 1;
int argument_record_all_frames = 0;
#endif

//--page-split-- help

static void help(void) {
  extern char data_help_txt; extern int size_help_txt;
  printf("\n");
  fwrite(&data_help_txt, size_help_txt, 1, stdout);
  printf("\n");
  exit(1);
};

//--page-split-- server_parse

static void server_parse (char **destination, char *server) {
  int length = strlen(server);
  if (server[0] == '[') {
    if (server[length-1] == ']') {
      memory_allocate(destination, length + 7);
      strcpy(*destination, server);
      strcpy(*destination + length, ":44434");
    } else {
      memory_allocate(destination, length + 1);
      strcpy(*destination, server);
    };
  } else if (strchr(server, ':') != strrchr(server, ':')) {
    memory_allocate(destination, length + 9);
    *destination[0] = '[';
    strcpy(*destination + 1, server);
    *destination[length + 1] = ']';
    strcpy(*destination + length + 2, ":44434");
  } else {
    char *split = strrchr(server, ':');
    if (split == NULL) {
      memory_allocate(destination, length + 7);
      strcpy(*destination, server);
      strcpy(*destination + length, ":44434");
    } else {
      memory_allocate(destination, length + 1);
      strcpy(*destination, server);
    };
  };
  argument_server = 1;
  server_action = SERVER_ACTION_CONNECT;
  menu_function_pointer = menus_server_connect;
};

//--page-split-- argument_parse

static void argument_parse (int argc, char **argv) {

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0) help();
    #ifdef TEST
    else if (strcmp(argv[i], "--lag") == 0) argument_lag = 1;
    else if (strcmp(argv[i], "--hacks") == 0) argument_hacks = 1;
    else if (strcmp(argv[i], "--no_threads") == 0) argument_no_threads = 1;
    else if (strcmp(argv[i], "--record_all_frames") == 0) argument_record_all_frames = 1;
    #endif
    else if (strcmp(argv[i], "--data") == 0) menu_display_data = 1;
    else if (strcmp(argv[i], "--packets") == 0) argument_packets = 1;
    else if (strcmp(argv[i], "--modes") == 0) argument_modes = 1;
    // Why does this exist?  We can just press F11 after it starts up.
    //else if (strcmp(argv[i], "--fullscreen") == 0) argument_fullscreen = 1;
    else if (strcmp(argv[i], "--server") == 0) {
      if (i + 1 < argc) {
        server_parse(&server_address, argv[++i]);
      } else {
        printf("Option --server requires an argument.\n");
        exit(1);
      };
    } else if (strcmp(argv[i], "--portal") == 0) {
      if (i + 1 < argc) {
        server_parse(&server_portal_address, argv[++i]);
      } else {
        printf("Option --protal requires an argument.\n");
        exit(1);
      };
    } else {
      printf("\nUnrecognized option '%s'\n", argv[i]);
      help();
    };
  };
};

//--page-split-- argument_check

void argument_check (int argc, char **argv) {

  char *environment = getenv("MME_OPTIONS");
  if (environment != NULL) {
    int length = strlen(environment);
    char *string = NULL;
    memory_allocate(&string, length + 1);
    memmove(string, environment, length + 1);

    int envc = 1;
    char *envv[64];

    envv[envc] = string;
    envc++;

    for (int i = 0; i < length; i++) {
      if (string[i] == ' ') {
        string[i] = 0;
        envv[envc] = string + i + 1;
        envc++;
      };
    };

    argument_parse(envc, envv);

    memory_allocate(&string, 0);

  };

  argument_parse(argc, argv);

};
