/*
  Released under the antivirial license.  Basically, you can do anything
  you want with it as long as what you want doesn't involve the GNU GPL.
  See http://www.ecstaticlyrics.com/antiviral/ for more information.
*/

#ifndef EASY_H
#define EASY_H

#ifdef WINDOWS
#ifndef WINVER
#define WINVER 0x0501
#endif
#include <windows.h>
void *memmem(const void *, size_t, const void *, size_t);
char *index(const char *, int);
char *rindex(const char *, int);
#endif

#define _GNU_SOURCE
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>  // for O_RDONLY and shit

#define memmove(a, b, c) memmove(a, b, (unsigned) (c))

#define EASY_TYPE_CHAR 1
#define EASY_TYPE_SHORT 2
#define EASY_TYPE_INT 3
#define EASY_TYPE_LONG 4
#define EASY_TYPE_INT64 5
#define EASY_TYPE_FLOAT 6
#define EASY_TYPE_DOUBLE 7
#define EASY_TYPE_STRING 8
#define EASY_TYPE_EASY_STRING 9

#define easy_initialize() easy__initialize(__FILE__, __LINE__)
void easy__initialize(char *file, int line);
#define easy_terminate() easy__terminate(__FILE__, __LINE__)
void easy__terminate(char *file, int line);

struct easy_string {
  int allocation_size;
  int length;
  union {
    char *buffer;
    const char *const_buffer;
  };
};

struct easy_string_array {
  int count;
  struct easy_string *string;
};

struct easy_string_aa {
  int count;
  char **key;
  struct easy_string *value;
};

#ifdef ASAN
  #define EASY_RAW_MALLOC
#endif

#ifdef EASY_RAW_MALLOC
  #define easy_memory_allocate(pointer, size) { int _easy_size = (size); if (_easy_size) { *(pointer) = realloc(*(pointer), _easy_size); } else { free(*(pointer)); *(pointer) = NULL; }; }
  #define easy_memory__allocate(pointer, size, file, line) { int _easy_size = (size); if (_easy_size) { *(pointer) = realloc(*(pointer), _easy_size); } else { free(*(pointer)); *(pointer) = NULL; }; }
  #define easy_memory_allow(count, size)
#else
  #define easy_memory_allocate(pointer, size) { void *__d95NNwASESLjBSXFn3k2__ = *(pointer); easy_memory__allocate(&__d95NNwASESLjBSXFn3k2__, size, __FILE__, __LINE__); *(pointer) = __d95NNwASESLjBSXFn3k2__; }
  #define easy_memory_canary_check(pointer) easy_memory__canary_check(__FILE__, __LINE__, pointer)
  #define easy_memory_allow(count, size) { static int once = 0; if (!once && ++once) easy_memory__allow(__FILE__, __LINE__, count, size); }
  void easy_memory__allocate(void **, int64_t, char *file, int line);
  int easy_memory__canary_check(char *file, int line, void *pointer);
  void easy_memory__allow(char *file, int line, int count, int64_t size);
#endif

#define easy_memory_allocate_once(pointer, size) do_not_use_easy_memory_allocate_once_anymore

char *easy_commas_int (int64_t);
char *easy_commas_float (double number, int decimals);

void easy_string_terminate ();

#define easy_string(string) &((const struct easy_string) {.allocation_size = 0, .length = strlen(string), .const_buffer = (string)})
#define easy_str_len(str, len) &((const struct easy_string) {.allocation_size = 0, .length = (len), .const_buffer = (str)})

#define easy_string_temp(string) easy_string__temp(string, __FILE__, __LINE__)
struct easy_string *easy_string__temp(const struct easy_string *source, char *file, int line);

#define easy_string_aa_read_integer(aa, key) ((int *) easy_string__aa_read(aa, key, __FILE__, __LINE__))
#define easy_string_aa_read(aa, key) easy_string__aa_read(aa, key, __FILE__, __LINE__)
struct easy_string *easy_string__aa_read(const struct easy_string_aa *aa, const char *key, char *file, int line);

