/*
  Released under the antivirial license.  Basically, you can do anything
  you want with it as long as what you want doesn't involve the GNU GPL.
  See http://www.ecstaticlyrics.com/antiviral/ for more information.

  Optional compile-time #define:

  #define EASY_NO_THREADS
  #define EASY_RAW_MALLOC
  #define EASY_ALL_LEAKS

*/

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>

#ifdef EASY_NO_THREADS
  // define some things so that declarations for non-existent functions don't cause errors
  #define sem_t void
  struct easy_thread_state {};
#else
  #include <pthread.h>
  #include <semaphore.h>
#endif

#include "easy.h"

#ifndef EASY_BUFFER_DEFAULT_PAGE_SIZE
#define EASY_BUFFER_DEFAULT_PAGE_SIZE (1 << 21)
#endif

#ifndef EASY_TEMPORARY_STRING_COUNT
#define EASY_TEMPORARY_STRING_COUNT 64
#endif

#ifdef FREEBSD
static const char *error_strings[] = {"EPERM: Operation not permitted.", "ENOENT: No such file or directory.", "ESRCH: No such process.", "EINTR: Interrupted system call.", "EIO: Input/output error.", "ENXIO: Device not configured.", "E2BIG: Argument list too long.", "ENOEXEC: Exec format error.", "EBADF: Bad file descriptor.", "ECHILD: No child processes.", "EDEADLK: Resource deadlock avoided.", "ENOMEM: Cannot allocate memory.", "EACCES: Permission denied.", "EFAULT: Bad address.", "ENOTBLK: Block device required.", "EBUSY: Device busy.", "EEXIST: File exists.", "EXDEV: Cross-device link.", "ENODEV: Operation not supported by device.", "ENOTDIR: Not a directory.", "EISDIR: Is a directory.", "EINVAL: Invalid argument.", "ENFILE: Too many open files in system.", "EMFILE: Too many open files.", "ENOTTY: Inappropriate ioctl for device.", "ETXTBSY: Text file busy.", "EFBIG: File too large.", "ENOSPC: No space left on device.", "ESPIPE: Illegal seek.", "EROFS: Read-only filesystem.", "EMLINK: Too many links.", "EPIPE: Broken pipe.", "EDOM: Numerical argument out of domain.", "ERANGE: Result too large.", "EAGAIN: Resource temporarily unavailable.", "EINPROGRESS: Operation now in progress.", "EALREADY: Operation already in progress.", "ENOTSOCK: Socket operation on non-socket.", "EDESTADDRREQ: Destination address required.", "EMSGSIZE: Message too long.", "EPROTOTYPE: Protocol wrong type for socket.", "ENOPROTOOPT: Protocol not available.", "EPROTONOSUPPORT: Protocol not supported.", "ESOCKTNOSUPPORT: Socket type not supported.", "EOPNOTSUPP: Operation not supported.", "EPFNOSUPPORT: Protocol family not supported.", "EAFNOSUPPORT: Address family not supported by protocol family.", "EADDRINUSE: Address already in use.", "EADDRNOTAVAIL: Can't assign requested address.", "ENETDOWN: Network is down.", "ENETUNREACH: Network is unreachable.", "ENETRESET: Network dropped connection on reset.", "ECONNABORTED: Software caused connection abort.", "ECONNRESET: Connection reset by peer.", "ENOBUFS: No buffer space available.", "EISCONN: Socket is already connected.", "ENOTCONN: Socket is not connected.", "ESHUTDOWN: Can't send after socket shutdown.", "ETOOMANYREFS: Too many references: can't splice.", "ETIMEDOUT: Operation timed out.", "ECONNREFUSED: Connection refused.", "ELOOP: Too many levels of symbolic links.", "ENAMETOOLONG: File name too long.", "EHOSTDOWN: Host is down.", "EHOSTUNREACH: No route to host.", "ENOTEMPTY: Directory not empty.", "EPROCLIM: Too many processes.", "EUSERS: Too many users.", "EDQUOT: Disc quota exceeded.", "ESTALE: Stale NFS file handle.", "EREMOTE: Too many levels of remote in path.", "EBADRPC: RPC struct is bad.", "ERPCMISMATCH: RPC version wrong.", "EPROGUNAVAIL: RPC prog. not avail.", "EPROGMISMATCH: Program version wrong.", "EPROCUNAVAIL: Bad procedure for program.", "ENOLCK: No locks available.", "ENOSYS: Function not implemented.", "EFTYPE: Inappropriate file type or format.", "EAUTH: Authentication error.", "ENEEDAUTH: Need authenticator.", "EIDRM: Identifier removed.", "ENOMSG: No message of desired type.", "EOVERFLOW: Value too large to be stored in data type.", "ECANCELED: Operation canceled.", "EILSEQ: Illegal byte sequence.", "ENOATTR: Attribute not found.", "EDOOFUS: Programming error.", "EBADMSG: Bad message.", "EMULTIHOP: Multihop attempted.", "ENOLINK: Link has been severed.", "EPROTO: Protocol error.", "ENOTCAPABLE: Capabilities insufficient.", "ECAPMODE: Not permitted in capability mode."};
#endif

#ifdef LINUX
static const char *error_strings[] = {"EPERM: Operation not permitted.", "ENOENT: No such file or directory.", "ESRCH: No such process.", "EINTR: Interrupted system call.", "EIO: I/O error.", "ENXIO: No such device or address.", "E2BIG: Argument list too long.", "ENOEXEC: Exec format error.", "EBADF: Bad file number.", "ECHILD: No child processes.", "EAGAIN: Try again.", "ENOMEM: Out of memory.", "EACCES: Permission denied.", "EFAULT: Bad address.", "ENOTBLK: Block device required.", "EBUSY: Device or resource busy.", "EEXIST: File exists.", "EXDEV: Cross-device link.", "ENODEV: No such device.", "ENOTDIR: Not a directory.", "EISDIR: Is a directory.", "EINVAL: Invalid argument.", "ENFILE: File table overflow.", "EMFILE: Too many open files.", "ENOTTY: Not a typewriter.", "ETXTBSY: Text file busy.", "EFBIG: File too large.", "ENOSPC: No space left on device.", "ESPIPE: Illegal seek.", "EROFS: Read-only file system.", "EMLINK: Too many links.", "EPIPE: Broken pipe.", "EDOM: Math argument out of domain of func.", "ERANGE: Math result not representable.", "EDEADLK: Resource deadlock would occur.", "ENAMETOOLONG: File name too long.", "ENOLCK: No record locks available.", "ENOSYS: Function not implemented.", "ENOTEMPTY: Directory not empty.", "ELOOP: Too many symbolic links encountered.", "EWOULDBLOCK: Operation would block.", "ENOMSG: No message of desired type.", "EIDRM: Identifier removed.", "ECHRNG: Channel number out of range.", "EL2NSYNC: Level 2 not synchronized.", "EL3HLT: Level 3 halted.", "EL3RST: Level 3 reset.", "ELNRNG: Link number out of range.", "EUNATCH: Protocol driver not attached.", "ENOCSI: No CSI structure available.", "EL2HLT: Level 2 halted.", "EBADE: Invalid exchange.", "EBADR: Invalid request descriptor.", "EXFULL: Exchange full.", "ENOANO: No anode.", "EBADRQC: Invalid request code.", "EBADSLT: Invalid slot.", "EDEADLOCK: Deadlock.", "EBFONT: Bad font file format.", "ENOSTR: Device not a stream.", "ENODATA: No data available.", "ETIME: Timer expired.", "ENOSR: Out of streams resources.", "ENONET: Machine is not on the network.", "ENOPKG: Package not installed.", "EREMOTE: Object is remote.", "ENOLINK: Link has been severed.", "EADV: Advertise error.", "ESRMNT: Srmount error.", "ECOMM: Communication error on send.", "EPROTO: Protocol error.", "EMULTIHOP: Multihop attempted.", "EDOTDOT: RFS specific error.", "EBADMSG: Not a data message.", "EOVERFLOW: Value too large for defined data type.", "ENOTUNIQ: Name not unique on network.", "EBADFD: File descriptor in bad state.", "EREMCHG: Remote address changed.", "ELIBACC: Can not access a needed shared library.", "ELIBBAD: Accessing a corrupted shared library.", "ELIBSCN: .lib section in a.out corrupted.", "ELIBMAX: Attempting to link in too many shared libraries.", "ELIBEXEC: Cannot exec a shared library directly.", "EILSEQ: Illegal byte sequence.", "ERESTART: Interrupted system call should be restarted.", "ESTRPIPE: Streams pipe error.", "EUSERS: Too many users.", "ENOTSOCK: Socket operation on non-socket.", "EDESTADDRREQ: Destination address required.", "EMSGSIZE: Message too long.", "EPROTOTYPE: Protocol wrong type for socket.", "ENOPROTOOPT: Protocol not available.", "EPROTONOSUPPORT: Protocol not supported.", "ESOCKTNOSUPPORT: Socket type not supported.", "EOPNOTSUPP: Operation not supported on transport endpoint.", "EPFNOSUPPORT: Protocol family not supported.", "EAFNOSUPPORT: Address family not supported by protocol.", "EADDRINUSE: Address already in use.", "EADDRNOTAVAIL: Cannot assign requested address.", "ENETDOWN: Network is down.", "ENETUNREACH: Network is unreachable.", "ENETRESET: Network dropped connection because of reset.", "ECONNABORTED: Software caused connection abort.", "ECONNRESET: Connection reset by peer.", "ENOBUFS: No buffer space available.", "EISCONN: Transport endpoint is already connected.", "ENOTCONN: Transport endpoint is not connected.", "ESHUTDOWN: Cannot send after transport endpoint shutdown.", "ETOOMANYREFS: Too many references: cannot splice.", "ETIMEDOUT: Connection timed out.", "ECONNREFUSED: Connection refused.", "EHOSTDOWN: Host is down.", "EHOSTUNREACH: No route to host.", "EALREADY: Operation already in progress.", "EINPROGRESS: Operation now in progress.", "ESTALE: Stale NFS file handle.", "EUCLEAN: Structure needs cleaning.", "ENOTNAM: Not a XENIX named type file.", "ENAVAIL: No XENIX semaphores available.", "EISNAM: Is a named type file.", "EREMOTEIO: Remote I/O error.", "EDQUOT: Quota exceeded.", "ENOMEDIUM: No medium found.", "EMEDIUMTYPE: Wrong medium type.", "ECANCELED: Operation Canceled.", "ENOKEY: Required key not available.", "EKEYEXPIRED: Key has expired.", "EKEYREVOKED: Key has been revoked.", "EKEYREJECTED: Key was rejected by service.", "EOWNERDEAD: Owner died.", "ENOTRECOVERABLE: State not recoverable.", "ERFKILL: Operation not possible due to RF-kill."};
#endif

// easy_string temporary variables

static __thread struct easy_string temporary[EASY_TEMPORARY_STRING_COUNT] = {};
static __thread int temporary_index = 0;

// easy_memory_allocate stuff

static int random_number = 0;

#ifndef EASY_NO_THREADS
  static pthread_rwlock_t memory_lock;
#endif

struct memory_double_linked_list {
  void *prev;
  void *next;
  int size;
  char *file;
  int line;
  int flags;
};

#define SIZE 64
#define  prev(X) (  *( (void **)     (  ((void *) (X)) - 64  ) )  )
#define  next(X) (  *( (void **)     (  ((void *) (X)) - 56  ) )  )
#define  size(X) (  *( (int64_t *)  (  ((void *) (X)) - 48  ) )  )
#define  file(X) (  *( (char **)     (  ((void *) (X)) - 40  ) )  )
#define  line(X) (  *( (int *)       (  ((void *) (X)) - 32  ) )  )
#define flags(X) (  *( (int *)       (  ((void *) (X)) - 28  ) )  )
#define test0(X) (  *( (int *)       (  ((void *) (X)) -  4  ) )  )
#define test1(X) (  *( (int *)       (  ((void *) (X)) + size(X))))

static struct memory_double_linked_list first_data, last_data;
static struct memory_double_linked_list first_data = {NULL, NULL, 0, "the first item in the list", 0, 0};
static struct memory_double_linked_list last_data = {NULL, NULL, 0, "the last item in the list", 0, 0};
static void *first = ((void *) &first_data) + SIZE;
static void *last = ((void *) &last_data) + SIZE;

struct allowed {
  char *file;
  int line;
  int count;
  int64_t size;
};

static struct allowed *allowance = NULL;
static int allowance_count = 0;

//--page-split-- #ifdef WINDOWS

#ifdef WINDOWS

  void *memmem (const void *haystack, size_t haystacklen, const void *needle, size_t needlelen) {
    while (haystacklen >= needlelen) {
      for (int i = 0; i < needlelen; i++) {
        if (((char *) haystack)[i] != ((char *) needle)[i]) goto NEXT;
      };
      return (void *) haystack;
      NEXT:
      haystack++;
      haystacklen--;
    };
    return NULL;
  };

  char *index (const char *s, int c) {
    const char *p = s;
    do {
      if (*p == c) return (char *) p;
    } while (*p++);
    return NULL;
  };

  char *rindex (const char *s, int c) {
    const char *p = s;
    while (*p) p++;
    while (--p >= s && *p != c);
    if (p >= s) return (char *) p;
    return NULL;
  };

#endif

//--page-split-- #ifdef LINUX

