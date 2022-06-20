#include "everything.h"

/*

Pthreads actually has a rwlock, but here I'll do both Windows and Linux the
same way, so that my mutex code gets more testing since I rarely use it in
Windows.

Intended behavior:

Allow one writer at a time, and as long as there are no writers, allow
multiple readers at once.  When a new writer wants access, block access to
new readers, so that readers don't tie up the lock forever, since otherwise
they would have preferential access due to being able to share the lock.

The basic plan: (assume ++, --, and subsequent comparisons will be atomic)

lock_read
lock(change);
if (count++ == 0) lock(in_use);
unlock(change);

unlock_read
  if (--count == 0) unlock(in_use);

lock_write
  lock(change);
  lock(in_use);
  unlock(change);

unlock_write
  unlock(in_use);

Notes:

The CPU has the LOCK instruction, allowing for atomic interactions with memory;
essentially the only reason the OS needs to be involved is for scheduling
purposes, so that we can sleep until the resource becomes available.  As such,
the number of required mutexes is equal to the number of different events
that we need to wait for, and the amount of data involved is irrelevant.

We need to wait in these cases:

1.  We want to write, but someone else is reading or writing.

2.  We want to read, but someone else is waiting to write.

Thus, two mutexes are required.

However, that doesn't explain how the above pseudocode solves the problem with
the two mutexes it uses.  Instead, it looks at it like this:

1.  Reading threads (as a group) vs. writing threads (individually), all need
    to wait until none of the others are using the lock.  This function is
    performed by the "in_use" mutex.

2.  Reading threads (as a group) need to know when one (and only one) of the
    other reading threads has acquired the "in_use" mutex, and if none have,
    need to determine which thread will do so.  The "count" variable combined
    with the "change" mutex accomplish this.

The "change" mutex determines who is next to determine the state of the lock,
essentially forcing all readers and writers to wait in a single line, so as
to avoid the problem of writers never getting to write because a new reader
always comes along before the previous reader finishes.

The "in_use" mutex tells whoever wins the "change" mutex that the previous
state of the lock is no longer being used.

The "count" variable is where things become confusing.  Essentially it becomes
non-zero when the lock is in read mode, but specifically, the lock is only in
read mode when "count" is non-zero when read from within the "change" mutex.
One read thread may increment "count" from zero, thus determining that it
should acquire the "in_use" mutex, but then block while aquiring the "in_use"
mutex because either a writing thread is in possession of it, or a reading
thread is in possession, and "count" was only zero because that reading thread
had just decremented it to zero, but hasn't yet released the "in_use" mutex.
In these cases, the "change" mutex prevents another reading thread from
seeing the non-zero "count" and assuming the "in_use" mutex is acquired and
that it may read.  Possession of the "change" mutex also guarantees that this
thread will be next to receive the "in_use" mutex, and so the non-zero state
of the "count" is not in error since, even though the lock is not currently
in read mode, it will be before anyone can actually read anything.

The "change" mutex is essentually a mutex that determines who is allowed to
acquire the "in_use" mutex.  That seems a bit silly since the point of a mutex
is to allow the OS to choose who is next to acquire it, but it is necessary
since the many reading threads need to cooperate in aquiring it.  Thus they
need a mutex to determine who will acquire the mutex, but it must be a
different mutex because they need to release the first mutex so that
subsequent reading threads can acquire it, determine they aren't the thread
that acquires the second mutex, then release it for the next reading thread.

Thus, the "in_use" mutex is held by either *all* reading threads or *a*
writing thread, and so is held throughout the duration of a read/write
operation, whereas the "change" mutex coordinates the multiple reading
threads with eachother, and so is released immediately after either the
"in_use" mutex is acquired, or it is determined that it has already been
acquired.  The "change" mutex is the also used by the writing threads to
halt the cooperative nature of the reading threads, so that they release
the "in_use" mutex rather than hold it until no reading threads remain.

Multithreading is really complicated when you think about it.

*/

