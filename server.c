#include "everything.h"

int server_action = SERVER_ACTION_CONNECT;
int server_status = SERVER_STATUS_DISCONNECTED;
char *server_error_string = NULL;
static char *custom_error_string = NULL;
double server_reconnect_time = 0;
static double last_packet_time = 0;
static double last_ping_request = 0;
static double disconnect_packet_time = 0;
static double last_width_report = -1;
static double last_height_report = -1;

static char *input_buffer = NULL;
static char *output_buffer = NULL;
int server_input_size = 0;
int server_output_size = 0;
#define input_size server_input_size
#define output_size server_output_size

double server_connect_time = -1.0;
double server_ping_time = 0.0;

char *server_address = NULL;
char *server_authentication = NULL;
char *server_portal_authentication = NULL;

static int received_packet_selections;
static int sent_authentication_packet;

static struct structure_socket_data *the_socket;

static double last_position_time = 0;

//--page-split-- communicate_with_server

static void communicate_with_server() {

  // Read from the network socket into our buffer...

  lag_push(1, "network_read()");
  network_read(&the_socket, &input_buffer, &input_size);
  lag_pop();

  // Then parse packets from that buffer...

  lag_push(15, "packet_parse_stream()");
  char *pointer = packet_parse_stream(input_buffer, input_size);
  lag_pop();

  // ...and check for error messages from packet_parse_stream()...

  if (pointer < input_buffer || pointer > input_buffer + input_size) {
    // protocol error, return value is an error string
    memory_allocate(&custom_error_string, 4096);
    sprintf(custom_error_string, "Communication error: %s", pointer);
    memory_allocate(&custom_error_string, strlen(custom_error_string) + 1);
    server_action = SERVER_ACTION_DISCONNECT;
    server_error_string = custom_error_string;
    if (packet_send(PACKET_DISCONNECT, DISCONNECT_ERROR, 0, custom_error_string)) {
      server_status = SERVER_STATUS_DISCONNECTING;
    } else {
      network_free_socket(&the_socket);
      server_status = SERVER_STATUS_DISCONNECTED;
      on_server_disconnect();
    };
    return;
  } else if (pointer > input_buffer) {
    memmove(input_buffer, pointer, input_size - (int) (long long) ((char *) pointer - (char *) input_buffer));
    input_size -= (int) (long long) ((char *) pointer - (char *) input_buffer);
    memory_allocate(&input_buffer, input_size);
  };

  //#ifdef TEST
  // Measure server ping time...
  static double last_ping_time = 0;
  if (last_ping_time < on_frame_time - 1.0) {
    packet_send(PACKET_PING_REQUEST, &on_frame_time, sizeof(on_frame_time));
    last_ping_time = on_frame_time;
  };
  //#endif

  // Send position update, but only if 50 ms since the last time we
  // sent an update, and only if the binary data to be sent differs
  // from the last data we sent, so as to ignore irrelevant changes.

  if (last_position_time < on_frame_time - 1.0 / 20.0) {
    static double last[5];
    int update = 0;
    if (fabs(last[0] - player_position.x) > 0.01) update = 1;
    if (fabs(last[1] - player_position.y) > 0.01) update = 1;
    if (fabs(last[2] - player_position.z) > 0.01) update = 1;
    if (last[3] != player_position.u) update = 1;
    if (last[4] != player_position.v) update = 1;

    // Let's send the position if we're otherwise going to send a ping...
    //if (last_packet_time < on_frame_time - 10.0) update = 1;

    if (update) {
      packet_send(PACKET_MOVE_PLAYER, player_position.x, player_position.y, player_position.z, player_position.u, player_position.v, 0);
      last[0] = player_position.x;
      last[1] = player_position.y;
      last[2] = player_position.z;
      last[3] = player_position.u;
      last[4] = player_position.v;
      last_position_time = on_frame_time;
    };
  };

  // If we haven't received anything for 15 seconds, send a ping request.

  if (network_last_response < on_frame_time - 15.0) {
    if (last_ping_request < on_frame_time - 10.0) {
      packet_send(PACKET_PING_REQUEST, NULL, 0);
      last_ping_request = on_frame_time;
    };
  };

  // Send keepalive if we haven't sent anything in 10 seconds.

  if (last_packet_time < on_frame_time - 10.0) {
    packet_send(PACKET_IDLE_PING);
    last_packet_time = on_frame_time;
  };

  // Send information about window size for server menus.
  if (menus_server_menu_active) {
    if (last_width_report != gui_text_columns || last_height_report != gui_text_lines) {
      packet_send(PACKET_MENU_RESIZE, gui_text_columns, gui_text_lines, menus_server_menu_number);
      last_width_report = gui_text_columns;
      last_height_report = gui_text_lines;
    };
  };

  // Send some of the output buffer to the server...

  lag_push(1, "network_write()");
  network_write(&the_socket, &output_buffer, &output_size);
  lag_pop();

  // Check that we are still connected to the server.

  if (server_status == SERVER_STATUS_CONNECTED) {
    // We only check if we thought we were connected.
    if (network_status(&the_socket) <= 0) {
      if (server_status == SERVER_STATUS_CONNECTED) {
        server_error_string = network_error_string;
      };
      server_action = SERVER_ACTION_DISCONNECT;
      network_free_socket(&the_socket);
      server_status = SERVER_STATUS_DISCONNECTED;
      on_server_disconnect();
    };
  };

};

//--page-split-- server_terminate

void server_terminate() {
  memory_allocate(&server_address, 0);
  memory_allocate(&server_authentication, 0);
  memory_allocate(&server_portal_authentication, 0);
  memory_allocate(&custom_error_string, 0);
  menus_server_menu_reset();
};

//--page-split-- server_stuff

