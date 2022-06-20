#include "everything.h"

#ifdef LINUX
char *alsa_error_function = NULL;
char *alsa_error_message = NULL;

static char *pcm_output_device = "default";
static snd_pcm_format_t format = SND_PCM_FORMAT_S16;
static unsigned int sample_rate = 44100;
static snd_pcm_uframes_t buffer_size = 1024;
static snd_pcm_uframes_t period_size = 256;
static snd_output_t *output = NULL;
static snd_pcm_t *pcm_handle;

#endif

//--page-split-- alsa_error

#ifdef LINUX

static void alsa_error (char *function, const char *message) {
  fprintf(stderr, "ALSA Error: %s: %s\n", function, message);
  alsa_error_function = function;
  int length = strlen(message);
  memory_allocate(&alsa_error_message, length + 1);
  strcpy(alsa_error_message, message);
};

static volatile int thread_run_flag = 0;    // indicates thread was spawned
static volatile int thread_exit_flag = 0;   // used to tell thread to exit

#endif

//--page-split-- pcm_output_thread

#ifdef LINUX

static void *pcm_output_thread(void *argument) {

  int underrun_count = 0;

  int result;

  float *float_buffer = NULL;
  short *short_buffer = NULL;

  memory_allocate(&float_buffer, 2 * period_size * sizeof(float));
  memory_allocate(&short_buffer, 2 * period_size * sizeof(short));

  while (!thread_exit_flag) {

    // generate some audio...

    mixer_generate(float_buffer, period_size);

    // convert it from float to short...

    for (int i = 0; i < 2 * period_size; i++) {
      short t = nearbyint(32768 * float_buffer[i]);
      if (t < -32768) t = -32768;
      if (t > +32767) t = +32767;
      short_buffer[i] = t;
    };

    // give it to alsa...

    short *pointer = short_buffer;
    int count = period_size;

    while (count > 0) {
      //printf("snd_pcm_writei(%p, %p, %d);\n", pcm_handle, pointer, count);
      result = snd_pcm_writei(pcm_handle, pointer, count);
      if (result == -EAGAIN) continue;
      if (result < 0) {
        //fprintf(stderr, "ALSA Error: A buffer under-run has occurred.\n");
        if (++underrun_count >= 10) {
          alsa_error("snd_pcm_writei()", "Too many underruns are occurring.");
          goto ABORT;
        };
        if (result == -EPIPE) {
          result = snd_pcm_prepare(pcm_handle);
          if (result < 0) {
            alsa_error("snd_pcm_close()", snd_strerror(result));
            goto ABORT;
          };
        } else if (result == -ESTRPIPE) {
          do {
            easy_sleep(0.01);
            result = snd_pcm_resume(pcm_handle);
          } while (result == -EAGAIN);
          if (result < 0) {
            alsa_error("snd_pcm_resume()", snd_strerror(result));
            goto ABORT;
          };
        };
      } else {
        pointer += 2 * result;
        count -= result;
      };
    };

    underrun_count = 0;

  };

  ABORT:;

  memory_allocate(&float_buffer, 0);
  memory_allocate(&short_buffer, 0);

  thread_run_flag = 0;

  thread_exit();

};

#endif

//--page-split-- alsa_initialize

#ifdef LINUX

void alsa_initialize() {

  alsa_error_function = NULL;

  int error;

  snd_pcm_hw_params_t *hwparams;
  snd_pcm_hw_params_alloca(&hwparams);

  error = snd_output_stdio_attach(&output, stdout, 0);
  if (error < 0) {
    alsa_error("snd_output_stdio_attach()", snd_strerror(error));
    return;
  };

  error = snd_pcm_open(&pcm_handle, pcm_output_device, SND_PCM_STREAM_PLAYBACK, 0);
  if (error < 0) {
    alsa_error("snd_pcm_open()", snd_strerror(error));
    return;
  };

  error = snd_pcm_hw_params_any(pcm_handle, hwparams);
  if (error < 0) {
    alsa_error("snd_pcm_hw_params_any()", snd_strerror(error));
    return;
  };

  error = snd_pcm_hw_params_set_rate_resample(pcm_handle, hwparams, 1);
  if (error < 0) {
    alsa_error("snd_pcm_hw_params_set_rate_resamble()", snd_strerror(error));
    return;
  };

  error = snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
  if (error < 0) {
    alsa_error("snd_pcm_hw_params_set_access()", snd_strerror(error));
    return;
  };

  error = snd_pcm_hw_params_set_format(pcm_handle, hwparams, SND_PCM_FORMAT_S16);
  if (error < 0) {
    alsa_error("snd_pcm_hw_params_set_format()", snd_strerror(error));
    return;
  };

  error = snd_pcm_hw_params_set_channels(pcm_handle, hwparams, 2);
  if (error < 0) {
    alsa_error("snd_pcm_hw_params_set_channels()", snd_strerror(error));
    return;
  };

  int actual_sample_rate = sample_rate;
  error = snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &actual_sample_rate, 0);
  if (error < 0) {
    alsa_error("snd_pcm_hw_params_set_channels()", snd_strerror(error));
    return;
  };

  if (actual_sample_rate != sample_rate) {
    fprintf(stderr, "ALSA gave us %d Hz...\n", actual_sample_rate);
    alsa_error("snd_pcm_hw_params_set_channels()", "Failed to set desired sampling rate.");
    return;
  };

  error = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, hwparams, &buffer_size);
  if (error < 0) {
    alsa_error("snd_pcm_hw_params_set_buffer_size_near()", snd_strerror(error));
    return;
  };

  period_size = buffer_size >> 2;

  int direction = -1;
  error = snd_pcm_hw_params_set_period_size_near(pcm_handle, hwparams, &period_size, &direction);
  if (error < 0) {
    alsa_error("snd_pcm_hw_params_set_period_size_near()", snd_strerror(error));
    return;
  };

  error = snd_pcm_hw_params(pcm_handle, hwparams);
  if (error < 0) {
    alsa_error("snd_pcm_hw_params()", snd_strerror(error));
    return;
  };

  snd_pcm_dump(pcm_handle, output);  // dump some shit to the screen

  thread_run_flag = 1;
  thread_exit_flag = 0;
  thread_create(pcm_output_thread, NULL);

};

#endif

//--page-split-- alsa_terminate

#ifdef LINUX

void alsa_terminate() {

  int error;

  if (thread_run_flag) {
    thread_exit_flag = 1;
    while (thread_run_flag) easy_sleep(0.001);
  };

  error = snd_pcm_close(pcm_handle);
  if (error < 0) alsa_error("snd_pcm_close()", snd_strerror(error));

  memory_allocate(&alsa_error_message, 0);

};

#endif