#define easy_string_aa_write_integer(aa, key) ((int *) easy_string__aa_write(aa, key, __FILE__, __LINE__))
#define easy_string_aa_write(aa, key) easy_string__aa_write(aa, key, __FILE__, __LINE__)
struct easy_string *easy_string__aa_write(struct easy_string_aa *aa, const char *key, char *file, int line);

#define easy_string_aa_delete(aa, key) easy_string__aa_delete(aa, key, __FILE__, __LINE__)
void easy_string__aa_delete(struct easy_string_aa *aa, const char *key, char *file, int line);

#define easy_string_aa_free(aa) easy_string__aa_free(aa, __FILE__, __LINE__)
void easy_string__aa_free(struct easy_string_aa *aa, char *file, int line);

#define easy_string_aa_copy(aa, key, string) easy_string__copy(easy_string__aa_write(aa, key, __FILE__, __LINE__), string, __FILE__, __LINE__)
#define easy_string_aa_append(aa, key, string) {struct easy_string *__iTU3VhyPw3KXY5L5HbA3__ = easy_string__aa_write(aa, key, __FILE__, __LINE__); easy_string__splice(NULL, __iTU3VhyPw3KXY5L5HbA3__, __iTU3VhyPw3KXY5L5HbA3__->length, 0, string, __FILE__, __LINE__); }
#define easy_string_aa_format(aa, key, ...) easy_string__format(__FILE__, __LINE__, easy_string__aa_write(aa, key, __FILE__, __LINE__), __VA_ARGS__)
#define easy_string_aa_append_format(aa, key, ...) easy_string__append_format(__FILE__, __LINE__, easy_string__aa_write(aa, key, __FILE__, __LINE__), __VA_ARGS__)

#define easy_string_template(destination, template, aa, path) easy_string__template(destination, template, aa, path, __FILE__, __LINE__)
void easy_string__template(struct easy_string *destination, const struct easy_string *template, const struct easy_string_aa *aa, const char *path, char *file, int line);

#define easy_string_copy(destination, source) easy_string__copy(destination, source, __FILE__, __LINE__)
void easy_string__copy (struct easy_string *destination, const struct easy_string *source, char *file, int line);

#define easy_string_move(destination, source) easy_string__move(destination, source, __FILE__, __LINE__)
void easy_string__move (struct easy_string *destination, struct easy_string *source, char *file, int line);

#define easy_string_append(destination, source) easy_string__splice(NULL, destination, (destination)->length, 0, source, __FILE__, __LINE__)
#define easy_string_splice(destination, tape, offset, length, source) easy_string__splice(destination, tape, offset, length, source, __FILE__, __LINE__)
struct easy_string *easy_string__splice(struct easy_string *destination, struct easy_string *tape, int offset, int length, const struct easy_string *source, char *file, int line);

#define easy_string_append_splice(destination, tape, offset, length) easy_string__append_splice(destination, tape, offset, length, __FILE__, __LINE__)
void easy_string__append_splice(struct easy_string *destination, const struct easy_string *tape, int offset, int length, char *file, int line);

#define easy_string_fgets(destination, the_file) easy_string__fgets(destination, the_file, __FILE__, __LINE__)
int easy_string__fgets (struct easy_string *destination, FILE *the_file, char *file, int line);

#define easy_string_fread_all(destination, the_file) easy_string__fread_all(destination, the_file, __FILE__, __LINE__)
void easy_string__fread_all (struct easy_string *destination, FILE *the_file, char *file, int line);

#define easy_string_load_file(dest, ...) easy_string__load_file(__FILE__, __LINE__, dest, __VA_ARGS__)
int easy_string__load_file(char *file, int line, struct easy_string *destination, const char *format, ...) __attribute__ ((format (printf, 4, 5)));

#define easy_string_save_file(dest, ...) easy_string__save_file(__FILE__, __LINE__, dest, __VA_ARGS__)
int easy_string__save_file(char *file, int line, struct easy_string *source, const char *format, ...) __attribute__ ((format (printf, 4, 5)));