#ifdef LINUX

  #include <backtrace.h>

  static int backtrace_known_count = 0;
  static int backtrace_unknown_count = 0;

  static void error_callback (void *data, const char *message, int error_number) {
    if (error_number == -1) {
      fprintf(stderr, "If you want backtraces, you have to compile with -g\n\n");
      _Exit(1);
    } else {
      fprintf(stderr, "Backtrace error %d: %s\n", error_number, message);
    };
  };

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

  static struct backtrace_state *state;

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

  static void backtrace_initialize () {
    state = backtrace_create_state(NULL, 1, error_callback, NULL);
    signal(SIGSEGV, sigsegv_handler);
    signal(SIGABRT, sigsegv_handler);
    signal(SIGFPE, sigsegv_handler);
    signal(SIGBUS, sigsegv_handler);
  };

#endif

//--page-split-- easy_memory__allocate

#ifndef EASY_RAW_MALLOC

void easy_memory__allocate(void **pointer, int64_t size, char *file, int line) {
  #ifdef EASY_RAW_MALLOC
  if (size) {
    *pointer = realloc(*pointer, (size_t) size);
  } else {
    free(*pointer);
    *pointer = NULL;
  };
  #else
  if (easy_memory__canary_check(file, line, *pointer)) {
    easy__fuck(file, line, 1, "easy_memory_allocate(%p, %ld) -- this pointer is not one of my pointers\n", *pointer, size);
  };
  if (*pointer == NULL && size != 0) {
    void *new = malloc((size_t) (size + SIZE + 4));
    if (new == NULL) {
      easy__fuck(file, line, 1, "easy_memory_allocate(%p, %ld) -- malloc() returned NULL\n", *pointer, size);
    };
    *pointer = new + SIZE;
    #ifndef EASY_NO_THREADS
    pthread_rwlock_wrlock(&memory_lock);
    #endif
    if (next(first) == NULL || prev(last) == NULL) {
      easy__fuck(file, line, 1, "easy_initialize() has not been called.");
    };
    prev(*pointer) = prev(last);
    next(*pointer) = last;
    next(prev(*pointer)) = *pointer;
    prev(next(*pointer)) = *pointer;
    file(*pointer) = file;
    line(*pointer) = line;
    size(*pointer) = size;
    flags(*pointer) = 0; // flags;
    test0(*pointer) = random_number;
    test1(*pointer) = random_number;
    #ifndef EASY_NO_THREADS
    pthread_rwlock_unlock(&memory_lock);
    #endif
  } else if (*pointer != NULL && size != 0) {
    #ifndef EASY_NO_THREADS
    pthread_rwlock_wrlock(&memory_lock);
    #endif
    if (next(first) == NULL || prev(last) == NULL) {
      easy__fuck(file, line, 1, "easy_initialize() has not been called.");
    };
    if (size != size(*pointer)) {
      void *_prev = prev(*pointer);
      void *_next = next(*pointer);
      char *_file = file(*pointer);
      int _line = line(*pointer);
      int _flags = flags(*pointer);
      void *new = realloc(*pointer - SIZE, (size_t) (size + SIZE + 4));
      if (new == NULL) {
        easy__fuck(file, line, 1, "easy_memory_allocate(%p, %ld) -- malloc() returned NULL\n", *pointer, size);
      };
      *pointer = new + SIZE;
      prev(*pointer) = _prev;
      next(*pointer) = _next;
      file(*pointer) = _file;
      line(*pointer) = _line;
      flags(*pointer) = _flags;
      prev(_next) = *pointer;
      next(_prev) = *pointer;
      size(*pointer) = size;
      test0(*pointer) = random_number;
      test1(*pointer) = random_number;
    };
    #ifndef EASY_NO_THREADS
    pthread_rwlock_unlock(&memory_lock);
    #endif
  } else if (*pointer != NULL && size == 0) {
    #ifndef EASY_NO_THREADS
    pthread_rwlock_wrlock(&memory_lock);
    #endif
    if (next(first) == NULL || prev(last) == NULL) {
      easy__fuck(file, line, 1, "easy_initialize() has not been called.");
    };
    next(prev(*pointer)) = next(*pointer);
    prev(next(*pointer)) = prev(*pointer);
    #ifndef EASY_NO_THREADS
    pthread_rwlock_unlock(&memory_lock);
    #endif
    free(*pointer - SIZE);
    *pointer = NULL;
  };
  #endif
};

#endif

//--page-split-- easy_buffer__append

void easy_buffer__append(struct easy_buffer *buffer, void *data, int length, char *file, int line) {
  void *pointer = easy_buffer__allocate(buffer, length, file, line);
  memmove(pointer, data, length);
  buffer->size += length;
};

//--page-split-- easy_buffer__remove

void easy_buffer__remove(struct easy_buffer *buffer, void *data, int length, char *file, int line) {
  if (length > buffer->size) easy__fuck(file, line, 1, "Attempt to remove %d bytes from buffer, but there are only %d bytes in the buffer.", length, buffer->size);
  memmove(data, buffer->pointer, length);
  easy_buffer__free(buffer, length, file, line);
};

//--page-split-- easy_buffer__allocate

void *easy_buffer__allocate(struct easy_buffer *buffer, int length, char *file, int line) {
  if (buffer->page_size == 0) buffer->page_size = EASY_BUFFER_DEFAULT_PAGE_SIZE;
  if (buffer->page_size & (buffer->page_size - 1)) easy__fuck(file, line, 1, "This buffer has a page size that isn't a power of two.");
  if (buffer->size == 0) buffer->pointer = buffer->memory_pointer;
  if (buffer->pointer - buffer->memory_pointer >= EASY_BUFFER_DEFAULT_PAGE_SIZE) {
    easy_buffer__free(buffer, 0, file, line); // free memory in case free is never being called
  };
//  printf("\e[1;33mbuffer_allocate(%d)\e[0m\n", length);
  int offset = buffer->pointer - buffer->memory_pointer;
  int available = buffer->memory_size - buffer->size - offset;
//  printf("buffer->memory_pointer = %p\n", buffer->memory_pointer);
//  printf("   buffer->memory_size = %lu\n", buffer->memory_size);
//  printf("       buffer->pointer = %p\n", buffer->pointer);
//  printf("          buffer->size = %lu\n", buffer->size);
//  printf("                offset = %lu\n", offset);
//  printf("             available = %lu\n", available);
  if (length > available) {
    int needed = length - available;
    int pages = (needed + buffer->page_size - 1) / buffer->page_size;
//    printf("                needed = %lu\n", needed);
//    printf("                 pages = %lu (%lu bytes)\n", pages, buffer->page_size * pages);
    void *previous_pointer = buffer->memory_pointer;
    easy_memory__allocate((void **) &buffer->memory_pointer, buffer->memory_size + pages * buffer->page_size, file, line);
    buffer->pointer += buffer->memory_pointer - previous_pointer;
    buffer->memory_size += pages * buffer->page_size;
//    if (buffer->memory_pointer != previous_pointer) {
//      printf("\e[1;32mBuffer has moved to %p\e[0m\n", buffer->memory_pointer);
//   };
  };
  assert(buffer->memory_pointer <= buffer->pointer);
  assert(buffer->pointer <= buffer->memory_pointer + buffer->memory_size);
  assert(buffer->pointer + buffer->size <= buffer->memory_pointer + buffer->memory_size);
  assert(buffer->pointer + buffer->size + length <= buffer->memory_pointer + buffer->memory_size);
  return buffer->pointer + buffer->size;
};

//--page-split-- easy_buffer__free

void easy_buffer__free(struct easy_buffer *buffer, int length, char *file, int line) {
//  printf("\e[1;33mbuffer_free(%d)\e[0m\n", length);
  if (buffer->size < length) easy__fuck(file, line, 1, "Attempt to free %d bytes from buffer, but there are only %d bytes in the buffer.", length, buffer->size);
  if (buffer->size == length) {
    easy_memory__allocate(&buffer->memory_pointer, 0, file, line);
    buffer->memory_size = 0;
    buffer->pointer = NULL;
    buffer->size = 0;
  } else {
    buffer->pointer += length;
    buffer->size -= length;
    int extra = buffer->pointer - buffer->memory_pointer;
  //  printf("buffer->memory_pointer = %p\n", buffer->memory_pointer);
  //  printf("   buffer->memory_size = %u\n", buffer->memory_size);
  //  printf("       buffer->pointer = %p\n", buffer->pointer);
  //  printf("          buffer->size = %u\n", buffer->size);
  //  printf("                 extra = %u\n", extra);
    if (extra >= buffer->page_size) {
      int pages = extra / buffer->page_size;
      void *offset = buffer->memory_pointer + pages * buffer->page_size;
      int count = (buffer->pointer + buffer->size - offset + buffer->page_size - 1) / buffer->page_size;
  //    printf("          buffer->size = %u\n", buffer->size);
  //    printf("                 pages = %u (%u bytes)\n", pages, buffer->page_size * pages);
  //    printf("                offset = %u\n", (int) (uint64_t) offset);
  //    printf("                 count = %u\n", count);
      memmove(buffer->memory_pointer, offset, count * buffer->page_size);
      buffer->pointer -= pages * buffer->page_size;
      void *previous_pointer = buffer->memory_pointer;
      easy_memory__allocate(&buffer->memory_pointer, buffer->memory_size - pages * buffer->page_size, file, line);
      buffer->memory_size -= pages * buffer->page_size;
      buffer->pointer += buffer->memory_pointer - previous_pointer;
  //    if (buffer->memory_pointer != previous_pointer) {
  //      printf("\e[1;32mBuffer has moved to %p\e[0m\n", buffer->memory_pointer);
  //    };
    };
    assert(buffer->memory_pointer <= buffer->pointer);
    assert(buffer->pointer <= buffer->memory_pointer + buffer->memory_size);
    assert(buffer->pointer + buffer->size <= buffer->memory_pointer + buffer->memory_size);
  };
};

//--page-split-- easy_error_string

const char *easy_error_string(int error_code) {
  static char string[1024];

  #ifdef UNIX
  if (error_code >= 1 && error_code <= sizeof(error_strings) / sizeof(char *)) {
    snprintf(string, 1024, "Error %d: %s (strerror(%d)=\"%s\")", error_code, error_strings[error_code - 1], error_code, strerror(error_code));
    return string;
  } else {
    snprintf(string, 1024, "Error %d: Unknown error code. (strerror(%d) = \"%s\")", error_code, error_code, strerror(error_code));
    return string;
  };
  #endif

  #ifdef WINDOWS
  switch (error_code) {
    case 6: return "WSA_INVALID_HANDLE: Specified event object handle is invalid."; break;
    case 8: return "WSA_NOT_ENOUGH_MEMORY: Insufficient memory available."; break;
    case 87: return "WSA_INVALID_PARAMETER: One or more parameters are invalid."; break;
    case 995: return "WSA_OPERATION_ABORTED: Overlapped operation aborted."; break;
    case 996: return "WSA_IO_INCOMPLETE: Overlapped I/O event object not in signaled state."; break;
    case 997: return "WSA_IO_PENDING: Overlapped operations will complete later."; break;
    case 10004: return "WSAEINTR: Interrupted function call."; break;
    case 10009: return "WSAEBADF: File handle is not valid."; break;
    case 10013: return "WSAEACCES: Permission denied."; break;
    case 10014: return "WSAEFAULT: Bad address."; break;
    case 10022: return "WSAEINVAL: Invalid argument."; break;
    case 10024: return "WSAEMFILE: Too many open files."; break;
    case 10035: return "WSAEWOULDBLOCK: Resource temporarily unavailable."; break;
    case 10036: return "WSAEINPROGRESS: Operation now in progress."; break;
    case 10037: return "WSAEALREADY: Operation already in progress."; break;
    case 10038: return "WSAENOTSOCK: Socket operation on nonsocket."; break;
    case 10039: return "WSAEDESTADDRREQ: Destination address required."; break;
    case 10040: return "WSAEMSGSIZE: Message too long."; break;
    case 10041: return "WSAEPROTOTYPE: Protocol wrong type for socket."; break;
    case 10042: return "WSAENOPROTOOPT: Bad protocol option."; break;
    case 10043: return "WSAEPROTONOSUPPORT: Protocol not supported."; break;
    case 10044: return "WSAESOCKTNOSUPPORT: Socket type not supported."; break;
    case 10045: return "WSAEOPNOTSUPP: Operation not supported."; break;
    case 10046: return "WSAEPFNOSUPPORT: Protocol family not supported."; break;
    case 10047: return "WSAEAFNOSUPPORT: Address family not supported by protocol family."; break;
    case 10048: return "WSAEADDRINUSE: Address already in use."; break;
    case 10049: return "WSAEADDRNOTAVAIL: Cannot assign requested address."; break;
    case 10050: return "WSAENETDOWN: Network is down."; break;
    case 10051: return "WSAENETUNREACH: Network is unreachable."; break;
    case 10052: return "WSAENETRESET: Network dropped connection on reset."; break;
    case 10053: return "WSAECONNABORTED: Software caused connection abort."; break;
    case 10054: return "WSAECONNRESET: Connection reset by peer."; break;
    case 10055: return "WSAENOBUFS: No buffer space available."; break;
    case 10056: return "WSAEISCONN: Socket is already connected."; break;
    case 10057: return "WSAENOTCONN: Socket is not connected."; break;
    case 10058: return "WSAESHUTDOWN: Cannot send after socket shutdown."; break;
    case 10059: return "WSAETOOMANYREFS: Too many references."; break;
    case 10060: return "WSAETIMEDOUT: Connection timed out."; break;
    case 10061: return "WSAECONNREFUSED: Connection refused."; break;
    case 10062: return "WSAELOOP: Cannot translate name."; break;
    case 10063: return "WSAENAMETOOLONG: Name too long."; break;
    case 10064: return "WSAEHOSTDOWN: Host is down."; break;
    case 10065: return "WSAEHOSTUNREACH: No route to host."; break;
    case 10066: return "WSAENOTEMPTY: Directory not empty."; break;
    case 10067: return "WSAEPROCLIM: Too many processes."; break;
    case 10068: return "WSAEUSERS: User quota exceeded."; break;
    case 10069: return "WSAEDQUOT: Disk quota exceeded."; break;
    case 10070: return "WSAESTALE: Stale file handle reference."; break;
    case 10071: return "WSAEREMOTE: Item is remote."; break;
    case 10091: return "WSASYSNOTREADY: Network subsystem is unavailable."; break;
    case 10092: return "WSAVERNOTSUPPORTED: Winsock.dll version out of range."; break;
    case 10093: return "WSANOTINITIALISED: Successful WSAStartup not yet performed."; break;
    case 10101: return "WSAEDISCON: Graceful shutdown in progress."; break;
    case 10102: return "WSAENOMORE: No more results."; break;
    case 10103: return "WSAECANCELLED: Call has been canceled."; break;
    case 10104: return "WSAEINVALIDPROCTABLE: Procedure call table is invalid."; break;
    case 10105: return "WSAEINVALIDPROVIDER: Service provider is invalid."; break;
    case 10106: return "WSAEPROVIDERFAILEDINIT: Service provider failed to initialize."; break;
    case 10107: return "WSASYSCALLFAILURE: System call failure."; break;
    case 10108: return "WSASERVICE_NOT_FOUND: Service not found."; break;
    case 10109: return "WSATYPE_NOT_FOUND: Class type not found."; break;
    case 10110: return "WSA_E_NO_MORE: No more results."; break;
    case 10111: return "WSA_E_CANCELLED: Call was canceled."; break;
    case 10112: return "WSAEREFUSED: Database query was refused."; break;
    case 11001: return "WSAHOST_NOT_FOUND: Host not found."; break;
    case 11002: return "WSATRY_AGAIN: Nonauthoritative host not found."; break;
    case 11003: return "WSANO_RECOVERY: This is a nonrecoverable error."; break;
    case 11004: return "WSANO_DATA: Valid name, no data record of requested type."; break;

    default:;

      // http://msdn.microsoft.com/en-us/library/ms680582(v=vs.85).aspx

      // Retrieve the system error message for the last-error code

      LPVOID lpMsgBuf;
      LPVOID lpDisplayBuf;
      DWORD dw = GetLastError();

      FormatMessage(
          FORMAT_MESSAGE_ALLOCATE_BUFFER |
          FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
          NULL,
          dw,
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          (LPTSTR) &lpMsgBuf,
          0, NULL );

      char *unknown = NULL;

      if (lpMsgBuf != NULL) {
        // Strip CRLF from end of message.
        char *message = malloc(strlen(lpMsgBuf) + 1);
        char *i = lpMsgBuf, *o = message;
        for (i = lpMsgBuf; *i != 0; i++) {
          if (*i < ' ') continue;
          *o++ = *i;
        };
        *o = 0;
        easy_sprintf(&unknown, "Error %d: %s.", error_code, message);
        free(message);
      } else {
        easy_sprintf(&unknown, "Error %d: Whatever that means...", error_code);
      };

      LocalFree(lpMsgBuf);

    return unknown;

  };
  #endif

};

