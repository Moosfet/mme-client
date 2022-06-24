#include "everything.h"

#ifdef WINDOWS

#include <dsound.h>

static void *directsound_output_thread(void *argument) ;

char *directsound_error_function = NULL;
char *directsound_error_message = NULL;

static unsigned int write_offset = 0;

// The following structures are normally typedef'd.  Unfortunately, cproto is a pissy piece of shit
// and gets all bent out of shape when you use global typedefs, even if they're static.
// cproto can eat a dick.

static struct IDirectSound *direct_sound;
static struct IDirectSoundBuffer *sound_buffer;
static WAVEFORMATEX wave_format;
static unsigned long buffer_size = 4096;
static unsigned long period_size = 0;  // set to buffer_size / 4 in code below...

#endif

//--page-split-- directsound_error

#ifdef WINDOWS

static void directsound_error (char *function, const char *message) {
  fprintf(stderr, "DirectSound Error: %s: %s\n", function, message);
  directsound_error_function = function;
  int length = strlen(message);
  memory_allocate(&directsound_error_message, length + 1);
  strcpy(directsound_error_message, message);
};

static volatile int thread_run_flag = 0;    // indicates thread was spawned
static volatile int thread_exit_flag = 0;   // used to tell thread to exit

#endif

//--page-split-- directsound_strerror

#ifdef WINDOWS

static char * directsound_strerror(int ret) {
  char *error_pointer = "undefined windows error";
//(static const char *)
  if (ret == DSERR_ALLOCATED) error_pointer = "DSERR_ALLOCATED";
  else if (ret == DSERR_INVALIDPARAM) error_pointer = "DSERR_INVALIDPARAM";
  else if (ret == DSERR_NOAGGREGATION) error_pointer = "DSERR_NOAGGREGATION";
  else if (ret == DSERR_NODRIVER) error_pointer = "DSERR_NODRIVER";
  else if (ret == DSERR_OUTOFMEMORY) error_pointer = "DSERR_OUTOFMEMORY";
  else if (ret == DSERR_UNINITIALIZED) error_pointer = "DSERR_UNINITIALIZED";
  else if (ret == DSERR_UNSUPPORTED) error_pointer = "DSERR_UNSUPPORTED";
  else if (ret == DSERR_BADFORMAT) error_pointer = "DSERR_BADFORMAT";
//  else if (ret == DSERR_BUFFERTOOSMALL) error_pointer = "DSERR_BUFFERTOOSMALL";
  else if (ret == DSERR_CONTROLUNAVAIL) error_pointer = "DSERR_CONTROLUNAVAIL";
//  else if (ret == DSERR_DS8_REQUIRED) error_pointer = "DSERR_DS8_REQUIRED";
  else if (ret == DSERR_INVALIDCALL) error_pointer = "DSERR_INVALIDCALL";
  else if (ret == DSERR_BUFFERLOST) error_pointer = "DSERR_BUFFERLOST";
  else if (ret == DSERR_PRIOLEVELNEEDED) error_pointer = "DSERR_PRIOLEVELNEEDED";
  else {
    static char suck[128];
    sprintf (suck, "Unknown error %i", ret);
    error_pointer = suck;
  }

  return (error_pointer);
}

#endif

//--page-split-- directsound_initialize

#ifdef WINDOWS

void directsound_initialize() {

  printf("DirectSound buffer size is %d\n", buffer_size);
  period_size = buffer_size / 4;

  printf ("directsound init\n");
  write_offset = 0;
  wave_format.wFormatTag = WAVE_FORMAT_PCM;
  wave_format.nChannels = 2;
  wave_format.nSamplesPerSec = 44100;
  wave_format.wBitsPerSample = 16;
  wave_format.nBlockAlign = wave_format.nChannels * wave_format.wBitsPerSample / 8;
  wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
  wave_format.cbSize = sizeof (wave_format);

 // buffer_size = wave_format.nAvgBytesPerSec * 2;

  directsound_error_function = NULL;

  CoInitialize( NULL );
  HRESULT ret = DirectSoundCreate(NULL, &direct_sound, NULL);
  if (ret != DS_OK) {
    directsound_error("DirectSoundCreate()", directsound_strerror(ret));
    return;

  }

  // GetDesktopWindow() should be replaced with our window handle, but I couldn't find a way
  // to get GLFW to tell me what it is, so this is what you get
  ret = direct_sound->lpVtbl->SetCooperativeLevel(direct_sound, GetDesktopWindow(), DSSCL_NORMAL );
  if (ret != DS_OK) {
    directsound_error("SetCooperativeLevel()", directsound_strerror(ret));
    return;
  }

  // Removed DSBCAPS_GETCURRENTPOSITION2 | DSCBCAPS_WAVEMAPPED  for XP compatability.

  DSBUFFERDESC buffer_description;
  buffer_description.dwSize = sizeof(DSBUFFERDESC);
  buffer_description.dwFlags = DSBCAPS_GLOBALFOCUS; // | DSBCAPS_CTRLPAN;
  buffer_description.dwBufferBytes = 4 * buffer_size;
  buffer_description.lpwfxFormat = &wave_format;
  buffer_description.dwReserved = 0;

  if ((ret = direct_sound->lpVtbl->CreateSoundBuffer(direct_sound, &buffer_description, &sound_buffer, 0)) != DS_OK) {
      directsound_error("CreateSoundBuffer()", directsound_strerror(ret));
      return;
  }

  if ((ret = sound_buffer->lpVtbl->GetFormat(sound_buffer, &wave_format, sizeof( WAVEFORMATEX ), NULL)) != DS_OK) {
      directsound_error("GetFormat()", directsound_strerror(ret));
      return;
  }

  printf ("format: %d\nchannels: %d\nsamples per sec: %i\navg bytes/sec: %i\nblock align: %i\nbits per sample: %i\ncbsize: %i\n"
            , wave_format.wFormatTag, wave_format.nChannels, wave_format.nSamplesPerSec, wave_format.nAvgBytesPerSec, wave_format.nBlockAlign,
            wave_format.wBitsPerSample, wave_format.cbSize);

  if ((ret = sound_buffer->lpVtbl->Play(sound_buffer, 0, 0, DSBPLAY_LOOPING )) != DS_OK) {
      directsound_error("Play()", directsound_strerror(ret));
      return;
  }

  thread_run_flag = 1;
  thread_exit_flag = 0;
  thread_create(directsound_output_thread, NULL);
};