#define easy_string_fread(destination, the_file, count) easy_string__fread(destination, the_file, count, __FILE__, __LINE__)
int easy_string__fread (struct easy_string *destination, FILE *the_file, int count, char *file, int line);

#define easy_string_join(destination, array, divisor) easy_string__join(destination, array, divisor, __FILE__, __LINE__)
struct easy_string *easy_string__join (struct easy_string *source, const struct easy_string_array *array, const struct easy_string *divisor, char *file, int line);

#define easy_string_split(array, source, divisor) easy_string__split(array, source, divisor, __FILE__, __LINE__)
void easy_string__split (struct easy_string_array *array, const struct easy_string *source, const struct easy_string *divisor, char *file, int line);

#define easy_string_format(dest, ...) easy_string__format(__FILE__, __LINE__, dest, __VA_ARGS__)
struct easy_string *easy_string__format(char *file, int line, struct easy_string *destination, const char *format, ...) __attribute__ ((format (printf, 4, 5)));

#define easy_string_append_format(dest, ...) easy_string__append_format(__FILE__, __LINE__, dest, __VA_ARGS__)
void easy_string__append_format(char *file, int line, struct easy_string *destination, const char *format, ...) __attribute__ ((format (printf, 4, 5)));

#define easy_string_fuck_spaces_array(array) for (int __iTU3VhyPw3KXY5L5HbA3__ = 0; __iTU3VhyPw3KXY5L5HbA3__ < (array)->count; __iTU3VhyPw3KXY5L5HbA3__++) { easy_string__fuck_spaces(&(array)->string[__iTU3VhyPw3KXY5L5HbA3__], __FILE__, __LINE__); }
#define easy_string_fuck_spaces(...) for (int __iTU3VhyPw3KXY5L5HbA3__ = 0; __iTU3VhyPw3KXY5L5HbA3__ < sizeof((struct easy_string *[]) {__VA_ARGS__}) / sizeof(struct easy_string *); __iTU3VhyPw3KXY5L5HbA3__++) { easy_string__fuck_spaces(((struct easy_string *[]) {__VA_ARGS__})[__iTU3VhyPw3KXY5L5HbA3__], __FILE__, __LINE__); }
void easy_string__fuck_spaces (struct easy_string *string, char *file, int line);

#define easy_string_free(...) for (int __jRhDfeY84CP2XQ3tpquU__ = 0; __jRhDfeY84CP2XQ3tpquU__ < sizeof((struct easy_string *[]) {__VA_ARGS__}) / sizeof(struct easy_string *); __jRhDfeY84CP2XQ3tpquU__++) { easy_string__free(((struct easy_string *[]) {__VA_ARGS__})[__jRhDfeY84CP2XQ3tpquU__], __FILE__, __LINE__); }
#define easy_string__free(string, file, line) easy_memory_allocate(&(string)->buffer, 0); (string)->allocation_size = 0; (string)->length = 0

#define easy_string_init(...) for (int __jRhDfeY84CP2XQ3tpquU__ = 0; __jRhDfeY84CP2XQ3tpquU__ < sizeof((struct easy_string *[]) {__VA_ARGS__}) / sizeof(struct easy_string *); __jRhDfeY84CP2XQ3tpquU__++) { easy_string__init(((struct easy_string *[]) {__VA_ARGS__})[__jRhDfeY84CP2XQ3tpquU__], __FILE__, __LINE__); }
#define easy_string__init(string, file, line) (string)->buffer = NULL; easy_memory_allocate(&(string)->buffer, 1); (string)->allocation_size = 1; (string)->buffer[0] = 0

// IDK that this "null" idea is ever really preferrable to freeing the string.
//#define easy_string_null(...) for (int __NvXijYazrUVRSZPT3p9M__ = 0; __NvXijYazrUVRSZPT3p9M__ < sizeof((struct easy_string *[]) {__VA_ARGS__}) / sizeof(struct easy_string *); __NvXijYazrUVRSZPT3p9M__++) { easy_string__null(((struct easy_string *[]) {__VA_ARGS__})[__NvXijYazrUVRSZPT3p9M__], __FILE__, __LINE__); }
//#define easy_string__null(string, file, line) if ((string)->length) (string)->buffer[0] = 0; (string)->length = 0

