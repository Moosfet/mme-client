#include "everything.h"

char statistics_string[1024] = {};
char statistics_report[1024] = {};

double statistics_minimum_fog = 10.0;
double statistics_fog_distance = 100.0;
double statistics_maximum_fog = 1000.0;

int statistics_vsync_rate = 0.0;
double statistics_current_fps = 0.0;
double statistics_current_cps = 0.0;
double statistics_peak_frame_time = 0.0;
double statistics_current_fps_q = 0.0;

double statistics_chunks_per_second = 0.0;
double statistics_frames_per_second = 1.0;
double statistics_chunk_percentage = 0.0;

static int frame_count = 0;

static int chunk_count = 0;
static int quad_count = 0;
static int chunk_percent = 0;

//--page-split-- statistics_count_frame

void statistics_count_frame() {
  frame_count++;

  // Also, we will determine what statistics to display!

  static double frame_time[1024] = {}; // time of the last few frames
  static int chunk_time[1024]; // chunk count at the last few frames
  double interval;

  // shift the arrays and add the current frame time and chunk count to the ends
  memmove(frame_time, frame_time + 1, 1023 * sizeof(double));
  memmove(chunk_time, chunk_time + 1, 1023 * sizeof(int));
  frame_time[1023] = on_frame_time;
  chunk_time[1023] = chunk_count;

  // search the array of frame times for a frame that occured about 1 second ago
  int i;
  for (i = 1022; i >= 0; i--) {
    if (frame_time[i] <= on_frame_time - 1.00) break;
  };

  // If the time of that frame is valid...
  if (frame_time[i] > 0) {
    double interval = on_frame_time - frame_time[i];
    double fps = (1023 - i) / interval;
    double cps = (chunk_count - chunk_time[i]) / interval;
    double speed = sqrt(player_velocity.x * player_velocity.x + player_velocity.y * player_velocity.y + player_velocity.z * player_velocity.z);
    statistics_current_fps = fps;
    statistics_current_cps = cps;
    double pft = 0.0;
    for (; i < 1023; i++) {
      double ft = frame_time[i+1] - frame_time[i];
      if (pft < ft) pft = ft;
    };
    statistics_peak_frame_time = pft;
    statistics_current_fps_q = (1.0 / statistics_current_fps) / pft;
    sprintf(statistics_report, "%0.0f FPS, %0.2f Q, fog=%0.0f, %0.0f c/s, %0.0f%% built, %0.0f%% in view", fps, statistics_current_fps_q, statistics_fog_distance / map_data.resolution.x, statistics_current_cps, statistics_chunk_percentage, map_percentage_in_view);
  };

  if (option_fog_distance <= 0) {
    // try to guess the best fog depth according to current performance
    double current_frame_interval = frame_time[1023] - frame_time[1022];
    current_frame_interval -= cpu_sleep_time; if (current_frame_interval < 0.001) current_frame_interval = 0.001;

    double ideal_frame_interval = 1.0 / option_fps_goal;
    //if (option_fps_limit) ideal_frame_interval = 1.0 / option_fps_limit;

    //printf("cfi = %0.3f  idf = %0.3f\n", current_frame_interval, ideal_frame_interval);
    double fraction = fabs(1 - sqrt(current_frame_interval / ideal_frame_interval));
    if (fraction > 1.0) fraction = 1.0;
    statistics_fog_distance = fraction * sqrt(ideal_frame_interval / current_frame_interval) * statistics_fog_distance + (1 - fraction) * statistics_fog_distance;
  } else {
    int fog_distance = option_fog_distance;
    #if 0
    // this limits the fog distance to 100 meters
    if (fog_distance > 100 / map_data.resolution.x) {
      fog_distance = 100 / map_data.resolution.x;
    };
    #endif
    statistics_fog_distance = fog_distance * map_data.resolution.x;
  };

  statistics_minimum_fog = 10;
  statistics_maximum_fog = 1000000;

  // Set a maximum fog level, to prevent insane fog levels.
  if (option_fog_distance == 0) {
    double maximum = sqrt(
      map_data.resolution.x * map_data.resolution.x * map_data.dimension.x * map_data.dimension.x +
      map_data.resolution.y * map_data.resolution.y * map_data.dimension.y * map_data.dimension.y +
      map_data.resolution.z * map_data.resolution.z * map_data.dimension.z * map_data.dimension.z
    );
    if (statistics_maximum_fog > maximum) statistics_maximum_fog = maximum;
  };

  // Fog limit imposed by chunk selection:
  #define whatthefuck(i) ((1 << map_chunk_bits) * ((map_data.dimension.i / (1 << map_chunk_bits) - 1) >> 1))
  if (map_data.wrap.x) {
    double maximum = map_data.resolution.x * whatthefuck(x);
    if (statistics_maximum_fog >= maximum) statistics_maximum_fog = maximum;
  };
  if (map_data.wrap.y) {
    double maximum = map_data.resolution.y * whatthefuck(y);
    if (statistics_maximum_fog >= maximum) statistics_maximum_fog = maximum;
  };
  if (map_data.wrap.z) {
    double maximum = map_data.resolution.z * whatthefuck(z);
    if (statistics_maximum_fog >= maximum) statistics_maximum_fog = maximum;
  };

  if (statistics_fog_distance > statistics_maximum_fog) statistics_fog_distance = statistics_maximum_fog;
  if (statistics_fog_distance < statistics_minimum_fog) statistics_fog_distance = statistics_minimum_fog;

  //if (!(option_fog_type & 8)) statistics_fog_distance = option_file.fog_distance;

};

//--page-split-- statistics_count_chunk

void statistics_count_chunk() {
  chunk_count++;
};

//--page-split-- statistics_count_quads

void statistics_count_quads(int change) {
  quad_count = change;
};

//--page-split-- statistics_complete_chunks

void statistics_complete_chunks(int count, int total) {
  chunk_percent = floor(100.0 * count / total);
  statistics_chunk_percentage = 100.0 * count / total;
};

//--page-split-- statistics_open_window

void statistics_open_window() {
  // Figure out if our OpenGL is limiting our frame rate...

  #ifdef WINDOWS
    // WTF is this for?  I wish I commented more stuff.
    glfwSwapBuffers(glfw_window);
    // Like, assuming it's even necessary, why not do it on Linux too?
  #endif

  double last_time, this_time, sum = 0.0, minimum = 1.0;
  glfwSwapBuffers(glfw_window); this_time = easy_time();
  for (int i = 0; i < 20; i++) {
    last_time = this_time;
    glfwSwapBuffers(glfw_window); this_time = easy_time();
    double interval = this_time - last_time;
    if (interval < minimum) minimum = interval;
    sum += interval;
  };
  double average = sum / 20;

  printf("Average frame interval is %0.1f ms (%0.0f FPS)\n", 1000 * average, 1.0 / average);
  printf("Minimum frame interval is %0.1f ms (%0.0f FPS)\n", 1000 * minimum, 1.0 / minimum);

  double fps = round(1.0 / average);
  if (fps < 100) {
    statistics_vsync_rate = fps;
  } else {
    statistics_vsync_rate = 0;
  };

};