void server_stuff() {
  DEBUG("enter server_stuff()");

  if (server_status == SERVER_STATUS_DISCONNECTED) {
    DEBUG("Enter: server_status == STATUS_DISCONNECTED");
    if (input_buffer != NULL) memory_allocate(&input_buffer, 0);
    if (output_buffer != NULL) memory_allocate(&output_buffer, 0);
    if (map_data.block) {
      on_map_unload();
      memory_allocate(&map_data.block, 0);
      map_data.limit = 0;
    };
    if (server_action == SERVER_ACTION_CONNECT) {
      if (server_address == NULL) {
        memory_allocate(&server_address, strlen(PORTAL_ADDRESS) + 1);
        strcpy(server_address, PORTAL_ADDRESS);
      };
      server_error_string = NULL;
      packet_reset();
      received_packet_selections = 0;
      sent_authentication_packet = 0;
      memory_allocate(&input_buffer, 0); input_size = 0;
      memory_allocate(&output_buffer, 0); output_size = 0;
      network_initiate_connect(&the_socket, server_address);
      server_status = SERVER_STATUS_CONNECTING;
      disconnect_packet_time = 0;
      server_reconnect_time = -1;
      last_packet_time = on_frame_time;
      last_ping_request = on_frame_time;
      // Also reset game parameters upon new server connect:
      player_allow_flying = 0;
      player_allow_speedy = 0;
      player_allow_noclip = 0;
      menus_play_box_volume_limit = 1;
      menus_play_box_dimension_limit = 1;
      last_width_report = -1;
      last_height_report = -1;
    };
    DEBUG("Leave: server_status == STATUS_DISCONNECTED");
  };

  if (server_status == SERVER_STATUS_CONNECTING) {
    DEBUG("Enter: server_status == STATUS_CONNECTING");
    if (server_action == SERVER_ACTION_CONNECT) {
      int status = network_status(&the_socket);
      if (status > 0) {
        server_status = SERVER_STATUS_CONNECTED;
        on_server_connect();
        packet_send(PACKET_AVAILABLE_PACKET_TYPES);
      } else if (status < 0) {
        server_error_string = network_error_string;
        server_action = SERVER_ACTION_DISCONNECT;
        network_free_socket(&the_socket);
        server_status = SERVER_STATUS_DISCONNECTED;
        on_server_disconnect();
      };
    } else if (server_action == SERVER_ACTION_DISCONNECT) {
      network_free_socket(&the_socket);
      server_status = SERVER_STATUS_DISCONNECTED;
      on_server_disconnect();
    };
    DEBUG("Leave: server_status == STATUS_CONNECTING");
  };

  if (server_status == SERVER_STATUS_CONNECTED) {
    DEBUG("Enter: server_status == STATUS_CONNECTED");
    if (server_action == SERVER_ACTION_CONNECT) {
      if (received_packet_selections && !sent_authentication_packet) {
        packet_send(PACKET_CLIENT_VERSION);
        if (strcmp(server_address, PORTAL_ADDRESS)) {
          packet_send(PACKET_AUTHENTICATE, server_authentication);
        } else {
          if (option_have_cookie && packet_is_sendable(PACKET_PORTAL_COOKIE)) {
            packet_send(PACKET_PORTAL_COOKIE, option_cookie_data);
          } else if (server_portal_authentication != NULL && packet_is_sendable(PACKET_AUTHENTICATE)) {
            packet_send(PACKET_AUTHENTICATE, server_portal_authentication);
          } else if (packet_is_sendable(PACKET_PORTAL_COOKIE)) {
            packet_send(PACKET_PORTAL_COOKIE, NULL);
          } else {
            packet_send(PACKET_AUTHENTICATE, NULL);
          };
        };
        sent_authentication_packet = 1;
      };
      communicate_with_server();
    } else if (server_action == SERVER_ACTION_DISCONNECT) {
      if (packet_send(PACKET_DISCONNECT, DISCONNECT_RANDOM, 0, "Player requested disconnect.")) {
        disconnect_packet_time = on_frame_time;
        server_status = SERVER_STATUS_DISCONNECTING;
      } else {
        network_free_socket(&the_socket);
        server_status = SERVER_STATUS_DISCONNECTED;
        on_server_disconnect();
      };
    };
    DEBUG("Leave: server_status == STATUS_CONNECTED");
  };

  if (server_status == SERVER_STATUS_DISCONNECTING) {
    DEBUG("Enter: server_status == STATUS_DISCONNECTING");
    network_read(&the_socket, &input_buffer, &input_size);
    memory_allocate(&input_buffer, 0);
    network_write(&the_socket, &output_buffer, &output_size);
    if (network_status(&the_socket) <= 0 || disconnect_packet_time < on_frame_time - 3.0) {
      server_action = SERVER_ACTION_DISCONNECT;
      network_free_socket(&the_socket);
      server_status = SERVER_STATUS_DISCONNECTED;
      on_server_disconnect();
    };
    DEBUG("Leave: server_status == STATUS_DISCONNECTING");
  };

  if (server_error_string != NULL && menu_function_pointer != menus_exit) {
    if (server_reconnect_time < 0) {
      // For errors without explicit reconnect delay...
      server_reconnect_time = on_frame_time + 3;
    };
    menu_switch(menus_server_error);
  };

  DEBUG("leave server_stuff()");
};

//--page-split-- server_menu_request

void server_menu_request() {
  packet_send(PACKET_MENU_REQUEST, gui_text_columns, gui_text_lines);
  last_width_report = gui_text_columns;
  last_height_report = gui_text_lines;
};

//--page-split-- server_send_raw_data

void server_send_raw_data(char *buffer, int length) {
  if (server_status != SERVER_STATUS_CONNECTED && server_status != SERVER_STATUS_DISCONNECTING) return;
  memory_allocate(&output_buffer, output_size + length);
  memmove(output_buffer + output_size, buffer, length);
  output_size += length;
  last_packet_time = on_frame_time;
};

//--page-split-- server_process_packet