#define easy_string_array_push(array, string) easy_string__array_push(array, string, __FILE__, __LINE__)
struct easy_string *easy_string__array_push (struct easy_string_array *array, const struct easy_string *string, char *file, int line);

#define easy_string_array_push_format(array, ...) easy_string__array_push_format(__FILE__, __LINE__, array, __VA_ARGS__)
struct easy_string *easy_string__array_push_format (char *file, int line, struct easy_string_array *array, const char *format, ...) __attribute__ ((format (printf, 4, 5)));

#define easy_string_array_pop(string, array) easy_string__array_pop(string, array, __FILE__, __LINE__)
void easy_string__array_pop (struct easy_string *string, struct easy_string_array *array, char *file, int line);

#define easy_string_array_insert(array, index, string) easy_string__array_insert(array, index, string, __FILE__, __LINE__)
void easy_string__array_insert (struct easy_string_array *array, int index, const struct easy_string *string, char *file, int line);

#define easy_string_array_delete(array, index) easy_string__array_delete(array, index, __FILE__, __LINE__)
void easy_string__array_delete (struct easy_string_array *array, int index, char *file, int line);

#define easy_string_array_free(array) easy_string__array_free(array, __FILE__, __LINE__)
void easy_string__array_free (struct easy_string_array *array, char *file, int line);

#define easy_string_allocate(string, size) easy_string__allocate(string, size, __FILE__, __LINE__)
void easy_string__allocate (struct easy_string *string, int size, char *file, int line);

#define easy_strncpy(dest, src, size) { int i; for (i = 0; i < size - 1; i++) { if (!src[i]) break; dest[i] = src[i]; }; dest[i] = 0; }
#define strncpy(dest, src, size) easy__fuck(__FILE__, __LINE__, "strncpy() is broken, use easy_strncpy() instead.")
#define strncat(dest, src, size) easy__fuck(__FILE__, __LINE__, "strncat() is broken, use easy_strncat() instead.")

#undef assert

#ifdef LINUX
#define assert(x) if (!(x)) { easy_warn("assert(%s) failed", #x); *((volatile int *) 0) = 0; }
#else
#define assert(x) if (!(x)) easy_fuck("assert(%s) failed", #x)
#endif

#define easy_warn(...) easy__fuck(__FILE__, __LINE__, 0, __VA_ARGS__)
#define easy_fuck(...) easy__fuck(__FILE__, __LINE__, 1, __VA_ARGS__)

#define EASY_BINARY_INCLUDE(symbol) \
extern char _binary_ ## symbol ## _start; \
extern char _binary_ ## symbol ## _end; \
extern char _binary_ ## symbol ## _size; \
char * symbol ## _pointer = &_binary_ ## symbol ## _start; \
int symbol ## _size = &_binary_ ## symbol ## _end - &_binary_ ## symbol ## _start

void easy_sort_floats(float *list, int count);
void easy_sort_by_float(void *list, int size, int count);
void easy_sort_structures(void *list, int size, int count, int type, int offset, int reverse);

void easy__fuck(char *file, int line, int fatal, char *format, ...) __attribute__ ((format (printf, 4, 5)));
#define easy_error(message, code) easy_fuck("%s: %s", message, easy_error_string(code))
double easy_time();
double easy_actual_time();
void easy_sleep(double seconds);
void easy_seed_random_number_generator();
void easy_random_binary_string(void *string, int bytes);
void easy_random_hex_string(char *string, int length);
void easy_binary_to_ascii(char *output, char *input, int size);
int easy_random(int limit);
int easy_strnlen(char *string, int limit);
#define easy_vsprintf(dest, format, ooo) easy__vsprintf(__FILE__, __LINE__, dest, format, ooo);
int easy__vsprintf(char *file, int line, char **destination, const char *format, va_list ooo);
#define easy_sprintf(dest, ...) easy__sprintf(__FILE__, __LINE__, dest, __VA_ARGS__);
int easy__sprintf(char *file, int line, char **destination, char *format, ...) __attribute__ ((format (printf, 4, 5)));
#define easy_strcpy(dest, src) easy__strcpy(__FILE__, __LINE__, dest, src);
void easy__strcpy(char *file, int line, char **destination, char *source);