//--page-split-- easy_memory__initialize

static void easy_memory__initialize(char *file, int line) {
  #ifndef EASY_RAW_MALLOC
  if (next(first) != NULL || prev(last) != NULL) {
    easy_fuck("easy_initialize() has been called twice");
  };
  #ifndef EASY_NO_THREADS
  pthread_rwlock_init(&memory_lock, NULL);
  pthread_rwlock_wrlock(&memory_lock);
  #endif
  easy_random_binary_string(&random_number, sizeof(random_number));
  //fprintf(stderr, "Canary value is %#08x\n", random_number);
  next(first) = last;
  prev(last) = first;
  #ifndef EASY_NO_THREADS
  pthread_rwlock_unlock(&memory_lock);
  #endif
  #endif
};

//--page-split-- commas

static char *commas (int64_t number) {
  static int i = 0;
  if (++i >= 4) i = 0;
  static char destination[4][64];
  static char temporary[32];
  snprintf(temporary, 32, "%ld", number);
  int l = strlen(temporary);
  int c = (l - 1) / 3;
  //if (l < 5) c = 0;
  char *s = temporary + l;
  char *d = destination[i] + l + c;
  *d = 0;
  while (c--) {
    *--d = *--s;
    *--d = *--s;
    *--d = *--s;
    *--d = ',';
  };
  while (d > destination[i]) *--d = *--s;
  return destination[i];
};

//--page-split-- easy_memory__allow

void easy_memory__allow (char *file, int line, int allocation_count, int64_t allocation_size) {
  #ifndef EASY_RAW_MALLOC
  #ifndef EASY_NO_THREADS
  pthread_rwlock_wrlock(&memory_lock);
  #endif
  int i;
  for (i = 0; i < allowance_count; i++) {
    if (allowance[i].file == file && allowance[i].line == line) break;
  };
  if (i < allowance_count) {
    if (allowance[i].count < allocation_count) allowance[i].count = allocation_count;
    if (allowance[i].size < allocation_size) allowance[i].size = allocation_size;
  } else {
    allowance = realloc(allowance, ++allowance_count * sizeof(allowance[0]));
    allowance[i].file = file;
    allowance[i].line = line;
    allowance[i].count = allocation_count;
    allowance[i].size = allocation_size;
  };
  #ifndef EASY_NO_THREADS
  pthread_rwlock_unlock(&memory_lock);
  #endif
  #endif
};

//--page-split-- easy_memory__terminate

static void easy_memory__terminate(char *file, int line) {
  #ifndef EASY_RAW_MALLOC
  easy_memory__canary_check(file, line, NULL);
  #ifndef EASY_NO_THREADS
  pthread_rwlock_rdlock(&memory_lock);
  #endif
  #if 0
  fprintf(stderr, "Memory allowances:\n");
  for (int i = 0; i < allowance_count; i++) {
    fprintf(stderr, "  %s:%d -- %s bytes in %s allocations\n", allowance[i].file, allowance[i].line, commas(allowance[i].size), commas(allowance[i].count));
  };
  #endif
  if (next(first) == NULL || prev(last) == NULL) {
    easy_fuck("easy_terminate() has been called before easy_initialize()");
  };
  struct {
    int count;
    int64_t size;
    char *file;
    int line;
  } *leak_array = NULL;
  int leak_count = 0;
  int multi_leaks = 0;
  int large_leaks = 0;
  for (void *pointer = next(first); pointer != last; pointer = next(pointer)) {
    //if (flags(pointer) & 1) continue; // memory_allocate_once
    int i;
    for (i = 0; i < allowance_count; i++) {
      if (allowance[i].file != file(pointer)) continue;
      if (allowance[i].line != line(pointer)) continue;
      allowance[i].count--;
      allowance[i].size -= size(pointer);
      break;
    };
    if (i >= allowance_count || allowance[i].count < 0 || allowance[i].size < 0) {
      for (i = 0; i < leak_count; i++) {
        if (leak_array[i].file != file(pointer)) continue;
        if (leak_array[i].line != line(pointer)) continue;
        if (size(pointer) >= 4096) large_leaks = 1;
        multi_leaks = 1;
        leak_array[i].size += size(pointer);
        leak_array[i].count++;
        break;
      };
      if (i == leak_count) {
        if (size(pointer) >= 4096) large_leaks = 1;
        leak_array = realloc(leak_array, (size_t) (leak_count + 1) * sizeof(leak_array[0]));
        leak_array[i].file = file(pointer);
        leak_array[i].line = line(pointer);
        leak_array[i].size = size(pointer);
        leak_array[i].count = 1;
        leak_count++;
      };
    };
  };
  // Only report lines that resulted in more than one allocation.
  if (multi_leaks || large_leaks) {
    fprintf(stderr, "Possible memory leaks:\n");
    for (int i = 0; i < leak_count; i++) {
      if (leak_array[i].count > 1 || leak_array[i].size >= 4096) {
        fprintf(stderr, "  %s:%d: %s bytes in %s allocations\n", leak_array[i].file, leak_array[i].line, commas(leak_array[i].size), commas(leak_array[i].count));
      };
    };
  };
  // Unless we want to see them all...
  #if EASY_ALL_LEAKS
  if (leak_count) {
    fprintf(stderr, "Unlikely memory leaks:\n");
    for (int i = 0; i < leak_count; i++) {
      if (leak_array[i].count == 1 && leak_array[i].size < 4096) {
        fprintf(stderr, "  %s:%d: %s bytes in one allocation\n", leak_array[i].file, leak_array[i].line, commas(leak_array[i].size));
      };
    };
    fprintf(stderr, "Exiting with error code due to memory leaks.\n");
    exit(1);
  };
  #endif
  // ...or just compare to the full list...
  #if 0
  fprintf(stderr, "All remaining memory allocations:\n");
  for (void *pointer = next(first); pointer != last; pointer = next(pointer)) {
    fprintf(stderr, "  %s:%d: %s bytes\n", file(pointer), line(pointer), commas(size(pointer)));
  };
  #endif
  free(leak_array);
  // Actually free the memory so that anything valgrind says is something we don't know about...
  // This also might cause malloc to throw some errors so we do it after generating our output.
  void *pointer = next(first);
  while (pointer != last) {
    void *next = next(pointer);
    free(pointer - SIZE);
    pointer = next;
  };
  next(first) = NULL;
  prev(last) = NULL;
  #ifndef EASY_NO_THREADS
  pthread_rwlock_unlock(&memory_lock);
  pthread_rwlock_destroy(&memory_lock);
  #endif
  #endif
};

#ifndef EASY_RAW_MALLOC

//--page-split-- easy_memory__canary_check

int easy_memory__canary_check(char *file, int line, void *search) {
  #ifndef EASY_NO_THREADS
  pthread_rwlock_rdlock(&memory_lock);
  #endif
  if (next(first) == NULL || prev(last) == NULL) {
    easy__fuck(file, line, 1, "easy_initialize() has not been called.");
  };
  int fuck = 0;
  int found = 0;
  if (search == NULL) found = 1;
  if (search) {
    void *pointer = search;
    found = 1;
  //for (void *pointer = next(first); pointer != last; pointer = next(pointer)) {
    //if (pointer == search) found = 1;
    if (test0(pointer) != random_number) {
      fprintf(stderr, "Canary value placed before memory allocated in %s line %d has been destroyed.\n", file(pointer), line(pointer));
      fuck = 1;
    };
    if (test1(pointer) != random_number) {
      fprintf(stderr, "Canary value placed after memory allocated in %s line %d has been destroyed.\n", file(pointer), line(pointer));
      fuck = 1;
    };
  };
  if (fuck) easy__fuck(file, line, 1, "It's also possible that an invalid pointer was passed to easy_memory_allocate().\n");
  #ifndef EASY_NO_THREADS
  pthread_rwlock_unlock(&memory_lock);
  #endif
  return !found;
};
#endif

//--page-split-- easy_sha1

/*

  Implemented from standard documentation at:
  http://www.itl.nist.gov/fipspubs/fip180-1.htm
  Function and variable names are from that documentation.
  Hooray for standards documents!

  Function sha1() parameters:

    char *digest:  an output buffer for the binary message digest (20 bytes)

    char *message:  a buffer containing the message data.

    int length:  the length of the message, in bytes.

  No return value.  It just does what it does and that's it.

  Written for a little-endian architecture.  Big endian sucks!

*/

struct sha1_state {
  unsigned int a, b, c, d, e;
  unsigned int h[5];
  unsigned int w[80];
  unsigned int temp;
};

#define a state->a
#define b state->b
#define c state->c
#define d state->d
#define e state->e
#define h state->h
#define w state->w
#define temp state->temp

