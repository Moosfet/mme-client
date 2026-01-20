// this isn't even used anymore is it?

#if 0

#include "everything.h"

struct memory_double_linked_list {
  void *prev;
  void *next;
  char *file;
  int line;
  int size;
};

struct structure_rwlock memory_lock;

#if BITS == 32
  #define SIZE 20
  #define prev(X) (  *( (void **) (  ((void *) (X)) - 20  ) )  )
  #define next(X) (  *( (void **) (  ((void *) (X)) - 16  ) )  )
  #define file(X) (  *( (char **) (  ((void *) (X)) - 12  ) )  )
  #define line(X) (  *( (int *)   (  ((void *) (X)) - 8   ) )  )
  #define size(X) (  *( (int *)   (  ((void *) (X)) - 4   ) )  )
  #define test(X) (  *( (int *)   (  ((void *) (X)) + size(X))))
#else
  #define SIZE 32
  #define prev(X) (  *( (void **) (  ((void *) (X)) - 32  ) )  )
  #define next(X) (  *( (void **) (  ((void *) (X)) - 24  ) )  )
  #define file(X) (  *( (char **) (  ((void *) (X)) - 16  ) )  )
  #define line(X) (  *( (int *)   (  ((void *) (X)) - 8   ) )  )
  #define size(X) (  *( (int *)   (  ((void *) (X)) - 4   ) )  )
  #define test(X) (  *( (int *)   (  ((void *) (X)) + size(X))))
#endif

static struct memory_double_linked_list first_data, last_data;
static struct memory_double_linked_list first_data = {NULL, ((void *) &last_data) + SIZE, "the first item in the list", 0, 0};
static struct memory_double_linked_list last_data = {((void *) &first_data) + SIZE, NULL, "the last item in the list", 0, 0};
static void *first = ((void *) &first_data) + SIZE;
static void *last = ((void *) &last_data) + SIZE;

static int total_malloc_size = 0;
static int peak_malloc_size = 0;

//--page-split-- canary_test

static void canary_test(void) {
  lag_push(1, "canary_test()");
  thread_lock_read(&memory_lock);
  for (void *pointer = next(first); pointer != last; pointer = next(pointer)) {
    if (test(pointer) != build_time_utc) {
      printf("WRT memory allocated from %s line %d:\n", file(pointer), line(pointer));
      easy_fuck("Canary value has been destroyed!");
    };
  };
  thread_unlock_read(&memory_lock);
  lag_pop();
};

//--page-split-- check_total

static void check_total(void) {
  if (total_malloc_size > peak_malloc_size) {
    peak_malloc_size = total_malloc_size;
  };
  int megabytes = round(peak_malloc_size / 1048576.0);
  static int last_mb = 0;
  if (megabytes > last_mb) {
    printf("Peak malloc() total so far: %d MB\n", megabytes);
    last_mb = megabytes;
  };
};

//--page-split-- memory__allocate

void memory__allocate(void **pointer, int size, char *file, int line) {
  lag_push(1, "memory_allocate()");
  canary_test();
  if (*pointer == NULL && size != 0) {
    void *new = malloc(size + SIZE + 4);
    if (new == NULL) {
      printf("memory_allocate(%p, %d) in %s line %d\n", *pointer, size, file, line);
      easy_fuck("Failure to reallocate memory!");
    };
    *pointer = new + SIZE;
    thread_lock_write(&memory_lock);
    total_malloc_size += size;
    prev(*pointer) = prev(last);
    next(*pointer) = last;
    next(prev(*pointer)) = *pointer;
    prev(next(*pointer)) = *pointer;
    file(*pointer) = file;
    line(*pointer) = line;
    size(*pointer) = size;
    test(*pointer) = build_time_utc;
    thread_unlock_write(&memory_lock);
    check_total();
  } else if (*pointer != NULL && size != 0) {
    thread_lock_write(&memory_lock);
    total_malloc_size -= size(*pointer);
    void *_prev = prev(*pointer);
    void *_next = next(*pointer);
    char *_file = file(*pointer);
    int _line = line(*pointer);
    void *new = realloc(*pointer - SIZE, size + SIZE + 4);
    if (new == NULL) {
      printf("memory_allocate(%p, %d) in %s line %d\n", *pointer, size, file, line);
      easy_fuck("Failure to reallocate memory!");
    };
    *pointer = new + SIZE;
    prev(*pointer) = _prev;
    next(*pointer) = _next;
    file(*pointer) = _file;
    line(*pointer) = _line;
    prev(_next) = *pointer;
    next(_prev) = *pointer;
    size(*pointer) = size;
    test(*pointer) = build_time_utc;
    total_malloc_size += size(*pointer);
    thread_unlock_write(&memory_lock);
    check_total();
  } else if (*pointer != NULL && size == 0) {
    thread_lock_write(&memory_lock);
    total_malloc_size -= size(*pointer);
    next(prev(*pointer)) = next(*pointer);
    prev(next(*pointer)) = prev(*pointer);
    thread_unlock_write(&memory_lock);
    free(*pointer - SIZE);
    *pointer = NULL;
  };
  #ifdef VERBOSE
    printf("...and the memory_allocate(%p, %d) did not crash!\n", *pointer, size);
  #endif
  lag_pop();
};

static int report_flag = 0;

//--page-split-- memory_initialize

void memory_initialize(void) {
  thread_lock_init(&memory_lock);
};

#ifdef TEST

//--page-split-- memory_terminate

void memory_terminate(void) {
  thread_lock_read(&memory_lock);
  printf("Memory leak report:\n");
  int total = 0;
  for (void *pointer = next(first); pointer != last; pointer = next(pointer)) {
    printf("Some %d bytes remain allocated from %s line %d.\n", size(pointer), file(pointer), line(pointer));
    total += size(pointer);
  };
  printf("Total memory leak: %d bytes\n", total);
  thread_unlock_read(&memory_lock);
};
#endif

#endif