#define FUCK fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
#define CANARY easy_memory__canary_check(__FILE__, __LINE__, NULL);

struct easy_buffer {
  void *memory_pointer;
  int memory_size;
  int page_size;
  void *pointer;
  int size;
};

void easy_buffer__append_sprintf(char *, int, struct easy_buffer *, char *format, ...) __attribute__ ((format (printf, 4, 5)));
void easy_buffer__append(struct easy_buffer *, void *, int, char *, int);
void easy_buffer__remove(struct easy_buffer *, void *, int, char *, int);
void *easy_buffer__allocate(struct easy_buffer *, int, char *, int);
void easy_buffer__free(struct easy_buffer *, int, char *, int);

#define easy_buffer_append_sprintf(pointer, ...) easy_buffer__append_sprintf(__FILE__, __LINE__, pointer, __VA_ARGS__);
#define easy_buffer_append(pointer, data, size) easy_buffer__append(pointer, data, size, __FILE__, __LINE__);
#define easy_buffer_remove(pointer, data, size) easy_buffer__remove(pointer, data, size, __FILE__, __LINE__);
#define easy_buffer_allocate(pointer, size) easy_buffer__allocate(pointer, size, __FILE__, __LINE__)
#define easy_buffer_free(pointer, size) easy_buffer__free(pointer, size, __FILE__, __LINE__)
#define easy_buffer_destroy(pointer) easy_buffer__free(pointer, (pointer)->size, __FILE__, __LINE__)

const char *easy_error_string(int error_code);

void easy_sha1(char *digest, const char *message, int length);

/*
  allow_errors =  1 means allow all errors
  allow_errors =  0 means easy_fuck() errors, allow short reads only at end of file
  allow_errors = -1 means don't allow errors or short reads
*/

#define easy_file_open(pathname, flags, allow_errors) easy__file_open(__FILE__, __LINE__, pathname, flags, allow_errors)
#define easy_file_close(fd, allow_errors) easy__file_close(__FILE__, __LINE__, fd, allow_errors)
#define easy_file_read(fd, buffer, count, allow_errors) easy__file_read(__FILE__, __LINE__, fd, buffer, count, allow_errors)
#define easy_file_write(fd, buffer, count, allow_errors) easy__file_write(__FILE__, __LINE__, fd, buffer, count, allow_errors)
int easy__file_open(char *file, int line, char *pathname, int flags, int allow_errors);
int easy__file_close(char *file, int line, int fd, int allow_errors);
int easy__file_read(char *file, int line, int fd, void *buffer, ssize_t count, int allow_errors);
int easy__file_write(char *file, int line, int fd, void *buffer, ssize_t count, int allow_errors);

#ifndef EASY_NO_THREADS

#include <pthread.h>
#include <semaphore.h>

struct easy_thread_state {
  pthread_t thread; // needed for pthread_join()
  sem_t init; // so child thread can tell parent thread it has initialized
  sem_t exit; // so parent thread can tell child thread that it is time to exit
};

// set or querty init or exit flags
void easy_thread_set_init_flag (struct easy_thread_state *thread_state);
int easy_thread_get_init_flag (struct easy_thread_state *thread_state);
void easy_thread_wait_init_flag (struct easy_thread_state *thread_state);
void easy_thread_set_exit_flag (struct easy_thread_state *thread_state);
int easy_thread_get_exit_flag (struct easy_thread_state *thread_state);
void easy_thread_wait_exit_flag (struct easy_thread_state *thread_state);

void easy_thread_fork(struct easy_thread_state *thread, void * (*function)(void *), void *parameter);
void easy_thread_join(struct easy_thread_state *thread); // wait until thread exits on its own