#define SWAP(X) ((((unsigned int) (X) & 0xFF) << 24) | (((unsigned int) (X) & 0xFF00) << 8) | (((unsigned int) (X) & 0xFF0000) >> 8) | (((unsigned int) (X) & 0xFF000000) >> 24))

  static void sha1_process(struct sha1_state *state, const unsigned char *pointer) {
    // This code was auto-generated by scripts/sha1.pl
    w[0] = SWAP(*((unsigned int*) &pointer[0]));
    w[1] = SWAP(*((unsigned int*) &pointer[4]));
    w[2] = SWAP(*((unsigned int*) &pointer[8]));
    w[3] = SWAP(*((unsigned int*) &pointer[12]));
    w[4] = SWAP(*((unsigned int*) &pointer[16]));
    w[5] = SWAP(*((unsigned int*) &pointer[20]));
    w[6] = SWAP(*((unsigned int*) &pointer[24]));
    w[7] = SWAP(*((unsigned int*) &pointer[28]));
    w[8] = SWAP(*((unsigned int*) &pointer[32]));
    w[9] = SWAP(*((unsigned int*) &pointer[36]));
    w[10] = SWAP(*((unsigned int*) &pointer[40]));
    w[11] = SWAP(*((unsigned int*) &pointer[44]));
    w[12] = SWAP(*((unsigned int*) &pointer[48]));
    w[13] = SWAP(*((unsigned int*) &pointer[52]));
    w[14] = SWAP(*((unsigned int*) &pointer[56]));
    w[15] = SWAP(*((unsigned int*) &pointer[60]));
    temp = w[13] ^ w[8] ^ w[2] ^ w[0]; w[16] = (temp << 1) | (temp >> 31);
    temp = w[14] ^ w[9] ^ w[3] ^ w[1]; w[17] = (temp << 1) | (temp >> 31);
    temp = w[15] ^ w[10] ^ w[4] ^ w[2]; w[18] = (temp << 1) | (temp >> 31);
    temp = w[16] ^ w[11] ^ w[5] ^ w[3]; w[19] = (temp << 1) | (temp >> 31);
    temp = w[17] ^ w[12] ^ w[6] ^ w[4]; w[20] = (temp << 1) | (temp >> 31);
    temp = w[18] ^ w[13] ^ w[7] ^ w[5]; w[21] = (temp << 1) | (temp >> 31);
    temp = w[19] ^ w[14] ^ w[8] ^ w[6]; w[22] = (temp << 1) | (temp >> 31);
    temp = w[20] ^ w[15] ^ w[9] ^ w[7]; w[23] = (temp << 1) | (temp >> 31);
    temp = w[21] ^ w[16] ^ w[10] ^ w[8]; w[24] = (temp << 1) | (temp >> 31);
    temp = w[22] ^ w[17] ^ w[11] ^ w[9]; w[25] = (temp << 1) | (temp >> 31);
    temp = w[23] ^ w[18] ^ w[12] ^ w[10]; w[26] = (temp << 1) | (temp >> 31);
    temp = w[24] ^ w[19] ^ w[13] ^ w[11]; w[27] = (temp << 1) | (temp >> 31);
    temp = w[25] ^ w[20] ^ w[14] ^ w[12]; w[28] = (temp << 1) | (temp >> 31);
    temp = w[26] ^ w[21] ^ w[15] ^ w[13]; w[29] = (temp << 1) | (temp >> 31);
    temp = w[27] ^ w[22] ^ w[16] ^ w[14]; w[30] = (temp << 1) | (temp >> 31);
    temp = w[28] ^ w[23] ^ w[17] ^ w[15]; w[31] = (temp << 1) | (temp >> 31);
    temp = w[29] ^ w[24] ^ w[18] ^ w[16]; w[32] = (temp << 1) | (temp >> 31);
    temp = w[30] ^ w[25] ^ w[19] ^ w[17]; w[33] = (temp << 1) | (temp >> 31);
    temp = w[31] ^ w[26] ^ w[20] ^ w[18]; w[34] = (temp << 1) | (temp >> 31);
    temp = w[32] ^ w[27] ^ w[21] ^ w[19]; w[35] = (temp << 1) | (temp >> 31);
    temp = w[33] ^ w[28] ^ w[22] ^ w[20]; w[36] = (temp << 1) | (temp >> 31);
    temp = w[34] ^ w[29] ^ w[23] ^ w[21]; w[37] = (temp << 1) | (temp >> 31);
    temp = w[35] ^ w[30] ^ w[24] ^ w[22]; w[38] = (temp << 1) | (temp >> 31);
    temp = w[36] ^ w[31] ^ w[25] ^ w[23]; w[39] = (temp << 1) | (temp >> 31);
    temp = w[37] ^ w[32] ^ w[26] ^ w[24]; w[40] = (temp << 1) | (temp >> 31);
    temp = w[38] ^ w[33] ^ w[27] ^ w[25]; w[41] = (temp << 1) | (temp >> 31);
    temp = w[39] ^ w[34] ^ w[28] ^ w[26]; w[42] = (temp << 1) | (temp >> 31);
    temp = w[40] ^ w[35] ^ w[29] ^ w[27]; w[43] = (temp << 1) | (temp >> 31);
    temp = w[41] ^ w[36] ^ w[30] ^ w[28]; w[44] = (temp << 1) | (temp >> 31);
    temp = w[42] ^ w[37] ^ w[31] ^ w[29]; w[45] = (temp << 1) | (temp >> 31);
    temp = w[43] ^ w[38] ^ w[32] ^ w[30]; w[46] = (temp << 1) | (temp >> 31);
    temp = w[44] ^ w[39] ^ w[33] ^ w[31]; w[47] = (temp << 1) | (temp >> 31);
    temp = w[45] ^ w[40] ^ w[34] ^ w[32]; w[48] = (temp << 1) | (temp >> 31);
    temp = w[46] ^ w[41] ^ w[35] ^ w[33]; w[49] = (temp << 1) | (temp >> 31);
    temp = w[47] ^ w[42] ^ w[36] ^ w[34]; w[50] = (temp << 1) | (temp >> 31);
    temp = w[48] ^ w[43] ^ w[37] ^ w[35]; w[51] = (temp << 1) | (temp >> 31);
    temp = w[49] ^ w[44] ^ w[38] ^ w[36]; w[52] = (temp << 1) | (temp >> 31);
    temp = w[50] ^ w[45] ^ w[39] ^ w[37]; w[53] = (temp << 1) | (temp >> 31);
    temp = w[51] ^ w[46] ^ w[40] ^ w[38]; w[54] = (temp << 1) | (temp >> 31);
    temp = w[52] ^ w[47] ^ w[41] ^ w[39]; w[55] = (temp << 1) | (temp >> 31);
    temp = w[53] ^ w[48] ^ w[42] ^ w[40]; w[56] = (temp << 1) | (temp >> 31);
    temp = w[54] ^ w[49] ^ w[43] ^ w[41]; w[57] = (temp << 1) | (temp >> 31);
    temp = w[55] ^ w[50] ^ w[44] ^ w[42]; w[58] = (temp << 1) | (temp >> 31);
    temp = w[56] ^ w[51] ^ w[45] ^ w[43]; w[59] = (temp << 1) | (temp >> 31);
    temp = w[57] ^ w[52] ^ w[46] ^ w[44]; w[60] = (temp << 1) | (temp >> 31);
    temp = w[58] ^ w[53] ^ w[47] ^ w[45]; w[61] = (temp << 1) | (temp >> 31);
    temp = w[59] ^ w[54] ^ w[48] ^ w[46]; w[62] = (temp << 1) | (temp >> 31);
    temp = w[60] ^ w[55] ^ w[49] ^ w[47]; w[63] = (temp << 1) | (temp >> 31);
    temp = w[61] ^ w[56] ^ w[50] ^ w[48]; w[64] = (temp << 1) | (temp >> 31);
    temp = w[62] ^ w[57] ^ w[51] ^ w[49]; w[65] = (temp << 1) | (temp >> 31);
    temp = w[63] ^ w[58] ^ w[52] ^ w[50]; w[66] = (temp << 1) | (temp >> 31);
    temp = w[64] ^ w[59] ^ w[53] ^ w[51]; w[67] = (temp << 1) | (temp >> 31);
    temp = w[65] ^ w[60] ^ w[54] ^ w[52]; w[68] = (temp << 1) | (temp >> 31);
    temp = w[66] ^ w[61] ^ w[55] ^ w[53]; w[69] = (temp << 1) | (temp >> 31);
    temp = w[67] ^ w[62] ^ w[56] ^ w[54]; w[70] = (temp << 1) | (temp >> 31);
    temp = w[68] ^ w[63] ^ w[57] ^ w[55]; w[71] = (temp << 1) | (temp >> 31);
    temp = w[69] ^ w[64] ^ w[58] ^ w[56]; w[72] = (temp << 1) | (temp >> 31);
    temp = w[70] ^ w[65] ^ w[59] ^ w[57]; w[73] = (temp << 1) | (temp >> 31);
    temp = w[71] ^ w[66] ^ w[60] ^ w[58]; w[74] = (temp << 1) | (temp >> 31);
    temp = w[72] ^ w[67] ^ w[61] ^ w[59]; w[75] = (temp << 1) | (temp >> 31);
    temp = w[73] ^ w[68] ^ w[62] ^ w[60]; w[76] = (temp << 1) | (temp >> 31);
    temp = w[74] ^ w[69] ^ w[63] ^ w[61]; w[77] = (temp << 1) | (temp >> 31);
    temp = w[75] ^ w[70] ^ w[64] ^ w[62]; w[78] = (temp << 1) | (temp >> 31);
    temp = w[76] ^ w[71] ^ w[65] ^ w[63]; w[79] = (temp << 1) | (temp >> 31);
    a = h[0]; b = h[1]; c = h[2]; d = h[3]; e = h[4];
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[0] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[1] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[2] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[3] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[4] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[5] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[6] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[7] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[8] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[9] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[10] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[11] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[12] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[13] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[14] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[15] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[16] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[17] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[18] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | ((~b) & d)) + e + w[19] + 0x5A827999;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[20] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[21] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[22] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[23] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[24] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[25] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[26] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[27] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[28] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[29] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[30] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[31] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[32] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[33] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[34] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[35] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[36] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[37] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[38] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[39] + 0x6ED9EBA1;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[40] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[41] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[42] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[43] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[44] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[45] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[46] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[47] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[48] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[49] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[50] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[51] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[52] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[53] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[54] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[55] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[56] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[57] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[58] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + ((b & c) | (b & d) | (c & d)) + e + w[59] + 0x8F1BBCDC;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[60] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[61] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[62] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[63] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[64] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[65] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[66] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[67] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[68] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[69] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[70] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[71] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[72] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[73] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[74] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[75] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[76] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[77] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[78] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + w[79] + 0xCA62C1D6;
    e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
    h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
  };

void easy_sha1(char *digest, const char *message, int length) {

  // To pad the message, copy its last partial block (if any) to a
  // zero-filled buffer, then add the padding and length to the buffer.

  int source_block_count = length >> 6;
  int buffer_block_count = (((length % 64) + 8) >> 6) + 1;

  unsigned char *buffer;
  buffer = malloc(128);
  memset(buffer, 0, 128);
  memmove(buffer, message + source_block_count * 64, length % 64);
  buffer[length % 64] = 0x80;

  // Then add the length, in bits, in big-endian format.

  *((unsigned int *) &buffer[buffer_block_count * 64 - 8]) = SWAP((unsigned int) length >> 29);
  *((unsigned int *) &buffer[buffer_block_count * 64 - 4]) = SWAP((unsigned int) length << 3);

  // Then just compute the digest!

  struct sha1_state *state;
  state = malloc(sizeof(struct sha1_state));

  h[0] = 0x67452301;
  h[1] = 0xEFCDAB89;
  h[2] = 0x98BADCFE;
  h[3] = 0x10325476;
  h[4] = 0xC3D2E1F0;

  for (int i = 0; i < source_block_count; i++) {
    sha1_process(state, (unsigned char *) message + i * 64);
  };

  for (int i = 0; i < buffer_block_count; i++) {
    sha1_process(state, buffer + i * 64);
  };

  free(buffer);

  // Then make it big-endian.  (stupid big-endian bullshit)

  *((unsigned int *) &digest[0]) = SWAP(h[0]);
  *((unsigned int *) &digest[4]) = SWAP(h[1]);
  *((unsigned int *) &digest[8]) = SWAP(h[2]);
  *((unsigned int *) &digest[12]) = SWAP(h[3]);
  *((unsigned int *) &digest[16]) = SWAP(h[4]);

  free(state);

  // That was so easy!

};

#undef a
#undef b
#undef c
#undef d
#undef e
#undef h
#undef w
#undef temp
#undef SWAP

//--page-split-- easy_thread_set_init_flag

#ifndef EASY_NO_THREADS
void easy_thread_set_init_flag(struct easy_thread_state *thread_state) {
  while(sem_post(&thread_state->init));
};
#endif

//--page-split-- easy_thread_get_init_flag

#ifndef EASY_NO_THREADS
int easy_thread_get_init_flag(struct easy_thread_state *thread_state) {
  if (easy_sem_trywait(&thread_state->init)) return 0;
  easy_sem_post(&thread_state->init); return 1;
};
#endif

//--page-split-- easy_thread_wait_init_flag

#ifndef EASY_NO_THREADS
void easy_thread_wait_init_flag(struct easy_thread_state *thread_state) {
  easy_sem_wait(&thread_state->init);
  easy_sem_post(&thread_state->init);
};
#endif

//--page-split-- easy_thread_set_exit_flag

#ifndef EASY_NO_THREADS
void easy_thread_set_exit_flag(struct easy_thread_state *thread_state) {
  easy_sem_post(&thread_state->exit);
};
#endif

//--page-split-- easy_thread_get_exit_flag

#ifndef EASY_NO_THREADS
int easy_thread_get_exit_flag(struct easy_thread_state *thread_state) {
  if (easy_sem_trywait(&thread_state->exit)) return 0;
  easy_sem_post(&thread_state->exit); return 1;
};
#endif

//--page-split-- easy_thread_wait_exit_flag

#ifndef EASY_NO_THREADS
void easy_thread_wait_exit_flag(struct easy_thread_state *thread_state) {
  easy_sem_wait(&thread_state->exit);
  easy_sem_post(&thread_state->exit);
};
#endif

//--page-split-- easy_thread_fork

// Spawn thread and wait for thread to signal that initialization is complete.

#ifndef EASY_NO_THREADS
void easy_thread_fork(struct easy_thread_state *thread_state, void * (*function)(void *), void *parameter) {
  //if (!signal_handler_registered) easy_fuck("easy_initialize() must be called before easy_thread_spawn()");
  easy_sem_init(&thread_state->init, 0, 0);
  easy_sem_init(&thread_state->exit, 0, 0);
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  if (0) {
    // Rather than do this, it's probably a better idea that, where functions
    // need big amounts of data, to only put a pointer on the stack and just
    // use malloc() to obtain the memory, instead of allocating on the stack.
    size_t stacksize;
    pthread_attr_getstacksize(&attr, &stacksize);
    printf("Default stack size: %d\n", (int) stacksize);
    stacksize += 8 * 1048576;
    pthread_attr_setstacksize(&attr, stacksize);
    pthread_attr_getstacksize(&attr, &stacksize);
    printf("Stack size is now: %d\n", (int) stacksize);
  };
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&thread_state->thread, &attr, function, parameter);
  pthread_attr_destroy(&attr);
};
#endif