void server_process_packet(int packet_type, char *packet_data, int packet_size) {

  static int megadata_size;
  static char *megadata_buffer;
  static z_stream fulldata_strm;
  static int fulldata_active = 0;
  static int fulldata_lba = 0;

  #define DATA(OFFSET, TYPE) (*((TYPE*) (packet_data + OFFSET)))

  if (server_status != SERVER_STATUS_CONNECTED || server_action != SERVER_ACTION_CONNECT) return;

  if (packet_type == PACKET_IDLE_PING) {
  } else if (packet_type == PACKET_PING_RESPONSE) {
    //#ifdef TEST
    if (packet_size == 8) {
      server_ping_time = on_frame_time - DATA(0, double);
    };
    //#endif
  } else if (packet_type == PACKET_SELECT_PACKET_TYPES) {
    received_packet_selections = 1;
  } else if (packet_type == PACKET_CHAT_MESSAGE) {
    if (packet_size < 1) {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    } else {
      int player = DATA(0, char);
      int size = packet_size - 1;
      char *message = NULL;
      memory_allocate(&message, size + 1);
      memmove(message, packet_data + 1, size);
      message[size] = 0;
      chat_message(message);
      memory_allocate(&message, 0);
    };
  } else if (packet_type == PACKET_MAP_SETUP) {
    lag_push(1000, "receiving a PACKET_MAP_SETUP");
    if (packet_size == 0) {
      if (map_data.block) {
        on_map_unload();
        memory_allocate(&map_data.block, 0);
        map_data.limit = 0;
      };
    } else if (packet_size < 8) {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    } else {
      if (map_data.block) {
        on_map_unload();
        memory_allocate(&map_data.block, 0);
        map_data.limit = 0;
      };
      map_data.dimension.x = DATA(0, unsigned short);
      map_data.dimension.y = DATA(2, unsigned short);
      map_data.dimension.z = DATA(4, unsigned short);
      map_data.sealevel = DATA(6, unsigned short);
      map_data.limit = map_data.dimension.x * map_data.dimension.y * map_data.dimension.z;
      printf("New map dimensions: %d x %d x %d, sealevel = %d\n", map_data.dimension.x, map_data.dimension.y, map_data.dimension.z, map_data.sealevel);
      if (map_data.limit) {
        memory_allocate(&map_data.block, map_data.limit);
        memset(map_data.block, 0, map_data.limit);
        lag_push(1000, "on_map_load()");
        on_map_load();
        lag_pop();
      };
      map_data.resolution.x = 0.1;
      map_data.resolution.y = 0.1;
      map_data.resolution.z = 0.1;
      map_data.wrap.x = 0;
      map_data.wrap.y = 0;
      map_data.wrap.z = 0;
    };
    lag_pop();
  } else if (packet_type == PACKET_MAP_PROPERTIES) {
    if (packet_size < 8) {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    } else {
      if (argument_packets && packet_size > 8) printf("\e[1;33m%s: Packet contains more data than I expect! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
      map_data.wrap.x = 0;
      map_data.wrap.y = 0;
      map_data.wrap.z = 0;
      double scale = 0.05 * round(DATA(4, float) / 0.05);
      if (scale < 0.1) {
        map_data.resolution.x = 0.1;
        map_data.resolution.y = 0.1;
        map_data.resolution.z = 0.1;
      } else if (scale > 1.0) {
        map_data.resolution.x = 1.0;
        map_data.resolution.y = 1.0;
        map_data.resolution.z = 1.0;
      } else {
        map_data.resolution.x = scale;
        map_data.resolution.y = scale;
        map_data.resolution.z = scale;
      };
      //map_data.resolution.x = map_data.resolution.y = map_data.resolution.z = 0.7;
      map_invalidate_chunks();
    };
    printf("New map resolution = %0.2f x %0.2f x %0.2f\n", map_data.resolution.x, map_data.resolution.y, map_data.resolution.z);
  } else if (packet_type == PACKET_MAP_MODIFY) {
    if (packet_size < 7) {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    } else {
      if (argument_packets && packet_size > 8) printf("\e[1;33m%s: Packet contains more data than I expect! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
      if (!map_data.block) return;
      int type = DATA(6, char);
      if (type != 0) {
        int x = DATA(0, unsigned short);
        int y = DATA(2, unsigned short);
        int z = DATA(4, unsigned short);
        map_modify((struct int_xyz) {x, y, z}, type, 1);
      };
    };
  } else if (packet_type == PACKET_MOVE_PLAYER) {
    if (packet_size < 20) {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    } else {
      if (argument_packets && packet_size > 21) printf("\e[1;33m%s: Packet contains more data than I expect! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
      double x = DATA(0, float);
      double y = DATA(4, float);
      double z = DATA(8, float);
      double u = DATA(12, float);
      double v = DATA(16, float);
      if (packet_size > 20 && DATA(20, char) != 0) {
        int i = DATA(20, char);
        model_move(i, (struct double_xyzuv) {x, y, z, u, v});
      } else {
        player_teleport((struct double_xyzuv) {x, y, z, u, v});
      };
    };
  } else if (packet_type == PACKET_CONNECT) {
    if (packet_size == 52 && !strcmp(server_address, PORTAL_ADDRESS)) {
      memory_allocate(&server_portal_authentication, 52);
      memmove(server_portal_authentication, &DATA(0, char), 52);
    } else if (packet_size >= 53) {
      memory_allocate(&server_authentication, 52);
      memmove(server_authentication, &DATA(0, char), 52);
      memory_allocate(&server_address, packet_size - 52 + 1);
      memmove(server_address, &DATA(52, char), packet_size - 52);
      server_address[packet_size - 52] = 0;
      network_free_socket(&the_socket);
      server_status = SERVER_STATUS_DISCONNECTED;
      on_server_disconnect();
      server_action = SERVER_ACTION_CONNECT;
      server_connect_time = on_frame_time;
      menu_switch(menus_server_connect);
    } else {
      printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    };
  } else if (packet_type == PACKET_PORTAL_COOKIE) {
    if (!strcmp(server_address, PORTAL_ADDRESS)) {
      if (packet_size == 20) {
        memmove(option_cookie_data, &DATA(0, char), 20);
        option_have_cookie = 1;
        option_save();
      } else {
        memset(option_cookie_data, 0, 20);
        option_have_cookie = 0;
        option_save();
      };
    };
  } else if (packet_type == PACKET_DISCONNECT) {
    int reason = 0;
    int delay = 0;
    if (packet_size >= 1) reason = DATA(0, char);
    if (packet_size >= 2) delay = DATA(1, char);
    if (packet_size >= 3) {
      char *message = NULL;
      int length = packet_size - 2;
      memory_allocate(&message, length + 1);
      memmove(message, &packet_data[2], length);
      message[length] = 0;
      memory_allocate(&custom_error_string, 256 + length);
      sprintf(custom_error_string, "Server Disconnect: %s", message);
      memory_allocate(&custom_error_string, strlen(custom_error_string) + 1);
      memory_allocate(&message, 0);
      server_error_string = custom_error_string;
    } else {
      if (reason == DISCONNECT_NULL) {
        server_error_string = "The server initiated disconnection, but gave no explaination.";
      } else if (reason == DISCONNECT_ERROR) {
        server_error_string = "The server initiated disconnection because of a communication error.";
      } else if (reason == DISCONNECT_REBOOT) {
        server_error_string = "The server is rebooting.  You may reconnect in a few minutes.";
      } else if (reason == DISCONNECT_FULL) {
        server_error_string = "The server is too busy to allow new players to join.";
      } else if (reason == DISCONNECT_IDLE) {
        server_error_string = "The server is too busy to allow idle players to remain connected.";
      } else if (reason == DISCONNECT_KICK) {
        server_error_string = "The server initiated disconnection as a disciplinary action.";
      } else if (reason == DISCONNECT_BAN) {
        server_error_string = "The server will not allow you to connect.";
      } else if (reason == DISCONNECT_RANDOM) {
        server_error_string = "Congratulations!  You've won the random disconnection lottery!";
      } else {
        server_error_string = "Server initiated disconnection for an unknown reason.";
      };
    };
    if (delay > 0) {
      server_reconnect_time = on_frame_time + delay;
    } else {
      server_reconnect_time = 0;
    };
    network_free_socket(&the_socket);
    server_status = SERVER_STATUS_DISCONNECTED;
    on_server_disconnect();
    server_action = SERVER_ACTION_DISCONNECT;
    if (reason == DISCONNECT_RANDOM && !strcmp(server_address, PORTAL_ADDRESS)) menu_switch(menus_exit);
  } else if (packet_type == PACKET_ANNOUNCE_PLAYER) {
    if (packet_size < 26) {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    } else {
      if (argument_packets && packet_size > 26) printf("\e[1;33m%s: Packet contains more data than I expect! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
      int i = DATA(0, char);
      model_player[i].valid = 1;
      model_player[i].color = DATA(1, char);
      memmove(model_player[i].name, &DATA(2, char), 24);
      model_player[i].name[24] = 0;
    };
  } else if (packet_type == PACKET_DENOUNCE_PLAYER) {
    if (packet_size < 1) {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    } else {
      if (argument_packets && packet_size > 1) printf("\e[1;33m%s: Packet contains more data than I expect! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
      int i = DATA(0, char);
      model_player[i].valid = 0;
    };
  } else if (packet_type == PACKET_PLAYER_ABILITIES) {
    if (packet_size < 10) {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    } else {
      if (argument_packets && packet_size > 10) printf("\e[1;33m%s: Packet contains more data than I expect! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
      int bitmask = DATA(0, unsigned int);
      int v_limit = DATA(4, unsigned int);
      int d_limit = DATA(8, unsigned short);
      // Clip unsigned values to signed values.
      if (v_limit < 0) v_limit = 0x7FFFFFFF;
      player_allow_flying = (bitmask & 0x01) != 0;
      player_allow_speedy = (bitmask & 0x02) != 0;
      player_allow_noclip = (bitmask & 0x04) != 0;
      menus_play_box_volume_limit = v_limit;
      menus_play_box_dimension_limit = d_limit;
    };
  } else if (packet_type == PACKET_MAP_LOADING) {
    if (packet_size == 0) {
      menus_server_loading_active = 0;
      map_invalidate_chunks();
      memory_allocate(&menus_server_loading_message, 0);
      if (menu_function_pointer == menus_server_loading) menu_switch(menus_play);
    } else {
      menus_server_loading_active = 1;
      menu_switch(menus_server_loading);
      menus_server_loading_progress = DATA(0, char);
      if (packet_size >= 2) {
        memory_allocate(&menus_server_loading_message, packet_size);
        memmove(menus_server_loading_message, &DATA(1, char), packet_size - 1);
        menus_server_loading_message[packet_size - 1] = 0;
      };
    };
  } else if (packet_type == PACKET_MAP_DATA) {
    if (packet_size < 12) {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    } else {
      lag_push(12, "receiving a PACKET_MAP_DATA");
      int x1 = DATA(0, short);
      int y1 = DATA(2, short);
      int z1 = DATA(4, short);
      int x2 = DATA(6, short);
      int y2 = DATA(8, short);
      int z2 = DATA(10, short);
      if (x2 < x1 || y2 < y1 || z2 < z1) {
        if (argument_packets) printf("\e[1;31mPACKET_MAP_DATA: Invalid coordinates!\e[0m\n");
      } else {
        int size = (x2 - x1 + 1) * (y2 - y1 + 1) * (z2 - z1 + 1);
        char *data = NULL;
        memory_allocate(&data, size);
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.next_in = &packet_data[12];
        strm.avail_in = packet_size - 12;
        strm.next_out = data;
        strm.avail_out = size;
        while (strm.avail_in != 0 && strm.avail_out != 0) {
          inflateInit2(&strm, 47);
          int result = inflate(&strm, Z_FINISH);
          inflateEnd(&strm);
          if (result < 0) {
            if (argument_packets) {
              printf("\e[1;31mzlib returned error %d after deflation\e[0m\n", result);
              printf("(avail_in=%d, avail_out=%d)\n", strm.avail_in, strm.avail_out);
            };
            break;
          };
        };
        if (strm.avail_out != 0 || strm.avail_in != 0) {
          if (argument_packets) {
            printf("\e[1;31mPACKET_MAP_DATA: Decompressed data size does not match map area size!\e[0m\n");
            printf("(in_size=%d, out_size=%d, area_size=%d, avail_in=%d, avail_out=%d)\n", packet_size - 12, size - strm.avail_out, size, strm.avail_in, strm.avail_out);
          };
        } else {
          DEBUG("enter map data update");
          for (int z = z1; z <= z2; z++) {
            for (int y = y1; y <= y2; y++) {
              for (int x = x1; x <= x2; x++) {
                int index = ((z - z1) * (y2 - y1 + 1) + (y - y1)) * (x2 - x1 + 1) + (x - x1);
                if (data[index] != 0) map_modify((struct int_xyz) {x, y, z}, data[index], 2);
              };
            };
          };
          DEBUG("leave map data update");
        };
        memory_allocate(&data, 0);
      };
      lag_pop();
    };
  } else if (packet_type == PACKET_BUFFER_RESET) {
    memory_allocate(&megadata_buffer, packet_size);
    memmove(megadata_buffer, packet_data, packet_size); megadata_size = packet_size;
  } else if (packet_type == PACKET_BUFFER_APPEND) {
    memory_allocate(&megadata_buffer, megadata_size + packet_size);
    memmove(megadata_buffer + megadata_size, packet_data, packet_size); megadata_size += packet_size;
  } else if (packet_type == PACKET_BUFFER_IS_FILE_DATA) {
    char binary[20], ascii[41]; ascii[40] = 0;
    sha1(binary, megadata_buffer, megadata_size);
    easy_binary_to_ascii(ascii, binary, 20);
    if (argument_packets) printf("\e[1;32mReceived a file with SHA-1 digest of %s.\e[0m\n", ascii);
    char pathname[64];
    sprintf(pathname, "cache/%s", ascii);
    FILE *file = fopen(pathname, "wb");
    if (file != NULL) {
      if (fwrite(megadata_buffer, 1, megadata_size, file) != megadata_size) {
        printf("Failed to write data to cache!\n");
      };
      fclose(file);
    } else {
      printf("Error while creating cache file: %s\n", error_string(errno));
      printf("file = %p, error_string = %p, errno = %d\n", file, error_string(errno), errno);
    };
    for (int texture = 0; texture < TEXTURE_MAX_SERVER_TEXTURES; texture++) {
      if (texture_data[texture].digest != NULL) {
        if (memcmp(texture_data[texture].digest, binary, 20) == 0) {
          texture_list(texture, megadata_buffer, megadata_size, texture_data[texture].flags);
          memory_allocate(&texture_data[texture].digest, 0);
        };
      };
    };
    memory_allocate(&megadata_buffer, 0); megadata_size = 0;
  } else if (packet_type == PACKET_BUFFER_IS_MAP_DATA) {
    if (packet_size < 12) {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    } else {
      lag_push(100, "receiving a PACKET_BUFFER_IS_MAP_DATA");
      int x1 = DATA(0, short);
      int y1 = DATA(2, short);
      int z1 = DATA(4, short);
      int x2 = DATA(6, short);
      int y2 = DATA(8, short);
      int z2 = DATA(10, short);
      if (x2 < x1 || y2 < y1 || z2 < z1) {
        if (argument_packets) printf("\e[1;31mPACKET_BUFFER_IS_MAP_DATA: Invalid coordinates!\e[0m\n");
      } else {
        int size = (x2 - x1 + 1) * (y2 - y1 + 1) * (z2 - z1 + 1);
        char *data = NULL;
        memory_allocate(&data, size);
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.next_in = megadata_buffer;
        strm.avail_in = megadata_size;
        strm.next_out = data;
        strm.avail_out = size;
        while (strm.avail_in != 0 && strm.avail_out != 0) {
          inflateInit2(&strm, 47);
          int result = inflate(&strm, Z_FINISH);
          inflateEnd(&strm);
          if (result < 0) {
            if (argument_packets) {
              printf("\e[1;31mzlib returned error %d after deflation\e[0m\n", result);
              printf("(avail_in=%d, avail_out=%d)\n", strm.avail_in, strm.avail_out);
            };
          };
        };
        if (strm.avail_out != 0 || strm.avail_in != 0) {
          if (argument_packets) {
            printf("\e[1;31mPACKET_BUFFER_IS_MAP_DATA: Decompressed data size does not match map area size!\e[0m\n");
            printf("(in_size=%d, out_size=%d, area_size=%d, avail_in=%d, avail_out=%d)\n", packet_size - 12, size - strm.avail_out, size, strm.avail_in, strm.avail_out);
          };
        } else {
          DEBUG("enter map data update");
          for (int z = z1; z <= z2; z++) {
            for (int y = y1; y <= y2; y++) {
              for (int x = x1; x <= x2; x++) {
                int index = ((z - z1) * (y2 - y1 + 1) + (y - y1)) * (x2 - x1 + 1) + (x - x1);
                if (index >= map_data.limit) continue;
                if (data[index] != 0) map_modify((struct int_xyz) {x, y, z}, data[index], 3);
              };
            };
          };
          DEBUG("leave map data update");
        };
        memory_allocate(&data, 0);
      };
      lag_pop();
    };
    memory_allocate(&megadata_buffer, 0); megadata_size = 0;
  } else if (packet_type == PACKET_MAP_TOTAL_RESET) {
    if (fulldata_active) {
      inflateEnd(&fulldata_strm);
      fulldata_active = 0;
    };
    goto TOTAL_DATA;
  } else if (packet_type == PACKET_MAP_TOTAL_DATA) {
    TOTAL_DATA:
    if (!map_data.block) return;
    lag_push(100, "receiving a PACKET_MAP_TOTAL_DATA");
    if (fulldata_active && (fulldata_lba < 0 || fulldata_lba >= map_data.limit)) {
      inflateEnd(&fulldata_strm);
      fulldata_active = 0;
    };
    char buffer[65536];
    if (!fulldata_active) {
      //memset(&fulldata_strm, 0, sizeof(z_stream));
      fulldata_strm.zalloc = Z_NULL;
      fulldata_strm.zfree = Z_NULL;
      fulldata_strm.opaque = Z_NULL;
      fulldata_strm.next_in = packet_data;
      fulldata_strm.avail_in = packet_size;
      fulldata_strm.next_out = NULL;
      fulldata_strm.avail_out = 0;
      inflateInit2(&fulldata_strm, 47);
      fulldata_active = 1;
      fulldata_lba = 0;
    };
    fulldata_strm.next_in = packet_data;
    fulldata_strm.avail_in = packet_size;
    while (fulldata_strm.avail_in) {
      fulldata_strm.next_out = buffer;
      fulldata_strm.avail_out = 65536;
      int result = inflate(&fulldata_strm, Z_SYNC_FLUSH);
      if (result < 0) {
        if (argument_packets) {
          printf("\e[1;31mzlib returned error %d after deflation\e[0m\n", result);
          printf("(avail_in=%d, avail_out=%d)\n", fulldata_strm.avail_in, fulldata_strm.avail_out);
        };
        inflateEnd(&fulldata_strm);
        fulldata_active = 0;
        break;
      };
      int size = 65536 - fulldata_strm.avail_out;
      if (fulldata_lba + size <= map_data.limit && menus_server_loading_active) {
        memmove(map_data.block + fulldata_lba, buffer, size);
        fulldata_lba += size;
        if (fulldata_lba >= map_data.limit) fulldata_lba = 0;
      } else {
        for (int i = 0; i < 65536 - fulldata_strm.avail_out; i++) {
          if (fulldata_lba >= map_data.limit) fulldata_lba = 0;
          if (buffer[i] != 0) {
            int x = fulldata_lba % map_data.dimension.x;
            int y = (fulldata_lba / map_data.dimension.x) % map_data.dimension.y;
            int z = fulldata_lba / (map_data.dimension.x * map_data.dimension.y);
            map_modify((struct int_xyz) {x, y, z}, buffer[i], 3);
          };
          fulldata_lba++;
        };
      };
      if (result == Z_STREAM_END) {
        map_invalidate_chunks();
        inflateInit2(&fulldata_strm, 47);
      };
      static int total_map_bytes = 0;
      total_map_bytes += size;
      //printf("received %d map bytes (%d out of %d)\n", size, total_map_bytes, map_data.limit);
    };
    lag_pop();
  } else if (packet_type == PACKET_MAP_FILL) {
    if (packet_size < 12) {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    } else {
      if (argument_packets && packet_size > 13) printf("\e[1;33m%s: Packet contains more data than I expect! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
      lag_push(10, "receiving a PACKET_MAP_FILL");
      int x1 = DATA(0, short);
      int y1 = DATA(2, short);
      int z1 = DATA(4, short);
      int x2 = DATA(6, short);
      int y2 = DATA(8, short);
      int z2 = DATA(10, short);
      int type = DATA(12, char);
      if (x2 < x1 || y2 < y1 || z2 < z1) {
        if (argument_packets) printf("\e[1;31mPACKET_MAP_FILL: Invalid coordinates!\e[0m\n");
      } else if (type != 0) {
        DEBUG("enter map data update");
        for (int z = z1; z <= z2; z++) {
          for (int y = y1; y <= y2; y++) {
            for (int x = x1; x <= x2; x++) {
              map_modify((struct int_xyz) {x, y, z}, type, 1);
            };
          };
        };
        DEBUG("leave map data update");
      };
      lag_pop();
    };
  } else if (packet_type == PACKET_GROUP_SETUP) {
    if (packet_size < 1) {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    } else {
      int group = DATA(0, char);
      if (packet_size == 1) {
        DELETE_GROUP:
        group_data[group].size = 0;
        memory_allocate(&group_data[group].name, 0);
        memory_allocate(&group_data[group].list, 0);
      } else {
        char *pointer = packet_data + 1;
        int remaining = packet_size - 1;
        group_data[group].size = easy_strnlen(pointer, remaining);
        memory_allocate(&group_data[group].list, sizeof(int) * group_data[group].size);
        for (int i = 0; i < group_data[group].size; i++) {
          group_data[group].list[i] = *pointer;
          pointer++; remaining--;
        };
        if (remaining) pointer++, remaining--;
        int length = easy_strnlen(pointer, remaining);
        if (length > 0) {
          memory_allocate(&group_data[group].name, length + 1);
          memmove(group_data[group].name, pointer, length);
          group_data[group].name[length] = 0;
        } else {
          char name[64];
          sprintf(name, "Group %d", group);
          memory_allocate(&group_data[group].name, strlen(name) + 1);
          strcpy(group_data[group].name, name);
        };
      };
    };
  } else if (packet_type == PACKET_TEXTURE_SETUP) {
    if (packet_size >= 2) {
      if (DATA(0, short) < 0 || DATA(0, short) >= TEXTURE_MAX_SERVER_TEXTURES) {
        if (argument_packets) printf("\e[1;31mPACKET_TEXTURE_SETUP: Invalid texture number! (texture number %d)\e[0m\n", DATA(0, short));
        return;
      };
    };
    if (packet_size == 2) {
      int texture = DATA(0, short);
      if (texture_data[texture].name != 0) {
        glDeleteTextures(1, &texture_data[texture].name);
        texture_data[texture].name = 0;
      };
      if (texture_data[texture].file != NULL) {
        memory_allocate(&texture_data[texture].file, 0);
      };
      texture_data[texture].alpha = 0.0;
      texture_data[texture].x_scale = -1;
      texture_data[texture].y_scale = -1;
      map_invalidate_chunks();
    } else if (packet_size >= 38) {
      if (argument_packets && packet_size > 38) printf("\e[1;33m%s: Packet contains more data than I expect! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
      int texture = DATA(0, short);
      char ascii[41]; ascii[40] = 0;
      easy_binary_to_ascii(ascii, &DATA(2, char), 20);
      memory_allocate(&texture_data[texture].file, 40 + 6 + 1);
      sprintf(texture_data[texture].file, "cache/%s", ascii);
      texture_data[texture].x_scale = 1.0 / DATA(22, float);
      texture_data[texture].y_scale = 1.0 / DATA(26, float);
      texture_data[texture].alpha = DATA(30, float);
      int flags = DATA(34, unsigned int);
      texture_data[texture].flags = 0;
      if (argument_packets) printf("Texture data: hash=%s x_scale=%0.3f y_scale=%0.3f alpha=%0.3f flags=%d\n", ascii, texture_data[texture].x_scale, texture_data[texture].y_scale, texture_data[texture].alpha, texture_data[texture].flags);
      if (flags & 0x01) texture_data[texture].flags |= TEXTURE_FLAG_MIPMAP;
      if (flags & 0x02) texture_data[texture].flags |= TEXTURE_FLAG_PIXELATE;
      if (cache_validate(&DATA(2, char))) {
        texture_list(texture, cache_data, cache_size, texture_data[texture].flags);
        memory_allocate(&cache_data, 0);
      } else {
        extern char data_loading_png; extern int size_loading_png;
        texture_list(texture, &data_loading_png, size_loading_png, 1);
        memory_allocate(&texture_data[texture].digest, 20);
        memmove(texture_data[texture].digest, &DATA(2, char), 20);
        packet_send(PACKET_FILE_REQUEST, &DATA(2, char));
      };
      map_invalidate_chunks();
    } else if (packet_size >= 22) {
      if (argument_packets && packet_size > 22) printf("\e[1;33m%s: Packet contains an unusual amount of data! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
      int texture = DATA(0, short);
      char ascii[41]; ascii[40] = 0;
      easy_binary_to_ascii(ascii, &DATA(2, char), 20);
      memory_allocate(&texture_data[texture].file, 40 + 6 + 1);
      sprintf(texture_data[texture].file, "cache/%s", ascii);
      if (cache_validate(&DATA(2, char))) {
        texture_list(texture, cache_data, cache_size, texture_data[texture].flags);
        memory_allocate(&cache_data, 0);
      } else {
        extern char data_loading_png; extern int size_loading_png;
        texture_list(texture, &data_loading_png, size_loading_png, 1);
        memory_allocate(&texture_data[texture].digest, 20);
        memmove(texture_data[texture].digest, &DATA(2, char), 20);
        packet_send(PACKET_FILE_REQUEST, &DATA(2, char));
      };
      map_invalidate_chunks();
    } else {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    };
  } else if (packet_type == PACKET_BLOCK_SETUP) {
    if (packet_size >= 1) {
      if (DATA(0, char) == 0) {
        if (argument_packets) printf("\e[1;31mPACKET_BLOCK_SETUP: Zero is not a valid block number!\e[0m\n");
        return;
      };
    };
    if (packet_size == 1) {
      int block = DATA(0, char);
      memory_allocate(&block_data[block].comment, 0);
      block_data[block].visible = 1;
      block_data[block].reverse = 0;
      block_data[block].between = 0;
      block_data[block].impassable = 1;
      block_data[block].transparent = 0;
      block_data[block].emission = 0;
      block_data[block].index[BLOCK_SIDE_UP] = TEXTURE_SWEET_U;
      block_data[block].index[BLOCK_SIDE_FRONT] = TEXTURE_SWEET_S;
      block_data[block].index[BLOCK_SIDE_RIGHT] = TEXTURE_SWEET_S;
      block_data[block].index[BLOCK_SIDE_LEFT] = TEXTURE_SWEET_S;
      block_data[block].index[BLOCK_SIDE_BACK] = TEXTURE_SWEET_S;
      block_data[block].index[BLOCK_SIDE_DOWN] = TEXTURE_SWEET_D;
      map_invalidate_chunks();
    } else if (packet_size >= 17) {
      if (DATA(13, unsigned int) & 0x00000001) {
        for (int i = 0; i < 6; i++) {
          if (DATA(1 + 2 * i, unsigned short) > TEXTURE_MAX_SERVER_TEXTURES) {
            if (argument_packets) printf("\e[1;31mPACKET_BLOCK_SETUP: Texture number must be less than %d!\e[0m\n", TEXTURE_MAX_SERVER_TEXTURES);
            return;
          };
        };
      };
      int block = DATA(0, char);
      int flags = DATA(13, unsigned int);
      block_data[block].visible = 0 != (flags & 0x00000001);
      if (block_data[block].visible) {
        block_data[block].index[BLOCK_SIDE_UP] = DATA(1, unsigned short);
        block_data[block].index[BLOCK_SIDE_DOWN] = DATA(3, unsigned short);
        block_data[block].index[BLOCK_SIDE_FRONT] = DATA(5, unsigned short);
        block_data[block].index[BLOCK_SIDE_BACK] = DATA(7, unsigned short);
        block_data[block].index[BLOCK_SIDE_LEFT] = DATA(9, unsigned short);
        block_data[block].index[BLOCK_SIDE_RIGHT] = DATA(11, unsigned short);
        block_data[block].transparent = 0;
        for (int i = 0; i < 6; i++) {
          if (block_data[block].index[i] > TEXTURE_MAX_SERVER_TEXTURES) block_data[block].index[i] = 0;
          if (texture_data[block_data[block].index[i]].alpha != 0.0) block_data[block].transparent = 1;
        };
      } else {
        block_data[block].index[BLOCK_SIDE_UP] = TEXTURE_SWEET_U;
        block_data[block].index[BLOCK_SIDE_FRONT] = TEXTURE_SWEET_S;
        block_data[block].index[BLOCK_SIDE_RIGHT] = TEXTURE_SWEET_S;
        block_data[block].index[BLOCK_SIDE_LEFT] = TEXTURE_SWEET_S;
        block_data[block].index[BLOCK_SIDE_BACK] = TEXTURE_SWEET_S;
        block_data[block].index[BLOCK_SIDE_DOWN] = TEXTURE_SWEET_D;
        block_data[block].transparent = 1;
      };
      block_data[block].impassable = 0 != (flags & 0x00000002);
      block_data[block].reverse = 0 != (flags & 0x00000004);
      block_data[block].between = 0 != (flags & 0x00000020);
      block_data[block].emission = (flags >> 3) & 3;
      int size = packet_size - 17;
      memory_allocate(&block_data[block].comment, size + 1);
      memmove(block_data[block].comment, &DATA(17, char), size);
      block_data[block].comment[size] = 0;
      map_invalidate_chunks();
      #ifdef TEST
        if (block == 134) block_data[block].between = 1;
      #endif
    } else {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    };
  } else if (packet_type == PACKET_MENU_RESET) {
    if (packet_size == 0) {
      menus_server_menu_reset();
    } else if (packet_size >= 3) {
      if (argument_packets && packet_size > 4) printf("\e[1;33m%s: Packet contains more data than I expect! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
      menus_server_menu_reset();
      menus_server_menu_active = 1;
      menus_server_menu_number = DATA(0, char);
      menus_server_menu_width = DATA(1, char);
      menus_server_menu_height = DATA(2, char);
      if (packet_size >= 4) {
        menus_server_menu_options = DATA(3, char);
      } else {
        menus_server_menu_options = 0;
      };
    } else {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    };
  } else if (packet_type == PACKET_MENU_TEXT) {
    if (packet_size >= 3) {
      int i = DATA(0, char);
      menus_server_menu_data[i].type = MENU_TEXT;
      menus_server_menu_data[i].column = DATA(1, char);
      menus_server_menu_data[i].line = DATA(2, char);
      if (menus_server_menu_data[i].column == 255) menus_server_menu_data[i].column = -1;
      int length = packet_size - 3;
      memory_allocate(&menus_server_menu_data[i].text, length + 1);
      memmove(menus_server_menu_data[i].text, &DATA(3, char), length);
      menus_server_menu_data[i].text[length] = 0;
    } else {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    };
  } else if (packet_type == PACKET_MENU_LINK) {
    if (packet_size >= 6) {
      int i = DATA(0, char);
      menus_server_menu_data[i].type = MENU_LINK;
      menus_server_menu_data[i].column = DATA(1, char);
      menus_server_menu_data[i].line = DATA(2, char);
      menus_server_menu_data[i].name = DATA(3, char);
      menus_server_menu_data[i].value = DATA(4, char);
      menus_server_menu_data[i].flags = DATA(5, char);
      if (menus_server_menu_data[i].column == 255) menus_server_menu_data[i].column = -1;
      int length = packet_size - 6;
      memory_allocate(&menus_server_menu_data[i].text, length + 1);
      memmove(menus_server_menu_data[i].text, &DATA(6, char), length);
      menus_server_menu_data[i].text[length] = 0;
    } else {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    };
  } else if (packet_type == PACKET_MENU_CHECK) {
    if (packet_size >= 6) {
      int i = DATA(0, char);
      menus_server_menu_data[i].type = MENU_CHECK;
      menus_server_menu_data[i].column = DATA(1, char) + 2;
      menus_server_menu_data[i].line = DATA(2, char);
      menus_server_menu_data[i].name = DATA(3, char);
      menus_server_menu_data[i].value = DATA(4, char);
      menus_server_menu_data[i].flags = DATA(5, char);
      int length = packet_size - 6;
      memory_allocate(&menus_server_menu_data[i].text, length + 1);
      memmove(menus_server_menu_data[i].text, &DATA(6, char), length);
      menus_server_menu_data[i].text[length] = 0;
    } else {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    };
  } else if (packet_type == PACKET_MENU_RADIO) {
    if (packet_size >= 6) {
      int i = DATA(0, char);
      menus_server_menu_data[i].type = MENU_RADIO;
      menus_server_menu_data[i].column = DATA(1, char) + 2;
      menus_server_menu_data[i].line = DATA(2, char);
      menus_server_menu_data[i].name = DATA(3, char);
      menus_server_menu_data[i].value = DATA(4, char);
      menus_server_menu_data[i].flags = DATA(5, char);
      if (menus_server_menu_data[i].flags & MENU_FLAG_ACTIVE) {
        menus_server_menu_value[menus_server_menu_data[i].name] = menus_server_menu_data[i].value;
      };
      int length = packet_size - 6;
      memory_allocate(&menus_server_menu_data[i].text, length + 1);
      memmove(menus_server_menu_data[i].text, &DATA(6, char), length);
      menus_server_menu_data[i].text[length] = 0;
    } else {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    };
  } else if (packet_type == PACKET_MENU_INPUT) {
    if (packet_size >= 8) {
      int i = DATA(0, char);
      menus_server_menu_data[i].type = MENU_INPUT;
      menus_server_menu_data[i].column = DATA(1, char) + 1;
      menus_server_menu_data[i].line = DATA(2, char);
      menus_server_menu_data[i].name = DATA(3, char);
      menus_server_menu_data[i].value = DATA(4, char);
      menus_server_menu_data[i].width = DATA(5, char);
      menus_server_menu_data[i].length = DATA(6, char);
      menus_server_menu_data[i].flags = DATA(7, char);
      int length = packet_size - 8;
      if (length > menus_server_menu_data[i].length) length = menus_server_menu_data[i].length;
      memory_allocate(&menus_server_menu_data[i].text, menus_server_menu_data[i].length + 1);
      memmove(menus_server_menu_data[i].text, &DATA(8, char), length);
      menus_server_menu_data[i].text[length] = 0;
    } else {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    };
  } else if (packet_type == PACKET_MENU_PASSWORD) {
    if (packet_size >= 7) {
      int i = DATA(0, char);
      menus_server_menu_data[i].type = MENU_PASSWORD;
      menus_server_menu_data[i].column = DATA(1, char) + 1;
      menus_server_menu_data[i].line = DATA(2, char);
      menus_server_menu_data[i].name = DATA(3, char);
      menus_server_menu_data[i].value = DATA(4, char);
      menus_server_menu_data[i].width = DATA(5, char);
      menus_server_menu_data[i].length = 255;
      menus_server_menu_data[i].flags = DATA(6, char) | MENU_FLAG_STARS;
      menus_server_menu_data[i].salt_count = 0;
      if (packet_size >= 27) {
        menus_server_menu_data[i].salt_count = 1;
        memmove(menus_server_menu_data[i].salt_one, &DATA(7, char), 20);
      };
      if (packet_size >= 47) {
        menus_server_menu_data[i].salt_count = 2;
        memmove(menus_server_menu_data[i].salt_two, &DATA(27, char), 20);
      };
      int length = 0;
      memory_allocate(&menus_server_menu_data[i].text, menus_server_menu_data[i].length + 1);
      menus_server_menu_data[i].text[length] = 0;
    } else {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    };
  } else if (packet_type == PACKET_MENU_BUTTON) {
    if (packet_size >= 7) {
      int i = DATA(0, char);
      menus_server_menu_data[i].type = MENU_BUTTON;
      menus_server_menu_data[i].column = DATA(1, char) + 1;
      menus_server_menu_data[i].line = DATA(2, char);
      menus_server_menu_data[i].name = DATA(3, char);
      menus_server_menu_data[i].value = DATA(4, char);
      menus_server_menu_data[i].width = DATA(5, char);
      menus_server_menu_data[i].flags = DATA(6, char);
      if (menus_server_menu_data[i].column == 256) menus_server_menu_data[i].column = -1;
      int length = packet_size - 7;
      memory_allocate(&menus_server_menu_data[i].text, length + 1);
      memmove(menus_server_menu_data[i].text, &DATA(7, char), length);
      menus_server_menu_data[i].text[length] = 0;
    } else {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    };
  } else if (packet_type == PACKET_SKY_BOX) {
    if (packet_size == 0) {
      sky_box_type = 0;
    } else if (packet_size == 1) {
      sky_box_flags = DATA(0, char);
    } else if (packet_size >= 2 && packet_size <= 5) {
      sky_box_type = packet_size >> 1;
      sky_box_texture[0] = DATA(0, unsigned short);
      if (packet_size >= 4) {
        sky_box_texture[1] = DATA(2, unsigned short);
      };
      if (packet_size & 1) sky_box_flags = DATA(packet_size - 1, char);
    } else if (packet_size >= 10 || packet_size <= 13) {
      sky_box_type = packet_size >> 1;
      int o = 0;
      sky_box_texture[0] = DATA(2 * o++, unsigned short);
      sky_box_texture[1] = DATA(2 * o++, unsigned short);
      if (packet_size >= 12) {
        sky_box_texture[2] = DATA(2 * o++, unsigned short);
      };
      sky_box_texture[3] = DATA(2 * o++, unsigned short);
      sky_box_texture[4] = DATA(2 * o++, unsigned short);
      sky_box_texture[5] = DATA(2 * o++, unsigned short);
      if (packet_size & 1) sky_box_flags = DATA(packet_size - 1, char);
    } else {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    };
  } else if (packet_type == PACKET_PLAYER_SKIN) {
    if (packet_size == 1) {
      int model_id = DATA(0, char);
      model_player[model_id].texture = -1;
    } else if (packet_size >= 3) {
      if (argument_packets && packet_size > 3) printf("\e[1;33m%s: Packet contains more data than I expect! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
      int model_id = DATA(0, char);
      int texture = DATA(1, unsigned short);
      if (texture < TEXTURE_MAX_SERVER_TEXTURES) {
        model_player[model_id].texture = texture;
      } else {
        if (argument_packets) printf("\e[1;31mPACKET_PLAYER_SKIN: Invalid texture number! (texture number %d)\e[0m\n", texture);
        model_player[model_id].texture = -1;
      };
    } else {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    };
  } else if (packet_type == PACKET_MENU_TEXTURE) {
    if (packet_size >= 11) {
      if (argument_packets && packet_size > 11) printf("\e[1;33m%s: Packet contains more data than I expect! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
      int i = DATA(0, char);
      menus_server_menu_data[i].type = MENU_TEXTURE;
      menus_server_menu_data[i].column = DATA(1, char);
      menus_server_menu_data[i].line = DATA(2, char);
      menus_server_menu_data[i].width = DATA(3, char);
      menus_server_menu_data[i].height = DATA(4, char);
      if (DATA(5, unsigned short) < TEXTURE_MAX_SERVER_TEXTURES) {
        menus_server_menu_data[i].texture = DATA(5, unsigned short);
      } else {
        menus_server_menu_data[i].texture = TEXTURE_INVALID;
        if (argument_packets) printf("\e[1;31mPACKET_MENU_TEXTURE: Invalid texture number! (texture number %d)\e[0m\n", DATA(5, unsigned short));
      };
      menus_server_menu_data[i].coordinate[0] = DATA(7, char);
      menus_server_menu_data[i].coordinate[1] = DATA(8, char);
      menus_server_menu_data[i].coordinate[2] = DATA(9, char);
      menus_server_menu_data[i].coordinate[3] = DATA(10, char);
      menus_server_menu_data[i].flags = 0;
    } else {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    };
  } else if (packet_type == PACKET_SERVER_NAME) {
    char *string = NULL;
    memory_allocate(&string, packet_size + 1);
    memmove(string, &DATA(0, char), packet_size);
    string[packet_size] = 0;
    glfw_set_window_title(string);
    memory_allocate(&string, 0);
  } else if (packet_type == PACKET_PROJECTILE_CREATE) {
    if (packet_size != 21) {
      if (argument_packets) printf("\e[1;31m%s: Invalid packet size! (payload is %d bytes)\e[0m\n", packet_type_to_string(packet_type), packet_size);
    } else {
      unsigned char type = DATA(0, unsigned char);
      double x = DATA(1, float);
      double y = DATA(5, float);
      double z = DATA(9, float);
      double u = DATA(13, float);
      double v = DATA(17, float);

      projectile_create(1, type, (struct double_xyzuv){x, y, z, u, v});
    };

  } else {
    printf("Write some code for packet type %s!\n", packet_type_to_string(packet_type));
    easy_fuck("For some reason I requested a packet type I don't know how to read!");
  };

};

//--page-split-- server_modify

void server_modify(struct int_xyz block, int type) {
  if (map_data.wrap.x) {
    if (block.x < 0) block.x += map_data.dimension.x;
    if (block.x >= map_data.dimension.x) block.x -= map_data.dimension.x;
  };
  if (map_data.wrap.y) {
    if (block.y < 0) block.y += map_data.dimension.y;
    if (block.y >= map_data.dimension.y) block.y -= map_data.dimension.y;
  };
  if (map_data.wrap.z) {
    if (block.z < 0) block.z += map_data.dimension.z;
    if (block.z >= map_data.dimension.z) block.z -= map_data.dimension.z;
  };
  if (block.x < 0 || block.x >= map_data.dimension.x) return;
  if (block.y < 0 || block.y >= map_data.dimension.y) return;
  if (block.z < 0 || block.z >= map_data.dimension.z) return;
  packet_send(PACKET_MAP_MODIFY, block.x, block.y, block.z, type, 0) ||
  packet_send(PACKET_MAP_FILL, block.x, block.y, block.z, block.x, block.y, block.z, type, 0);
};

//--page-split-- server_modify_bunch

void server_modify_bunch(struct int_xyz one, struct int_xyz two, int type) {

  if (one.x == two.x && one.y == two.y && one.z == two.z) {
    server_modify(one, type);
    return;
  };

  struct double_xyzuv pp = player_view;
  if (map_data.wrap.x && pp.x >= map_data.dimension.x / 2) pp.x -= map_data.dimension.x;
  if (map_data.wrap.y && pp.y >= map_data.dimension.y / 2) pp.y -= map_data.dimension.y;
  if (map_data.wrap.z && pp.z >= map_data.dimension.z / 2) pp.z -= map_data.dimension.z;

  // Arrange second point (which is always near the player) so it is near the player.
  if (map_data.wrap.x && two.x - pp.x >= map_data.dimension.x / 2) two.x -= map_data.dimension.x;
  if (map_data.wrap.y && two.y - pp.y >= map_data.dimension.y / 2) two.y -= map_data.dimension.y;
  if (map_data.wrap.z && two.z - pp.z >= map_data.dimension.z / 2) two.z -= map_data.dimension.z;

  // Wrap the "one" coordinates to come before the "two" coordinates.
  while (map_data.wrap.x && one.x > two.x) one.x -= map_data.dimension.x;
  while (map_data.wrap.y && one.y > two.y) one.y -= map_data.dimension.y;
  while (map_data.wrap.z && one.z > two.z) one.z -= map_data.dimension.z;

  // If resulting box is too large, unwrap the coordinates just once.
  if (map_data.wrap.x && two.x - one.x >= map_data.dimension.x / 2) one.x += map_data.dimension.x;
  if (map_data.wrap.y && two.y - one.y >= map_data.dimension.y / 2) one.y += map_data.dimension.y;
  if (map_data.wrap.z && two.z - one.z >= map_data.dimension.z / 2) one.z += map_data.dimension.z;

  // Then swap the coordinates so that "one" comes before "two".
  if (one.x > two.x) { int temp = one.x; one.x = two.x; two.x = temp; };
  if (one.y > two.y) { int temp = one.y; one.y = two.y; two.y = temp; };
  if (one.z > two.z) { int temp = one.z; one.z = two.z; two.z = temp; };

  if (!packet_send(PACKET_MAP_FILL, one.x, one.y, one.z, two.x, two.y, two.z, type, 0)) {
    struct int_xyz i;
    for (i.z = one.z; i.z <= two.z; i.z++) {
      for (i.y = one.y; i.y <= two.y; i.y++) {
        for (i.x = one.x; i.x <= two.x; i.x++) {
          server_modify(i, type);
        };
      };
    };
  };

};

//--page-split-- server_paste

int server_paste(struct int_xyz lower_corner, struct int_xyz dimension, char *data) {

  struct int_xyz output_dimension = dimension;
  struct int_xyz output_lower_corner = lower_corner;
  struct int_xyz output_upper_corner;

  output_upper_corner.x = lower_corner.x + dimension.x - 1;
  output_upper_corner.y = lower_corner.y + dimension.y - 1;
  output_upper_corner.z = lower_corner.z + dimension.z - 1;

  if (output_lower_corner.x < 0) {
    output_dimension.x -= 0 - output_lower_corner.x;
    output_lower_corner.x = 0;
  };
  if (output_lower_corner.y < 0) {
    output_dimension.y -= 0 - output_lower_corner.y;
    output_lower_corner.y = 0;
  };
  if (output_lower_corner.z < 0) {
    output_dimension.z -= 0 - output_lower_corner.z;
    output_lower_corner.z = 0;
  };
  if (output_upper_corner.x > map_data.dimension.x - 1) {
    output_dimension.x -= output_upper_corner.x - (map_data.dimension.x - 1);
    output_upper_corner.x = map_data.dimension.x - 1;
  };
  if (output_upper_corner.y > map_data.dimension.y - 1) {
    output_dimension.y -= output_upper_corner.y - (map_data.dimension.y - 1);
    output_upper_corner.y = map_data.dimension.y - 1;
  };
  if (output_upper_corner.z > map_data.dimension.z - 1) {
    output_dimension.z -= output_upper_corner.z - (map_data.dimension.z - 1);
    output_upper_corner.z = map_data.dimension.z - 1;
  };

  // If the whole thing is outside the map, nothing needs to be done.
  if (output_upper_corner.x < output_lower_corner.x) return 1;
  if (output_upper_corner.y < output_lower_corner.y) return 1;
  if (output_upper_corner.z < output_lower_corner.z) return 1;

  int output_size = output_dimension.x * output_dimension.y * output_dimension.z;

  char *output = NULL;
  memory_allocate(&output, output_size);

  char *pointer = output;
  for (int z = 0; z < output_dimension.z; z++) {
    for (int y = 0; y < output_dimension.y; y++) {
      for (int x = 0; x < output_dimension.x; x++) {
        int zi = output_lower_corner.z - lower_corner.z + z;
        int yi = output_lower_corner.y - lower_corner.y + y;
        int xi = output_lower_corner.x - lower_corner.x + x;
        int o = (zi * dimension.y + yi) * dimension.x + xi;
        *pointer++ = data[o];
      };
    };
  };

  int can_buffer = packet_is_sendable(PACKET_BUFFER_RESET) && packet_is_sendable(PACKET_BUFFER_APPEND) && packet_is_sendable(PACKET_BUFFER_IS_MAP_DATA);
  int can_compress = packet_is_sendable(PACKET_MAP_DATA);

  int paste_successful = 0;

  can_buffer = 0; can_compress = 0;

  #ifndef TEST

  #endif

  if (can_buffer || can_compress) {
    char *compressed_data = NULL;
    memory_allocate(&compressed_data, 65536);
    z_stream stream = {};
    stream.next_in = output;
    stream.avail_in = output_dimension.x * output_dimension.y * output_dimension.z;
    stream.next_out = compressed_data;
    stream.avail_out = 65535;
    deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15, 9, Z_DEFAULT_STRATEGY);
    int result = deflate(&stream, Z_FINISH);
    if (result == Z_STREAM_END && 65535 - stream.avail_out < 65535 - 12) {
      int data_size = 65535 - stream.avail_out;
      printf("Pasting via PACKET_MAP_DATA (data size is %d bytes)...\n", data_size);
      packet_send(PACKET_MAP_DATA, output_lower_corner, output_upper_corner, data_size, compressed_data);
      paste_successful = 1;
    } else {
      printf("I don't know how to properly send such a large paste!\n");
      printf("result = %d, size = %d\n", result, 65535 - stream.avail_out);
    };
    deflateEnd(&stream);
    memory_allocate(&compressed_data, 0);
  };

  // Send one block at a time, if previous attempt failed.
  if (!paste_successful) {
    struct int_xyz c;
    char *block = output;
    for (c.z = output_lower_corner.z; c.z <= output_upper_corner.z; c.z++) {
      for (c.y = output_lower_corner.y; c.y <= output_upper_corner.y; c.y++) {
        for (c.x = output_lower_corner.x; c.x <= output_upper_corner.x; c.x++) {
          if (*block != 0) {
            server_modify(c, *block);
          };
          block++;
        };
      };
    };
    paste_successful = 1;
  };

  memory_allocate(&output, 0);

  return paste_successful;

};
