#include "everything.h"

#ifdef LINUX

#include <backtrace.h>

static int backtrace_known_count = 0;
static int backtrace_unknown_count = 0;
static struct backtrace_state *state;

#endif

//--page-split-- error_callback

#ifdef LINUX

static void error_callback (void *data, const char *message, int error_number) {
  if (error_number == -1) {
    fprintf(stderr, "If you want backtraces, you have to compile with -g\n\n");
    _Exit(1);
  } else {
    fprintf(stderr, "Backtrace error %d: %s\n", error_number, message);
  };
};

#endif

//--page-split-- full_callback

#ifdef LINUX

static int full_callback (void *data, uintptr_t pc, const char *pathname, int line_number, const char *function) {
  if (pathname != NULL || function != NULL || line_number != 0) {
    if (backtrace_unknown_count) {
      fprintf(stderr, "    ...\n");
      backtrace_unknown_count = 0;
    };
    const char *filename = rindex(pathname, '/');
    if (filename) filename++; else filename = pathname;
    fprintf(stderr, "  %s:%d -- %s\n", filename, line_number, function);
    backtrace_known_count++;
  } else {
    backtrace_unknown_count++;
  };
  return 0;
};

#endif

//--page-split-- sigsegv_handler

#ifdef LINUX

static void sigsegv_handler (int number) {
  fprintf(stderr, "\n*** Segmentation Fault ***\n\n");
  backtrace_full(state, 0, full_callback, error_callback, NULL);
  if (!backtrace_known_count) {
    if (backtrace_unknown_count) {
      fprintf(stderr, "Unfortunately libbacktrace only reports %d calls about which nothing is known.\n", backtrace_unknown_count);
    } else {
      fprintf(stderr, "Unfortunately libbacktrace isn't reporting anything to us.\n");
    };
  };
  printf("\n");
  _Exit(1);
};

#endif

//--page-split-- backtrace_initialize

#ifdef LINUX

void backtrace_initialize (void) {
  state = backtrace_create_state(NULL, 1, error_callback, NULL);
  signal(SIGSEGV, sigsegv_handler);
};

#endif