//--page-split-- easy_thread_join

// Wait for a thread to decide to exit on its own.

#ifndef EASY_NO_THREADS
void easy_thread_join(struct easy_thread_state *thread_state) {
  pthread_join(thread_state->thread, NULL);
  easy_sem_destroy(&thread_state->init);
  easy_sem_destroy(&thread_state->exit);
};
#endif

//--page-split-- easy__sem_init

#ifndef EASY_NO_THREADS
void easy__sem_init(char *file, int line, sem_t *sem, int pshared, unsigned int value) {
  int rv = sem_init(sem, pshared, value);
  if (rv) easy__fuck(file, line, 1, "sem_init() returned %s", easy_error_string(errno));
};
#endif

//--page-split-- easy__sem_post

#ifndef EASY_NO_THREADS
void easy__sem_post(char *file, int line, sem_t *sem) {
  int rv = sem_post(sem);
  if (rv) easy__fuck(file, line, 1, "sem_post() returned %s", easy_error_string(errno));
};
#endif

//--page-split-- easy__sem_wait

#ifndef EASY_NO_THREADS
void easy__sem_wait(char *file, int line, sem_t *sem) {
  int rv = sem_wait(sem);
  if (rv) easy__fuck(file, line, 1, "sem_wait() returned %s", easy_error_string(errno));
};
#endif

//--page-split-- easy__sem_trywait

#ifndef EASY_NO_THREADS
int easy__sem_trywait(char *file, int line, sem_t *sem) {
  int rv = sem_trywait(sem);
  if (rv && errno != EAGAIN) easy__fuck(file, line, 1, "sem_trywait() returned %s", easy_error_string(errno));
  return rv;
};
#endif

//--page-split-- easy__sem_destroy

#ifndef EASY_NO_THREADS
void easy__sem_destroy(char *file, int line, sem_t *sem){
  int rv = sem_destroy(sem);
  if (rv) easy__fuck(file, line, 1, "sem_destroy() returned %s", easy_error_string(errno));
};
#endif

//--page-split-- easy__mutex_init

#ifndef EASY_NO_THREADS
void easy__mutex_init(char *file, int line, pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
  int rv = pthread_mutex_init(mutex, attr);
  if (rv) easy__fuck(file, line, 1, "pthread_mutex_init() returned %s", easy_error_string(errno));
};
#endif

//--page-split-- easy__mutex_lock

#ifndef EASY_NO_THREADS
void easy__mutex_lock(char *file, int line, pthread_mutex_t *mutex) {
  int rv = pthread_mutex_lock(mutex);
  if (rv) easy__fuck(file, line, 1, "pthread_mutex_lock() returned %s", easy_error_string(errno));
};
#endif

//--page-split-- easy__mutex_trylock

#ifndef EASY_NO_THREADS
int easy__mutex_trylock(char *file, int line, pthread_mutex_t *mutex) {
  int rv = pthread_mutex_trylock(mutex);
  if (rv && errno != EAGAIN) easy__fuck(file, line, 1, "pthread_mutex_init() returned %s", easy_error_string(errno));
};
#endif

//--page-split-- easy__mutex_unlock

#ifndef EASY_NO_THREADS
void easy__mutex_unlock(char *file, int line, pthread_mutex_t *mutex) {
  int rv = pthread_mutex_unlock(mutex);
  if (rv) easy__fuck(file, line, 1, "pthread_mutex_unlock() returned %s", easy_error_string(errno));
};
#endif

//--page-split-- easy__mutex_destroy

#ifndef EASY_NO_THREADS
void easy__mutex_destroy(char *file, int line, pthread_mutex_t *mutex) {
  int rv = pthread_mutex_destroy(mutex);
  if (rv) easy__fuck(file, line, 1, "pthread_mutex_destroy() returned %s", easy_error_string(errno));
};
#endif

//--page-split-- easy__fuck

void easy__fuck(char *file, int line, int fatal, char *format, ...) {
  va_list ooo;
  va_start(ooo, format);
  #ifdef LINUX
  fprintf(stderr, fatal ? "\e[1;31m" : "\e[1;33m");
  #endif
  fprintf(stderr, "%s:%d -- ", file, line);
  vfprintf(stderr, format, ooo);
  #ifdef LINUX
  fprintf(stderr, "\e[0m");
  #endif
  fprintf(stderr, "\n");
  va_end(ooo);
  if (fatal) exit(1);
};

//--page-split-- easy_error

//void easy_error(char * message, int error_code) {
//  printf("%s: %s\n", message, easy_error_string(error_code));
//  exit(1);
//};

//--page-split-- easy_actual_time

double easy_actual_time(void) {
  #ifdef WINDOWS
    easy_fuck("I don't know how to do this in Windows.");
  #else
    struct timespec bullshit;
    clock_gettime(CLOCK_REALTIME, &bullshit);
    return bullshit.tv_sec + bullshit.tv_nsec / 1000000000.0;
  #endif
};

//--page-split-- easy_time

double easy_time(void) {
  #ifdef WINDOWS
    //void QueryUnbiasedInterruptTimePrecise( [out] PULONGLONG lpUnbiasedInterruptTimePrecise );  windows 10 only
    //BOOL QueryPerformanceFrequency( [out] LARGE_INTEGER *lpFrequency );
    //BOOL QueryPerformanceCounter( [out] LARGE_INTEGER *lpPerformanceCount );
    static int64_t frequency = 0;
    if (!frequency) QueryPerformanceFrequency((LARGE_INTEGER *) &frequency);
    int64_t now;
    QueryPerformanceCounter((LARGE_INTEGER *) &now);
    return (double) now / frequency;
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

void easy_seed_random_number_generator(void) {
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
    // XOR together, the randomness is more consistent.

    int reverse = 0;
    for (int i = 0; i < 31; i++) {
      reverse |= ((two >> i) & 1) << (30 - i);
    };

    int seed = one ^ reverse;

    //printf("one=%.10d  two=%.10d  reverse=%.10d  seed=%.10d\n", one, two, reverse, seed);

    srand((unsigned int) seed);

  };
};

//--page-split-- easy_random_binary_string

