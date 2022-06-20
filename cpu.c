#include "everything.h"

double cpu_sleep_time = 0.0;
int cpu_core_count = 2;

//--page-split-- cpu_initialize

void cpu_initialize() {
  #ifdef WINDOWS
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    cpu_core_count = sysinfo.dwNumberOfProcessors;
    #define OK
  #endif
  #ifdef LINUX
    cpu_core_count = sysconf(_SC_NPROCESSORS_ONLN);
    #define OK
  #endif
  #ifndef OK
    #error "There's no code to calculate number of cores for this platform."
  #endif
  #undef OK
  map_chunk_thread_count = cpu_core_count;
};

//--page-split-- cpu_analyze

void cpu_analyze() {
  DEBUG("enter cpu_analyze()");

  double min_next_frame_time = on_frame_time;
  double desired_fps = 0;
  if (option_fps_limit) {
    if (option_fps_limit == 1) {
      desired_fps = option_custom_fps;
      if (menu_function_pointer == menus_fps) {
        if (desired_fps < 20) desired_fps = 20;
      };
    } else {
      desired_fps = option_fps_limit;
    };
    if (desired_fps < 20 && (!map_is_active() || menu_function_pointer == menus_server_loading)) desired_fps = 20;
  } else if (statistics_vsync_rate) {
    desired_fps = statistics_vsync_rate;
  };

  if (desired_fps < 1.0) {
    min_next_frame_time = 0.0;
  } else if (desired_fps == statistics_vsync_rate) {
    min_next_frame_time = 0.0;
  } else {
    min_next_frame_time += 1.0 / desired_fps;
    static double fps_target_tweak = 0.0;
    fps_target_tweak += (statistics_current_fps - desired_fps) / (1000.0 * statistics_current_fps);
    if (fps_target_tweak < -0.001) fps_target_tweak = -0.001;
    if (fps_target_tweak > +0.001) fps_target_tweak = +0.001;
    //printf("FPS tweak: %0.1f us\n", fps_target_tweak * 1000000.0);
    min_next_frame_time += fps_target_tweak;
  };

  double max_next_frame_time = on_frame_time;
  max_next_frame_time += 1.0 / option_fps_goal;

  if (!menus_server_loading_active) {
    for (int i = 0;; i++) {
      lag_push(10, "map_process_a_chunk()");
      int unprocessed_chunks = map_process_a_chunk();
      lag_pop();
      if (i >= unprocessed_chunks / statistics_frames_per_second) break;
      if (easy_time() >= max_next_frame_time) break;
    };
  };

  // If we're ahead, sleep until it is time for the next frame.

  if (easy_time() < min_next_frame_time) {
    cpu_sleep_time = min_next_frame_time - easy_time();
    lag_push(100, "limiting frame rate");
    easy_sleep(cpu_sleep_time);
    lag_pop();
  } else {
    cpu_sleep_time = 0.0;
  };

  DEBUG("leave cpu_analyze()");
};