// Since ++ and -- are not atomic operations, we use these functions instead.
// acquire() takes the place of ++, and release() takes the place of --, but
// unlike ++ and --, acquire actually decrements the integer, while release()
// increments it.  This is because the carry flag indicates when a number
// becomes negative, which can tell us when we've decremented it below zero,
// then the zero flag can tell us when when increment it back to zero.  This
// is important since, not only must the increment/decrement of the integer
// be atomic, but so must our examiniation of its value be atomic with the
// increment/decrement operation.

//--page-split-- acquire

static int acquire(int *i) {
  // Uses integer *i to determine if you were the first to acquire a resource.
  // Returns true if you caused *i to become non-zero, false otherwise.
  char result;
  __asm__ __volatile__ (
    "  lock;\n"
    "  subl $1, %0;\n"
    "  setc %1;\n"
    : "=m" (*i), "=r" (result)
    : "m" (*i)
  );
  return result;
};

//--page-split-- release

static int release(int *i) {
  // Uses integer *i to determine if you were the last to release a resource.
  // Returns true if you caused *i to become zero, false otherwise.
  char result;
  __asm__ __volatile__ (
    "  lock;\n"
    "  addl $1, %0;\n"
    "  setz %1;\n"
    : "=m" (*i), "=r" (result)
    : "m" (*i)
  );
  return result;
};

//--page-split-- thread_lock_init

void thread_lock_init(struct structure_rwlock *rwlock) {
  MUTEX_INIT(rwlock->change);
  MUTEX_INIT(rwlock->in_use);
  rwlock->count = 0;
};

//--page-split-- thread_lock_read

void thread_lock_read(struct structure_rwlock *rwlock) {
  //DEBUG("Waiting for read lock, step one...");
  MUTEX_LOCK(rwlock->change);
  if (acquire(&rwlock->count)) {
    //DEBUG("Waiting for read lock, step two...");
    MUTEX_LOCK(rwlock->in_use);
  };
  MUTEX_UNLOCK(rwlock->change);
  //DEBUG("Read lock acquired!");
};

//--page-split-- thread_unlock_read

void thread_unlock_read(struct structure_rwlock *rwlock) {
  if (release(&rwlock->count)) {
    //DEBUG("Releasing read lock!");
    MUTEX_UNLOCK(rwlock->in_use);
  };
};

//--page-split-- thread_lock_write

void thread_lock_write(struct structure_rwlock *rwlock) {
  //DEBUG("Waiting for write lock, step 1...");
  MUTEX_LOCK(rwlock->change);
  //DEBUG("Waiting for write lock, step 2...");
  MUTEX_LOCK(rwlock->in_use);
  MUTEX_UNLOCK(rwlock->change);
  //DEBUG("Write lock acquired!");
};

//--page-split-- thread_unlock_write

void thread_unlock_write(struct structure_rwlock *rwlock) {
  //DEBUG("Releasing write lock!");
  MUTEX_UNLOCK(rwlock->in_use);
};

// Thread creation code!

//--page-split-- thread_priority_background

void thread_priority_background() {
  #ifdef WINDOWS
  if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST)) easy_fuck("Failed to set thread priority.");
  #else
  //if (pthread_setschedpolicy(pthread_self(), SCHED_OTHER)) easy_fuck("Failed to set thread scheduling policy.");
  struct sched_param my_param;
  my_param.sched_priority = sched_get_priority_min(SCHED_OTHER);
  if (pthread_setschedparam(pthread_self(), SCHED_OTHER, &my_param)) easy_fuck("Failed to set thread scheduling priority.");
  #endif
};

//--page-split-- thread_create

void thread_create(void (*function(void *)), void *parameter) {
  #ifdef WINDOWS
  if (NULL == CreateThread(NULL, 0, (void *) function, parameter, 0, NULL)) easy_fuck("Failed to create thread.");
  #else
  pthread_t whatever;
  pthread_attr_t ta;
  pthread_attr_init(&ta);
  pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);
  if (pthread_create(&whatever, &ta, function, parameter)) easy_fuck("Failed to create thread.");
  pthread_attr_destroy(&ta);
  #endif
};

//--page-split-- thread_exit

void thread_exit() {
  #ifdef WINDOWS
  ExitThread(0);
  #else
  pthread_exit(0);
  #endif
};