void easy_random_binary_string(void *string, int bytes) {
  #ifdef WINDOWS
    easy_seed_random_number_generator();
    if (RAND_MAX < 255) easy_fuck("C is so unprepared to accomplish anything.");
    for (int i = 0; i < bytes; i++) {
      ((unsigned char *) string)[i] = rand() & 0xFF;
    };
  #else
    FILE *urandom;
    urandom = fopen("/dev/urandom", "rb");
    if (0 == urandom) easy_fuck("cannot open /dev/urandom");
    if (0 == fread(string, (size_t) bytes, 1, urandom)) easy_fuck("cannot read /dev/urandom");
    fclose(urandom);
  #endif
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

//--page-split-- easy_random

int easy_random(int limit) {

  easy_seed_random_number_generator();

  // creates random number between 0 and limit - 1

  if (limit < 1) easy_fuck("invalid random number limit");
  if (limit == 1) return 0;

  if (RAND_MAX < limit - 1) easy_fuck("C is so unprepared to accomplish anything.");

  int mask = limit - 1;
  mask |= mask >> 1;
  mask |= mask >> 2;
  mask |= mask >> 4;
  mask |= mask >> 8;
  mask |= mask >> 16;

  int random;
  do {
    random = rand() & mask;
  } while (random >= limit);

  return random;

};

//--page-split-- easy_strnlen

int easy_strnlen(char *string, int limit) {
  int i;
  for (i = 0; i < limit; i++) {
    if (string[i] == 0) break;
  };
  return i;
};

//--page-split-- easy__vsprintf

int easy__vsprintf(char *file, int line, char **destination, const char *format, va_list ooo) {
  va_list copy;
  va_copy(copy, ooo);
  int size = vsnprintf(NULL, 0, format, copy);
  va_end(copy);
  while (1) {
    easy_memory__allocate((void **) destination, size + 1, file, line);
    va_copy(copy, ooo);
    int result = vsnprintf(*destination, (size_t) (size + 1), format, copy);
    va_end(copy);
    if (result == size) break;
    fprintf(stderr, "\e[1;31mvsnprintf() told us the string would be %d bytes, but then told us it should be %d bytes.\e[0m\n", size, result);
    size = result;
  };
  return size;
};

//--page-split-- easy__strcpy

void easy__strcpy(char *file, int line, char **destination, char *source) {
  if (source == NULL) {
    easy_memory__allocate((void **) destination, 0, file, line);
  } else {
    int length = strlen(source);
    easy_memory__allocate((void **) destination, length + 1, file, line);
    memmove(*destination, source, length + 1);
  };
};

//--page-split-- easy__sprintf

int easy__sprintf(char *file, int line, char **destination, char *format, ...) {
  va_list ooo;
  va_start(ooo, format);
  int size = easy__vsprintf(file, line, destination, format, ooo);
  va_end(ooo);
  return size;
};

//--page-split-- easy_buffer__append_sprintf

void easy_buffer__append_sprintf(char *file, int line, struct easy_buffer *buffer, char *format, ...) {
  char *destination = NULL;
  va_list ooo;
  va_start(ooo, format);
  int size = easy__vsprintf(file, line, &destination, format, ooo);
  va_end(ooo);
  easy_buffer__append(buffer, destination, size, file, line);
  easy_memory_allocate(&destination, 0);
};

//--page-split-- easy_string__allocate

void easy_string__allocate (struct easy_string *string, int size, char *file, int line) {

  // This function picks a new allocation size for string data, such that the
  // allocation size is the next larger power of two, but does not shrink the
  // allocation if it is not greater than 4kB or it is less than two sizes too
  // big, in order to avoid calling realloc() hundreds of times in cases where
  // it makes basically no difference how large the allocation is anyway.

  size++; // Always leave room for a null character at the end.

  if (size <= string->allocation_size && string->allocation_size <= 4096) return;
  ssize_t desired_size = 1; // ssize_t so that we don't fail at 1 GB
  while (desired_size < size) desired_size <<= 1;
  if (desired_size == (string->allocation_size >> 1)) {
    // don't shrink if only shrinking by a factor of two
    desired_size = string->allocation_size;
  };
  if (desired_size > string->allocation_size) {
    // If the space is too small, it obviously has to get bigger.
    easy_memory__allocate((void **) &string->buffer, desired_size, file, line);
    // If there previously was no allocation, we should add a null byte at the end.
    if (string->allocation_size == 0) string->buffer[0] = 0;
    string->allocation_size = desired_size;
  } else if (desired_size < string->allocation_size) {
    // ...but if the space is too big, let's think about it.
    if (string->allocation_size < 4096) {
      // If the current size isn't that large anyway, then do nothing.
      // If we're reading a text file and one line is 75 bytes and the
      // next line is 5 bytes, it's reasonable to think that another
      // 75 byte line might come along, and until then, no one gives a
      // fuck about wasting 70 bytes of memory.
    } else {
      // However, if the current size is large, then we shouldn't shrink
      // it more than we would have above, as that would just be silly.
      if (desired_size < 4096) desired_size = 4096;
      easy_memory__allocate((void **) &string->buffer, desired_size, file, line);
      string->allocation_size = desired_size;
    };
  };
  assert(string->allocation_size >= size);

};

//--page-split-- add_commas

static struct easy_string *add_commas (struct easy_string *original) {

  // We want to do this:
  //        100 -> 100             3->3
  //       1000 -> 1000            4->4
  //      10000 -> 10,000          5->6
  //     100000 -> 100,000         6->7
  //    1000000 -> 1,000,000       7->9
  //   10000000 -> 10,000,000      8->10
  //  100000000 -> 100,000,000     9->11
  // 1000000000 -> 1,000,000,000  10->13

  // Figure out how long the new string will be.

  char *p = index(original->buffer, '.');
  if (!p) p = original->buffer + original->length;
  int number_of_digits = (p - original->buffer);
  if (original->buffer[0] == '+' || original->buffer[0] == '-') number_of_digits--;
  int number_of_commas = (number_of_digits - 1) / 3;
  if (number_of_digits < 5) number_of_commas = 0;
  int new_string_length = original->length + number_of_commas;

  //fprintf(stderr, "old_string_length = %d\n", original->length);
  //fprintf(stderr, "number_of_digits = %d\n", number_of_digits);
  //fprintf(stderr, "number_of_commas = %d\n", number_of_commas);
  //fprintf(stderr, "new_string_length = %d\n", new_string_length);

  // grab a new temporary easy string and allocate correct length

  struct easy_string *destination = &temporary[temporary_index++];
  if (temporary_index >= EASY_TEMPORARY_STRING_COUNT) temporary_index = 0;
  easy_string__allocate(destination, new_string_length, __FILE__, __LINE__);

  // copy the decimal point and whatever comes after it, including terminating null byte

  memmove(destination->buffer + (p - original->buffer) + number_of_commas, p, original->length - (p - original->buffer) + 1);

  // then work backwards to add the commas

  char *d = destination->buffer + (p - original->buffer) + number_of_commas;
  while (number_of_commas) {
    *--d = *--p;
    *--d = *--p;
    *--d = *--p;
    *--d = ',';
    number_of_commas--;
  };
  while (d > destination->buffer) *--d = *--p;

  return destination;
};

//--page-split-- easy_commas_int

char *easy_commas_int (int64_t number) {
  return add_commas(easy_string_format(NULL, "%ld", number))->buffer;
};

//--page-split-- easy_commas_float

char *easy_commas_float (double number, int decimals) {
  return add_commas(easy_string_format(NULL, easy_string_format(NULL, "%%0.%df", decimals)->buffer, number))->buffer;
};

//--page-split-- easy_string__aa_write

struct easy_string *easy_string__aa_write (struct easy_string_aa *aa, const char *key, char *file, int line) {

  int i;
  for (i = 0; i < aa->count; i++) {
    if (strcmp(aa->key[i], key) == 0) {
      return &aa->value[i];
    };
  };
  aa->count++;

  easy_memory__allocate((void **) &aa->key, aa->count * (int) sizeof(aa->key[0]), file, line);
  aa->key[i] = NULL;
  int key_length = strlen(key);
  easy_memory__allocate((void **) &aa->key[i], key_length + 1, file, line);
  strcpy(aa->key[i], key);

  easy_memory__allocate((void **) &aa->value, aa->count * (int) sizeof(aa->value[0]), file, line);
  memset(&aa->value[i], 0, sizeof(aa->value[0]));

  return &aa->value[i];

};

//--page-split-- easy_string__aa_read

struct easy_string *easy_string__aa_read(const struct easy_string_aa *aa, const char *key, char *file, int line) {
  for (int i = 0; i < aa->count; i++) {
    if (strcmp(aa->key[i], key) == 0) return &aa->value[i];
  };
  return NULL;
};

//--page-split-- easy_string__aa_delete

void easy_string__aa_delete(struct easy_string_aa *aa, const char *key, char *file, int line) {
  for (int i = 0; i < aa->count; i++) {
    if (strcmp(aa->key[i], key) == 0) {
      easy_memory__allocate((void **) &aa->key[i], 0, file, line);
      easy_memory__allocate((void **) &aa->value[i].buffer, 0, file, line);
      memmove(&aa->key[i], &aa->key[i + 1], (aa->count - i - 1) * sizeof(aa->key[i]));
      memmove(&aa->value[i], &aa->value[i + 1], (aa->count - i - 1) * sizeof(aa->value[i]));
      aa->count--;
      easy_memory__allocate((void **) &aa->key, aa->count * (int) sizeof(aa->key[0]), file, line);
    };
  };
};

//--page-split-- easy_string__aa_free

void easy_string__aa_free(struct easy_string_aa *aa, char *file, int line) {
  for (int i = 0; i < aa->count; i++) {
    easy_memory__allocate((void **) &aa->key[i], 0, file, line);
    easy_memory__allocate((void **) &aa->value[i].buffer, 0, file, line);
  };
  easy_memory__allocate((void **) &aa->key, 0, file, line);
  easy_memory__allocate((void **) &aa->value, 0, file, line);
  aa->count = 0;
};

//--page-split-- template_nested

static void template_nested(struct easy_string *destination, const struct easy_string *template, const struct easy_string_aa *aa, const char *path, struct easy_string_array *included, char *file, int line) {
  easy_string__free(destination, file, line);
  char key[64]; // Longer keys will just be ignored, presumably they're actual HTML comments.
  int offset = 0;
  while (1) {
    char *where;
    where = memmem(template->buffer + offset, (size_t) (template->length - offset), "<!--", 4);
    if (where) {
      int start = where - template->buffer;
      easy_string__append_splice(destination, template, offset, start - offset, file, line);
      where = memmem(template->buffer + start, (size_t) (template->length - start), "-->", 3);
      if (where) {
        int finish = where - template->buffer + 3;
        int key_length = finish - start - 7;
        key[0] = 0;
        if (key_length < 64) {
          memmove(key, template->buffer + start + 4, key_length);
          key[key_length] = 0;
          struct easy_string *value = easy_string__aa_read(aa, key, file, line);
          if (value) {
            easy_string__splice(NULL, destination, destination->length, 0, value, file, line);
          } else if (path && (key_length < 3 || key[0] != '.' || key[1] != '.' || key[2] != '/') && !strstr(key, "/../")) {
            int found = 0;
            for (int i = 0; i < included->count; i++) {
              if (strcmp(included->string[i].buffer, key) == 0) found = 1;
            };
            if (!found) {
              easy_string_array_push(included, easy_string(key));
              struct easy_string nested_template = {};
              struct easy_string nested_destination = {};
              if (easy_string_load_file(&nested_template, "%s%s", path, key)) {
                if (nested_template.length && nested_template.buffer[nested_template.length - 1] == '\n') nested_template.length--;
                template_nested(&nested_destination, &nested_template, aa, path, included, file, line);
                easy_string__splice(NULL, destination, destination->length, 0, &nested_destination, file, line);
              };
              easy_string_free(&nested_template, &nested_destination);
              easy_string_array_pop(NULL, included);
            };
          };
        };
        offset = finish;
        continue;
      };
    };
    easy_string__append_splice(destination, template, offset, template->length - offset, file, line);
    break;
  };
};

//--page-split-- easy_string__template

void easy_string__template(struct easy_string *destination, const struct easy_string *template, const struct easy_string_aa *aa, const char *path, char *file, int line) {
  struct easy_string_array included = {};
  template_nested(destination, template, aa, path, &included, file, line);
  easy_string_array_free(&included);
};

//--page-split-- easy_string__format

struct easy_string *easy_string__format(char *file, int line, struct easy_string *destination, const char *format, ...) {
  if (destination == NULL) {
    destination = &temporary[temporary_index++];
    if (temporary_index >= EASY_TEMPORARY_STRING_COUNT) temporary_index = 0;
  };
  char *string = NULL;
  int length;
  va_list ooo;
  va_start(ooo, format);
  length = easy__vsprintf(file, line, &string, format, ooo);
  va_end(ooo);
  easy_string__allocate(destination, length, file, line);
  memmove(destination->buffer, string, length);
  destination->length = length;
  destination->buffer[destination->length] = 0;
  easy_memory__allocate((void **) &string, 0, file, line);
  return destination;
};

//--page-split-- easy_string__append_format

void easy_string__append_format(char *file, int line, struct easy_string *destination, const char *format, ...) {
  char *string = NULL;
  int length;
  va_list ooo;
  va_start(ooo, format);
  length = easy__vsprintf(file, line, &string, format, ooo);
  va_end(ooo);
  easy_string__allocate(destination, destination->length + length, file, line);
  memmove(destination->buffer + destination->length, string, length);
  destination->length += length;
  destination->buffer[destination->length] = 0;
  easy_memory__allocate((void **) &string, 0, file, line);
};

//--page-split-- easy_string__append_splice

void easy_string__append_splice(struct easy_string *destination, const struct easy_string *tape, int offset, int length, char *file, int line) {
  int original_offset = offset;
  int original_length = length;
  if (offset < 0) offset += tape->length;
  if (length < 0) {
    offset += length;
    length = -length;
  };
  if (offset < 0 || offset > tape->length || offset + length > tape->length) {
    easy__fuck(file, line, 1, "easy_string_append_splice: Substring extends outside of string.\nString's length is %d, substring's offset and length are %d and %d, which correspond to an actual offset and length of %d and %d\n", tape->length, original_offset, original_length, offset, length);
  };
  easy_string__allocate(destination, destination->length + length, file, line);
  memmove(destination->buffer + destination->length, tape->buffer + offset, length);
  destination->length += length;
  destination->buffer[destination->length] = 0;
};

//--page-split-- easy_string__copy

void easy_string__copy (struct easy_string *destination, const struct easy_string *source, char *file, int line) {
  easy_string__allocate(destination, source->length, file, line);
  memmove(destination->buffer, source->buffer, source->length);
  destination->length = source->length;
  destination->buffer[destination->length] = 0;
};

//--page-split-- easy_string__move

// this is like copy, but when it's done, the original string is null

void easy_string__move (struct easy_string *destination, struct easy_string *source, char *file, int line) {
  if (destination->buffer) easy_memory__allocate((void **) &destination->buffer, 0, file, line);
  *destination = *source;
  *source = (struct easy_string) {};
};

//--page-split-- easy_string__fuck_spaces

// removes leading, trailing, and runs of multiple spaces
void easy_string__fuck_spaces (struct easy_string *string, char *file, int line) {
  char *o = string->buffer;
  int last_was_space = 1;
  for (int i = 0; i < string->length; i++) {
    if (string->buffer[i] == ' ') {
      if (!last_was_space) *o++ = ' ';
      last_was_space = 1;
    } else {
      *o++ = string->buffer[i];
      last_was_space = 0;
    };
  };
  while (o > string->buffer && *(o - 1) == ' ') o--;
  string->length = o - string->buffer;
  string->buffer[string->length] = 0;
  if (string->allocation_size > 4096) easy_string__allocate(string, string->length, file, line);
};

//--page-split-- easy_string__array_push

struct easy_string *easy_string__array_push (struct easy_string_array *array, const struct easy_string *string, char *file, int line) {
  array->count++;
  easy_memory__allocate((void **) &array->string, (size_t) (array->count * (int) sizeof(array->string[0])), file, line);
  memset(&array->string[array->count - 1], 0, sizeof(array->string[0]));
  if (string) easy_string__copy(&array->string[array->count - 1], string, file, line);
  return &array->string[array->count - 1];
};

//--page-split-- easy_string__array_push_format

struct easy_string *easy_string__array_push_format (char *file, int line, struct easy_string_array *array,  const char *format, ...) {
  array->count++;
  easy_memory__allocate((void **) &array->string, (size_t) (array->count * (int) sizeof(array->string[0])), file, line);
  memset(&array->string[array->count - 1], 0, sizeof(array->string[0]));

  char *string = NULL;
  int length;
  va_list ooo;
  va_start(ooo, format);
  length = easy__vsprintf(file, line, &string, format, ooo);
  va_end(ooo);
  easy_string__allocate(&array->string[array->count - 1], length, file, line);
  memmove(array->string[array->count - 1].buffer, string, length);
  array->string[array->count - 1].length = length;
  array->string[array->count - 1].buffer[array->string[array->count - 1].length] = 0;
  easy_memory__allocate((void **) &string, 0, file, line);

  return &array->string[array->count - 1];
};

//--page-split-- easy_string__array_pop

void easy_string__array_pop (struct easy_string *string, struct easy_string_array *array, char *file, int line) {
  if (!array->count) easy__fuck(file, line, 1, "empty string array");
  if (string) easy_string__copy(string, &array->string[array->count - 1], file, line);
  easy_string__free(&array->string[array->count - 1], file, line);
  array->count--;
};

//--page-split-- easy_string__array_insert

void easy_string__array_insert (struct easy_string_array *array, int index, const struct easy_string *string, char *file, int line) {
  array->count++;
  easy_memory__allocate((void **) &array->string, array->count * sizeof(array->string[0]), file, line);
  memmove(&array->string[index + 1], &array->string[index], (array->count - index - 1) * sizeof(array->string[0]));
  memset(&array->string[index], 0, sizeof(array->string[0]));
  easy_string__copy(&array->string[index], string, file, line);
};

//--page-split-- easy_string__array_delete

void easy_string__array_delete (struct easy_string_array *array, int index, char *file, int line) {
  if (index < 0 || index >= array->count) easy__fuck(file, line, 1, "invalid string array index");
  easy_string__free(&array->string[index], file, line);
  memmove(&array->string[index], &array->string[index + 1], (array->count - index - 1) * sizeof(array->string[0]));
  array->count--;
  easy_memory_allocate(&array->string, array->count * sizeof(array->string[0]));
};

//--page-split-- easy_string__array_free

void easy_string__array_free (struct easy_string_array *array, char *file, int line) {
  for (int i = 0; i < array->count; i++) {
    easy_memory__allocate((void **) &array->string[i].buffer, 0, file, line);
  };
  easy_memory_allocate(&array->string, 0);
  array->count = 0;
};

//--page-split-- easy_string__join

struct easy_string *easy_string__join (struct easy_string *destination, const struct easy_string_array *array, const struct easy_string *divisor, char *file, int line) {
  if (array == NULL) easy__fuck(file, line, 1, "pointer to struct easy_string_array should not be null");
  if (destination == NULL) {
    destination = &temporary[temporary_index++];
    if (temporary_index >= EASY_TEMPORARY_STRING_COUNT) temporary_index = 0;
  };
  if (array->count) {
    destination->length = 0;
    for (int i = 0; i < array->count - 1; i++) {
      easy_string__splice(NULL, destination, destination->length, 0, &array->string[i], file, line);
      easy_string__splice(NULL, destination, destination->length, 0, divisor, file, line);
    };
    easy_string__splice(NULL, destination, destination->length, 0, &array->string[array->count - 1], file, line);
  } else {
    easy_string__allocate(destination, 0, file, line);
    destination->length = 0;
  };
  return destination;
};

//--page-split-- easy_string__split

void easy_string__split (struct easy_string_array *array, const struct easy_string *source, const struct easy_string *divisor, char *file, int line) {
  if (array == NULL) easy__fuck(file, line, 1, "pointer to struct easy_string_array should not be null");
  int array_limit = array->count;
  for (int i = 0; i < array->count; i++) {
    easy_memory__allocate((void **) &array->string[i].buffer, 0, file, line);
    array->string[i].allocation_size = 0;
  };
  array->count = 0;
  if (array_limit < 1) {
    array_limit = 1;
    easy_memory__allocate((void **) &array->string, array_limit * (int) sizeof(array->string[0]), file, line);
    memset(array->string, 0, sizeof(array->string[0]));
  };
  int offset = 0;
  while (1) {
    char *where;
    if (divisor->length) {
      where = memmem(source->buffer + offset, (size_t) (source->length - offset), divisor->buffer, (size_t) divisor->length);
    } else {
      if (offset == source->length) break;
      where = source->buffer + offset + 1;
    };
    if (array->count == array_limit) {
      array_limit *= 2;
      easy_memory_allocate(&array->string, array_limit * (int) sizeof(array->string[0]));
      memset(array->string + (array_limit / 2), 0, (size_t) (array_limit / 2) * sizeof(array->string[0]));
    };
    int length;
    if (where) {
      length = where - (source->buffer + offset);
    } else {
      length = source->length - offset;
    };
    easy_string__allocate(&array->string[array->count], length, file, line);
    memmove(array->string[array->count].buffer, source->buffer + offset, (size_t) length);
    array->string[array->count].buffer[length] = 0;
    array->string[array->count].length = length;
    array->count++;
    if (where == NULL) break;
    offset += length + divisor->length;
  };
  easy_memory_allocate(&array->string, array->count * (int) sizeof(array->string[0]));
};

//--page-split-- easy_string__fread

int easy_string__fread (struct easy_string *destination, FILE *the_file, int count, char *file, int line) {
  easy_string__allocate(destination, count, file, line);
  count = fread(destination->buffer, 1, (size_t) count, the_file);
  if (count >= 0) {
    destination->buffer[count] = 0;
    destination->length = count;
    if (destination->allocation_size > 4096) easy_string__allocate(destination, count, file, line);
  } else {
    easy_string__allocate(destination, 0, file, line);
    destination->buffer[0] = 0;
    destination->length = 0;
    easy_warn("%s:%d: fread() returned %d: %s\n", file, line, count, easy_error_string(count));
  };
  return count;
};

//--page-split-- easy_string__load_file

int easy_string__load_file(char *file, int line, struct easy_string *destination, const char *format, ...) {
  char *string = NULL;
  va_list ooo;
  va_start(ooo, format);
  easy__vsprintf(file, line, &string, format, ooo);
  va_end(ooo);
  FILE *the_file = fopen(string, "rb");
  easy_memory__allocate((void **) &string, 0, file, line);
  if (the_file) {
    easy_string__fread_all(destination, the_file, file, line);
    fclose(the_file);
    return 1;
  } else {
    return 0;
  };
};

//--page-split-- easy_string__save_file

int easy_string__save_file(char *file, int line, struct easy_string *source, const char *format, ...) {
  char *string = NULL;
  va_list ooo;
  va_start(ooo, format);
  easy__vsprintf(file, line, &string, format, ooo);
  va_end(ooo);
  FILE *the_file = fopen(string, "wb");
  easy_memory_allocate(&string, 0);
  if (the_file) {
    if (fwrite(source->buffer, 1, source->length, the_file) == source->length) {
      fclose(the_file);
      return 1;
    } else {
      fclose(the_file);
      return 0;
    };
  } else {
    return 0;
  };
};

//--page-split-- easy_string__fread_all

void easy_string__fread_all (struct easy_string *destination, FILE *the_file, char *file, int line) {
  if (destination->allocation_size < 1) {
    destination->allocation_size = 1;
    easy_memory__allocate((void **) &destination->buffer, destination->allocation_size, file, line);
  };
  destination->length = 0;
  destination->buffer[0] = 0;
  while (1) {
    int count = fread(destination->buffer + destination->length, 1, (size_t) (destination->allocation_size - destination->length), the_file);
    if (count < 0) easy__fuck(file, line, 1, "Error while reading from file: %s", easy_error_string(errno));
    if (count == 0) break;
    destination->length += count;
    if (destination->length < destination->allocation_size) break;
    destination->allocation_size *= 2;
    easy_memory__allocate((void **) &destination->buffer, destination->allocation_size, file, line);
  };
  easy_string__allocate(destination, destination->length, file, line);
  destination->buffer[destination->length] = 0;
};

//--page-split-- easy_string__fgets

int easy_string__fgets (struct easy_string *destination, FILE *the_file, char *file, int line) {

  if (destination->allocation_size < 2) {
    destination->allocation_size = 2;
    easy_memory__allocate((void **) &destination->buffer, destination->allocation_size, file, line);
  };
  destination->length = 0;
  while (fgets(destination->buffer + destination->length, destination->allocation_size - destination->length, the_file)) {

    //fprintf(stderr, "length = %d, allocation_size = %d\n", destination->length, destination->allocation_size);
    char *null = memchr(destination->buffer + destination->length, 0, (size_t) (destination->allocation_size - destination->length));
    if (null > destination->buffer && *(null - 1) == '\n') {
      *--null = 0;
      if (null > destination->buffer && *(null - 1) == '\r') {
        *--null = 0;
      };
      destination->length = null - destination->buffer;
      //easy_string__allocate(destination, destination->length, file, line);
      //fprintf(stderr, "1 Read %d characters + newline from file: \"%s\"\n", destination->length, destination->buffer);
      return 1;
    };
    destination->length = null - destination->buffer;
    if (destination->length < destination->allocation_size - 1) {
      //fprintf(stderr, "2 Read %d characters from file: \"%s\"\n", destination->length, destination->buffer);
      return 1;
    };
    destination->allocation_size *= 2;
    easy_memory__allocate((void **) &destination->buffer, destination->allocation_size + 1, file, line);
  };
  //easy_string__allocate(destination, destination->length, file, line);
  if (destination->length) {
    //fprintf(stderr, "3 Read %d characters from file: \"%s\"\n", destination->length, destination->buffer);
    return 1;
  };
  //fprintf(stderr, "4 End of file.\n");
  return 0;
};

//--page-split-- easy_string__temp

struct easy_string *easy_string__temp (const struct easy_string *source, char *file, int line) {
  if (!source) easy__fuck(file, line, 1, "You don't want a null temporary string because they're only for very short term uses.");
  struct easy_string *t = &temporary[temporary_index++];
  if (temporary_index >= EASY_TEMPORARY_STRING_COUNT) temporary_index = 0;
  easy_string__allocate(t, source->length, file, line);
  memmove(t->buffer, source->buffer, source->length);
  t->length = source->length;
  t->buffer[t->length] = 0;
  return t;
};

//--page-split-- easy_string__splice

struct easy_string *easy_string__splice (struct easy_string *destination, struct easy_string *tape, int offset, int length, const struct easy_string *source, char *file, int line) {
  int original_offset = offset;
  int original_length = length;
  if (offset < 0) offset += tape->length;
  if (length < 0) {
    offset += length;
    length = -length;
  };
  if (offset < 0 || offset > tape->length || offset + length > tape->length) {
    easy__fuck(file, line, 1, "easy_string_splice: Substring extends outside of string.\nString's length is %d, substring's offset and length are %d and %d, which correspond to an actual offset and length of %d and %d\n", tape->length, original_offset, original_length, offset, length);
  };
  struct easy_string *t = destination;
  //if (!destination && !source) {
  if (!t) {
    t = &temporary[temporary_index++];
    if (temporary_index >= EASY_TEMPORARY_STRING_COUNT) temporary_index = 0;
  };
  if (t) {
    easy_string__allocate(t, length, file, line);
    memmove(t->buffer, tape->buffer + offset, length);
    t->length = length;
    t->buffer[length] = 0;
  };
  if (source) {
    int change = source->length - length;
    easy_string__allocate(tape, tape->length + change, file, line);
    memmove(tape->buffer + offset + source->length, tape->buffer + offset + length, tape->length - offset - length);
    memmove(tape->buffer + offset, source->buffer, source->length);
    tape->length += change;
    tape->buffer[tape->length] = 0;
  };
  return t;
};

//--page-split-- easy__initialize

void easy__initialize(char *file, int line) {
  #ifdef LINUX
  #ifndef NOBACKTRACE
  backtrace_initialize();
  #endif
  #endif
  easy_memory__initialize(file, line);
};

//--page-split-- easy_string_terminate

void easy_string_terminate (void) {
  for (int i = 0; i < EASY_TEMPORARY_STRING_COUNT; i++) {
    if (temporary[i].buffer) easy_memory_allocate(&temporary[i].buffer, 0);
  };
};

//--page-split-- easy__terminate

void easy__terminate (char *file, int line) {
  easy_string_terminate();
  easy_memory__terminate(file, line);
};

//--page-split-- easy__file_open

int easy__file_open(char *file, int line, char *pathname, int flags, int allow_errors) {
  #ifdef LINUX
  int fd;
  flags |= O_LARGEFILE;
  if (flags & O_CREAT) {
    fd = open(pathname, flags, 0666);
    if (!allow_errors && fd < 0) easy__fuck(file, line, 1, "%s while creating '%s'\n", easy_error_string(errno), pathname);
  } else {
    fd = open(pathname, flags);
    if (!allow_errors && fd < 0) easy__fuck(file, line, 1, "%s while opening '%s'\n", easy_error_string(errno), pathname);
  };
  return fd;
  #else
  easy_fuck("This function only works in Linux.");
  return -1;
  #endif
};

//--page-split-- easy__file_close

int easy__file_close(char *file, int line, int fd, int allow_errors) {
  #ifdef LINUX
  int result = close(fd);
  if (!allow_errors && result < 0) easy__fuck(file, line, 1, "%s while closing file descriptor %d\n", easy_error_string(errno), fd);
  return result;
  #else
  easy_fuck("This function only works in Linux.");
  return -1;
  #endif
};

//--page-split-- easy__file_read

int easy__file_read(char *file, int line, int fd, void *buffer, ssize_t count, int allow_errors) {
  #ifdef LINUX
  ssize_t result;
  if (allow_errors <= 0) {
    ssize_t bytes_read = 0;
    while (bytes_read < count) {
      result = read(fd, buffer + bytes_read, (size_t) (count - bytes_read));
      if (result < 0) easy__fuck(file, line, 1, "%s while easy_file_read(%d, %p, %ld)\n", easy_error_string(errno), fd, buffer, count);
      if (result == 0 && allow_errors == 0) break;
      if (result == 0) easy__fuck(file, line, 1, "Unexpected end of file while reading file descriptor %d\n", fd);
      bytes_read += result;
    };
    result = bytes_read;
  } else {
    result = read(fd, buffer, (size_t) count);
  };
  return result;
  #else
  easy_fuck("This function only works in Linux.");
  return -1;
  #endif
};

//--page-split-- easy__file_write

int easy__file_write(char *file, int line, int fd, void *buffer, ssize_t count, int allow_errors) {
  #ifdef LINUX
  ssize_t result;
  if (allow_errors <= 0) {
    ssize_t bytes_written = 0;
    while (bytes_written < count) {
      result = write(fd, buffer + bytes_written, (size_t) (count - bytes_written));
      if (result < 0) easy__fuck(file, line, 1, "%s while easy_file_write(%d, %p, %ld)\n", easy_error_string(errno), fd, buffer, count);
      if (result == 0 && result < count - bytes_written) easy__fuck(file, line, 1, "Wrote zero bytes while writing file descriptor %d\n", fd);
      bytes_written += result;
    };
    result = bytes_written;
  } else {
    result = write(fd, buffer, (size_t) count);
  };
  return result;
  #else
  easy_fuck("This function only works in Linux.");
  return -1;
  #endif
};

//--page-split-- easy_sort_floats

void easy_sort_floats(float *item, int count) {
  if (count <= 1) return;
  int half_one = count >> 1;
  int half_two = count - half_one;
  easy_sort_floats(item, half_one);
  easy_sort_floats(item + half_one, half_two);
  float *temp = NULL;
  easy_memory_allocate(&temp, count * (int) sizeof(float));
  float *pointer_one = item;
  float *pointer_two = item + half_one;
  for (int i = 0; i < count; i++) {
    if (half_one && half_two) {
      if (*pointer_one < *pointer_two) {
        temp[i] = *pointer_one;
        pointer_one++;
        half_one--;
      } else {
        temp[i] = *pointer_two;
        pointer_two++;
        half_two--;
      };
    } else if (half_one) {
      temp[i] = *pointer_one;
      pointer_one++;
      half_one--;
    } else {
      temp[i] = *pointer_two;
      pointer_two++;
      half_two--;
    };
  };
  memmove(item, temp, count * (int) sizeof(float));
  easy_memory_allocate(&temp, 0);
};

//--page-split-- easy_sort_by_float

void easy_sort_by_float(void *item, int size, int count) {
  if (count <= 1) return;
  int half_one = count >> 1;
  int half_two = count - half_one;
  easy_sort_by_float(item, size, half_one);
  easy_sort_by_float(item + half_one * size, size, half_two);
  void *temp = NULL;
  easy_memory_allocate(&temp, count * size);
  void *pointer_one = item;
  void *pointer_two = item + half_one * size;
  for (int i = 0; i < count; i++) {
    if (half_one && half_two) {
      if (*((float *) pointer_one) < *((float *) pointer_two)) {
        memmove(temp + i * size, pointer_one, size);
        pointer_one += size;
        half_one--;
      } else {
        memmove(temp + i * size, pointer_two, size);
        pointer_two += size;
        half_two--;
      };
    } else if (half_one) {
      memmove(temp + i * size, pointer_one, size);
      pointer_one += size;
      half_one--;
    } else {
      memmove(temp + i * size, pointer_two, size);
      pointer_two += size;
      half_two--;
    };
  };
  memmove(item, temp, count * size);
  easy_memory_allocate(&temp, 0);
};

//--page-split-- easy_sort_structures

void easy_sort_structures(void *item, int size, int count, int type, int offset, int reverse) {
  if (count <= 1) return;
  int half_one = count >> 1;
  int half_two = count - half_one;
  if (half_one > 1) easy_sort_structures(item, size, half_one, type, offset, reverse);
  if (half_two > 1) easy_sort_structures(item + half_one * size, size, half_two, type, offset, reverse);
  void *temp = NULL;
  easy_memory_allocate(&temp, count * size);
  void *pointer_one = item;
  void *pointer_two = item + half_one * size;
  for (int i = 0; i < count; i++) {
    if (half_one && half_two) {
      int result = 0;
      switch (type) {
        case EASY_TYPE_CHAR:
          if (reverse) {
            result = *((char *) (pointer_two + offset)) <= *((char *) (pointer_one + offset));
          } else {
            result = *((char *) (pointer_one + offset)) <= *((char *) (pointer_two + offset));
          };
        break;
        case EASY_TYPE_SHORT:
          if (reverse) {
            result = *((short *) (pointer_two + offset)) <= *((short *) (pointer_one + offset));
          } else {
            result = *((short *) (pointer_one + offset)) <= *((short *) (pointer_two + offset));
          };
        break;
        case EASY_TYPE_INT:
          if (reverse) {
            result = *((int *) (pointer_two + offset)) <= *((int *) (pointer_one + offset));
          } else {
            result = *((int *) (pointer_one + offset)) <= *((int *) (pointer_two + offset));
          };
        break;
        case EASY_TYPE_LONG:
          if (reverse) {
            result = *((long *) (pointer_two + offset)) <= *((long *) (pointer_one + offset));
          } else {
            result = *((long *) (pointer_one + offset)) <= *((long *) (pointer_two + offset));
          };
        break;
        case EASY_TYPE_INT64:
          if (reverse) {
            result = *((int64_t *) (pointer_two + offset)) <= *((int64_t *) (pointer_one + offset));
          } else {
            result = *((int64_t *) (pointer_one + offset)) <= *((int64_t *) (pointer_two + offset));
          };
        break;
        case EASY_TYPE_FLOAT:
          if (reverse) {
            result = *((float *) (pointer_two + offset)) <= *((float *) (pointer_one + offset));
          } else {
            result = *((float *) (pointer_one + offset)) <= *((float *) (pointer_two + offset));
          };
        break;
        case EASY_TYPE_DOUBLE:
          if (reverse) {
            result = *((double *) (pointer_two + offset)) <= *((double *) (pointer_one + offset));
          } else {
            result = *((double *) (pointer_one + offset)) <= *((double *) (pointer_two + offset));
          };
        break;
        case EASY_TYPE_STRING:
          if (reverse) {
            result = strcmp(*((char **) (pointer_two + offset)), *((char **) (pointer_one + offset))) <= 0;
          } else {
            result = strcmp(*((char **) (pointer_one + offset)), *((char **) (pointer_two + offset))) <= 0;
          };
        break;
        case EASY_TYPE_EASY_STRING:;
          int length_one = ((struct easy_string *) (pointer_one + offset))->length;
          int length_two = ((struct easy_string *) (pointer_two + offset))->length;
          int length = length_one; if (length > length_two) length = length_two;
          if (reverse) {
            result = memcmp(((struct easy_string *) (pointer_two + offset))->buffer, ((struct easy_string *) (pointer_one + offset))->buffer, length);
            if (result == 0) result = -(length_two <= length_one);
            result = result <= 0;
          } else {
            result = memcmp(((struct easy_string *) (pointer_one + offset))->buffer, ((struct easy_string *) (pointer_two + offset))->buffer, length);
            if (result == 0) result = -(length_one <= length_two);
            result = result <= 0;
            //printf("sort result=%d for \"%s\" vs. \"%s\"\n", result, ((struct easy_string *) (pointer_one + offset))->buffer, ((struct easy_string *) (pointer_two + offset))->buffer);
          };
        break;
        default: easy_fuck("Unknown data type.\n");
      };
      if (result) {
        memmove(temp + i * size, pointer_one, size);
        pointer_one += size;
        half_one--;
      } else {
        memmove(temp + i * size, pointer_two, size);
        pointer_two += size;
        half_two--;
      };
    } else if (half_one) {
      memmove(temp + i * size, pointer_one, size);
      pointer_one += size;
      half_one--;
    } else {
      memmove(temp + i * size, pointer_two, size);
      pointer_two += size;
      half_two--;
    };
  };
  memmove(item, temp, count * size);
  easy_memory_allocate(&temp, 0);
};

//--page-split-- md5_process

static void md5_process(void *_digest, void *_message) {

  int * const digest = (int *) _digest;
  int * const message = (int *) _message;

  unsigned int a, b, c, d;

  a = digest[0];
  b = digest[1];
  c = digest[2];
  d = digest[3];

  a += ((b & c) | (~b & d)) + message[0] + 3614090360; a = b + ((a << 7) | (a >> 25));
  d += ((a & b) | (~a & c)) + message[1] + 3905402710; d = a + ((d << 12) | (d >> 20));
  c += ((d & a) | (~d & b)) + message[2] + 606105819; c = d + ((c << 17) | (c >> 15));
  b += ((c & d) | (~c & a)) + message[3] + 3250441966; b = c + ((b << 22) | (b >> 10));
  a += ((b & c) | (~b & d)) + message[4] + 4118548399; a = b + ((a << 7) | (a >> 25));
  d += ((a & b) | (~a & c)) + message[5] + 1200080426; d = a + ((d << 12) | (d >> 20));
  c += ((d & a) | (~d & b)) + message[6] + 2821735955; c = d + ((c << 17) | (c >> 15));
  b += ((c & d) | (~c & a)) + message[7] + 4249261313; b = c + ((b << 22) | (b >> 10));
  a += ((b & c) | (~b & d)) + message[8] + 1770035416; a = b + ((a << 7) | (a >> 25));
  d += ((a & b) | (~a & c)) + message[9] + 2336552879; d = a + ((d << 12) | (d >> 20));
  c += ((d & a) | (~d & b)) + message[10] + 4294925233; c = d + ((c << 17) | (c >> 15));
  b += ((c & d) | (~c & a)) + message[11] + 2304563134; b = c + ((b << 22) | (b >> 10));
  a += ((b & c) | (~b & d)) + message[12] + 1804603682; a = b + ((a << 7) | (a >> 25));
  d += ((a & b) | (~a & c)) + message[13] + 4254626195; d = a + ((d << 12) | (d >> 20));
  c += ((d & a) | (~d & b)) + message[14] + 2792965006; c = d + ((c << 17) | (c >> 15));
  b += ((c & d) | (~c & a)) + message[15] + 1236535329; b = c + ((b << 22) | (b >> 10));
  a += ((b & d) | (c & ~d)) + message[1] + 4129170786; a = b + ((a << 5) | (a >> 27));
  d += ((a & c) | (b & ~c)) + message[6] + 3225465664; d = a + ((d << 9) | (d >> 23));
  c += ((d & b) | (a & ~b)) + message[11] + 643717713; c = d + ((c << 14) | (c >> 18));
  b += ((c & a) | (d & ~a)) + message[0] + 3921069994; b = c + ((b << 20) | (b >> 12));
  a += ((b & d) | (c & ~d)) + message[5] + 3593408605; a = b + ((a << 5) | (a >> 27));
  d += ((a & c) | (b & ~c)) + message[10] + 38016083; d = a + ((d << 9) | (d >> 23));
  c += ((d & b) | (a & ~b)) + message[15] + 3634488961; c = d + ((c << 14) | (c >> 18));
  b += ((c & a) | (d & ~a)) + message[4] + 3889429448; b = c + ((b << 20) | (b >> 12));
  a += ((b & d) | (c & ~d)) + message[9] + 568446438; a = b + ((a << 5) | (a >> 27));
  d += ((a & c) | (b & ~c)) + message[14] + 3275163606; d = a + ((d << 9) | (d >> 23));
  c += ((d & b) | (a & ~b)) + message[3] + 4107603335; c = d + ((c << 14) | (c >> 18));
  b += ((c & a) | (d & ~a)) + message[8] + 1163531501; b = c + ((b << 20) | (b >> 12));
  a += ((b & d) | (c & ~d)) + message[13] + 2850285829; a = b + ((a << 5) | (a >> 27));
  d += ((a & c) | (b & ~c)) + message[2] + 4243563512; d = a + ((d << 9) | (d >> 23));
  c += ((d & b) | (a & ~b)) + message[7] + 1735328473; c = d + ((c << 14) | (c >> 18));
  b += ((c & a) | (d & ~a)) + message[12] + 2368359562; b = c + ((b << 20) | (b >> 12));
  a += (b ^ c ^ d) + message[5] + 4294588738; a = b + ((a << 4) | (a >> 28));
  d += (a ^ b ^ c) + message[8] + 2272392833; d = a + ((d << 11) | (d >> 21));
  c += (d ^ a ^ b) + message[11] + 1839030562; c = d + ((c << 16) | (c >> 16));
  b += (c ^ d ^ a) + message[14] + 4259657740; b = c + ((b << 23) | (b >> 9));
  a += (b ^ c ^ d) + message[1] + 2763975236; a = b + ((a << 4) | (a >> 28));
  d += (a ^ b ^ c) + message[4] + 1272893353; d = a + ((d << 11) | (d >> 21));
  c += (d ^ a ^ b) + message[7] + 4139469664; c = d + ((c << 16) | (c >> 16));
  b += (c ^ d ^ a) + message[10] + 3200236656; b = c + ((b << 23) | (b >> 9));
  a += (b ^ c ^ d) + message[13] + 681279174; a = b + ((a << 4) | (a >> 28));
  d += (a ^ b ^ c) + message[0] + 3936430074; d = a + ((d << 11) | (d >> 21));
  c += (d ^ a ^ b) + message[3] + 3572445317; c = d + ((c << 16) | (c >> 16));
  b += (c ^ d ^ a) + message[6] + 76029189; b = c + ((b << 23) | (b >> 9));
  a += (b ^ c ^ d) + message[9] + 3654602809; a = b + ((a << 4) | (a >> 28));
  d += (a ^ b ^ c) + message[12] + 3873151461; d = a + ((d << 11) | (d >> 21));
  c += (d ^ a ^ b) + message[15] + 530742520; c = d + ((c << 16) | (c >> 16));
  b += (c ^ d ^ a) + message[2] + 3299628645; b = c + ((b << 23) | (b >> 9));
  a += (c ^ (b | ~d)) + message[0] + 4096336452; a = b + ((a << 6) | (a >> 26));
  d += (b ^ (a | ~c)) + message[7] + 1126891415; d = a + ((d << 10) | (d >> 22));
  c += (a ^ (d | ~b)) + message[14] + 2878612391; c = d + ((c << 15) | (c >> 17));
  b += (d ^ (c | ~a)) + message[5] + 4237533241; b = c + ((b << 21) | (b >> 11));
  a += (c ^ (b | ~d)) + message[12] + 1700485571; a = b + ((a << 6) | (a >> 26));
  d += (b ^ (a | ~c)) + message[3] + 2399980690; d = a + ((d << 10) | (d >> 22));
  c += (a ^ (d | ~b)) + message[10] + 4293915773; c = d + ((c << 15) | (c >> 17));
  b += (d ^ (c | ~a)) + message[1] + 2240044497; b = c + ((b << 21) | (b >> 11));
  a += (c ^ (b | ~d)) + message[8] + 1873313359; a = b + ((a << 6) | (a >> 26));
  d += (b ^ (a | ~c)) + message[15] + 4264355552; d = a + ((d << 10) | (d >> 22));
  c += (a ^ (d | ~b)) + message[6] + 2734768916; c = d + ((c << 15) | (c >> 17));
  b += (d ^ (c | ~a)) + message[13] + 1309151649; b = c + ((b << 21) | (b >> 11));
  a += (c ^ (b | ~d)) + message[4] + 4149444226; a = b + ((a << 6) | (a >> 26));
  d += (b ^ (a | ~c)) + message[11] + 3174756917; d = a + ((d << 10) | (d >> 22));
  c += (a ^ (d | ~b)) + message[2] + 718787259; c = d + ((c << 15) | (c >> 17));
  b += (d ^ (c | ~a)) + message[9] + 3951481745; b = c + ((b << 21) | (b >> 11));

  digest[0] += a;
  digest[1] += b;
  digest[2] += c;
  digest[3] += d;

};

//--page-split-- easy_md5

void easy_md5(void *_digest, void *_message, unsigned int length) {

  // Buffer to store last two blocks, since they require
  // modification to add the padding and message length.

  unsigned int _buffer[32];

  // Cast everything to pointers of various types, in order
  // to avoid having to repeatedly type cast in the code below.
  // Placing the "const" after the "*" seems to be the way to
  // tell the compiler that it is the pointer, and not the
  // data it points to, which is constant.  Thus it will know
  // that we're simply defining another name for the pointer.

  unsigned char * const digest_chars = (unsigned char *) _digest;
  unsigned int * const digest_ints = (unsigned int *) _digest;
  unsigned char * const message_chars = (unsigned char *) _message;
  unsigned int * const message_ints = (unsigned int *) _message;
  unsigned char * const buffer_chars = (unsigned char *) _buffer;
  unsigned int * const buffer_ints = (unsigned int *) _buffer;

  // To pad the message, copy its last partial block (if any) to a
  // zero-filled buffer, then add the padding and length to the buffer.

  int source_block_count = length >> 6;
  int buffer_block_count = (((length % 64) + 8) >> 6) + 1;

  memset(buffer_chars, 0, 128);
  memmove(buffer_chars, message_chars + 64 * source_block_count, length % 64);
  buffer_chars[length % 64] = 0x80;

  // Then append the length, in bits.

  buffer_ints[16 * buffer_block_count - 2] = length << 3;
  buffer_ints[16 * buffer_block_count - 1] = length >> 29;

  // Then just compute the digest!

  digest_ints[0] = 0x67452301;
  digest_ints[1] = 0xEFCDAB89;
  digest_ints[2] = 0x98BADCFE;
  digest_ints[3] = 0x10325476;

  for (int i = 0; i < source_block_count; i++) {
    md5_process(digest_chars, message_chars + 64 * i);
  };

  for (int i = 0; i < buffer_block_count; i++) {
    md5_process(digest_chars, buffer_chars + 64 * i);
  };

  // That was so easy!

};