void easy__sem_init(char *file, int line, sem_t *sem, int pshared, unsigned int value);
void easy__sem_post(char *file, int line, sem_t *sem); // signal that something can be done
void easy__sem_wait(char *file, int line, sem_t *sem); // wait until something can be done
int easy__sem_trywait(char *file, int line, sem_t *sem); // check if something can be done
void easy__sem_destroy(char *file, int line, sem_t *sem);

void easy__mutex_init(char *file, int line, pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
void easy__mutex_lock(char *file, int line, pthread_mutex_t *mutex);
int easy__mutex_trylock(char *file, int line, pthread_mutex_t *mutex);
void easy__mutex_unlock(char *file, int line, pthread_mutex_t *mutex);
void easy__mutex_destroy(char *file, int line, pthread_mutex_t *mutex);

#define easy_sem_init(sem, pshared, value) easy__sem_init(__FILE__, __LINE__, sem, pshared, value)
#define easy_sem_post(sem) easy__sem_post(__FILE__, __LINE__, sem)
#define easy_sem_wait(sem) easy__sem_wait(__FILE__, __LINE__, sem)
#define easy_sem_trywait(sem) easy__sem_trywait(__FILE__, __LINE__, sem)
#define easy_sem_destroy(sem) easy__sem_destroy(__FILE__, __LINE__, sem)

#define easy_mutex_init(mutex, attr) easy__mutex_init(__FILE__, __LINE__, mutex, attr)
#define easy_mutex_lock(mutex) easy__mutex_lock(__FILE__, __LINE__, mutex)
#define easy_mutex_trylock(mutex) easy__mutex_trylock(__FILE__, __LINE__, mutex)
#define easy_mutex_unlock(mutex) easy__mutex_unlock(__FILE__, __LINE__, mutex)
#define easy_mutex_destroy(mutex) easy__mutex_destroy(__FILE__, __LINE__, mutex)

#endif

/* Example code:

  static struct easy_thread_state whatever_thread_state;

  void *thread (void *parameter) {
    struct easy_thread_state *thread_state = &whatever_thread_state;
    // initialization code
    easy_thread_set_init_flag(thread_state);
    while (!easy_thread_get_exit_flag(thread_state)) {
      // main loop
    };
    // termination code
    return NULL;
  };

  void initialize () {
    easy_thread_fork(&whatever_thread_state, thread, parameter);
    easy_thread_wait_init_flag(&whatever_thread_state);
  };

  void terminate () {
    easy_thread_set_exit_flag(&whatever_thread_state);
    easy_thread_join(&whatever_thread_state);
  };

*/

#endif

#define easy_declare_list_type(type) \
typedef struct easy_ ## type ## _list { \
  type *item; \
  int count; \
} easy_ ## type ## _list;

#define easy_list_push(pointer, new) \
easy_memory_allocate(&(pointer)->item, ++(pointer)->count * sizeof(*(pointer)->item)); \
(pointer)->item[(pointer)->count - 1] = (new);

#define easy_list_insert(pointer, index, new) { int _easy_temporary = (index); \
if (_easy_temporary < 0 || _easy_temporary > (pointer)->count) easy_fuck("Out of bounds list access.\n"); \
easy_memory_allocate(&(pointer)->item, ++(pointer)->count * sizeof(*(pointer)->item)); \
memmove(&(pointer)->item[_easy_temporary + 1], &(pointer)->item[_easy_temporary], ((pointer)->count - _easy_temporary - 1) * sizeof(*(pointer)->item)); \
(pointer)->item[_easy_temporary] = new; }

#define easy_list_delete(pointer, index) { int _easy_temporary = (index); \
if (_easy_temporary < 0 || _easy_temporary >= (pointer)->count) easy_fuck("Out of bounds list access.\n"); \
memmove(&(pointer)->item[_easy_temporary], &(pointer)->item[_easy_temporary + 1], ((pointer)->count - _easy_temporary - 1) * sizeof(*(pointer)->item)); \
easy_memory_allocate(&(pointer)->item, --(pointer)->count * sizeof(*(pointer)->item)); }

#define easy_list_free(pointer) \
easy_memory_allocate(&(pointer)->item, 0); \
(pointer)->item = NULL; \
(pointer)->count = 0;