#endif

//--page-split-- directsound_terminate

#ifdef WINDOWS

void directsound_terminate() {

  if (thread_run_flag) {
    thread_exit_flag = 1;
    while (thread_run_flag) easy_sleep(0.001);
  };

  sound_buffer->lpVtbl->Stop(sound_buffer);
};

#endif

//--page-split-- directsound_output_thread

#ifdef WINDOWS

static void *directsound_output_thread(void *argument) {

  float *float_buffer = NULL;
  short *short_buffer = NULL;

  memory_allocate(&float_buffer, 2 * buffer_size * sizeof(float));
  memory_allocate(&short_buffer, 2 * buffer_size * sizeof(short));

  DWORD last_write_position = 0;

  while (!thread_exit_flag) {
    PBYTE locked_buffer = NULL;
    DWORD locked_buffer_size = 0;
    PBYTE locked_buffer2 = NULL;
    DWORD locked_buffer_size2 = 0;
    int ret = 0;

    DWORD play_cursor = 12345;
    DWORD write_cursor = 67890;
    int available;

    ret = sound_buffer->lpVtbl->GetCurrentPosition(sound_buffer, &play_cursor, &write_cursor);
    if (ret != DS_OK) {
      directsound_error("GetCurrentPosition()", directsound_strerror(ret));
      //goto ABORT;
    };

    //available = ((int) play_cursor - (int) write_cursor);
    //available = ((int) write_cursor - (int) play_cursor);
    //available = ((int) write_cursor - (int) last_write_position);
    available = ((int) play_cursor - (int) last_write_position);
    if (available < 0) available += 4 * buffer_size;

    //printf("There are %d bytes available...  (play = %d, write = %d, last_write_position = %d)\n", available, play_cursor, write_cursor, last_write_position);

    if (available / 4 < 2 * period_size) {
      //printf("sleeping until more space is available\n");
      easy_sleep(0.001);
    } else {

      int bytes_to_write = available;

      if (bytes_to_write > period_size * 4) bytes_to_write = period_size * 4;

      //printf("writing %d bytes\n", bytes_to_write);

      retry_lock:;
      if ((ret = sound_buffer->lpVtbl->Lock (sound_buffer, 0, bytes_to_write, (LPVOID *)&locked_buffer,
           &locked_buffer_size, (LPVOID *) &locked_buffer2, &locked_buffer_size2, DSBLOCK_ENTIREBUFFER)) != DS_OK) { // DSBLOCK_FROMWRITECURSOR
        if (ret == DSERR_BUFFERLOST) {
          sound_buffer->lpVtbl->Restore(sound_buffer);
          goto retry_lock;
        } else {
          directsound_error("Lock()", directsound_strerror(ret));
          goto ABORT;
        }
      }

      //printf("%p, %d, %p, %d\n", locked_buffer, locked_buffer_size, locked_buffer2, locked_buffer_size2);

      // generate some audio...

      mixer_generate(float_buffer, bytes_to_write >> 2);

      // convert it from float to short...

      for (int i = 0; i < bytes_to_write >> 1; i++) {
        int t = nearbyint(32768 * float_buffer[i]);
        if (t < -32768) t = -32768;
        if (t > +32767) t = +32767;
        short_buffer[i] = t;
      };

      for (int i = 0; i < bytes_to_write >> 1; i++) {
        *((short *) ((char *) locked_buffer + last_write_position)) = short_buffer[i];
        last_write_position += 2;
        if (last_write_position >= 4 * buffer_size) last_write_position -= 4 * buffer_size;
      };

      sound_buffer->lpVtbl->Unlock(sound_buffer, locked_buffer, locked_buffer_size, locked_buffer2, locked_buffer_size2 );

      //last_write_position += bytes_to_write;
      //if (last_write_position >= 4 * buffer_size) last_write_position -= 4 * buffer_size;

      static int total_bytes = 0;
      total_bytes += bytes_to_write;
      //printf("Wrote %d bytes so far... (%f seconds of data)\n", total_bytes, (float) total_bytes / 4 / wave_format.nSamplesPerSec);

    };

  };

  ABORT:;

  memory_allocate(&float_buffer, 0);
  memory_allocate(&short_buffer, 0);

  thread_run_flag = 0;

  thread_exit();

};

#endif
