#include "everything.h"

// Packet code, intended for distribution!

#define display_packet_dumps argument_packets
#define process_packet server_process_packet
#define send_raw_data server_send_raw_data

const char *packet_type_string[] = {"PACKET_AVAILABLE_PACKET_TYPES", "PACKET_SELECT_PACKET_TYPES", "PACKET_PING_REQUEST", "PACKET_PING_RESPONSE", "PACKET_IDLE_PING", "PACKET_AUTHENTICATE", "PACKET_CONNECT", "PACKET_DISCONNECT", "PACKET_CHAT_MESSAGE", "PACKET_ANNOUNCE_PLAYER", "PACKET_DENOUNCE_PLAYER", "PACKET_PLAYER_ABILITIES", "PACKET_MAP_LOADING", "PACKET_RANDOM_GARBAGE", "PACKET_MAP_SETUP", "PACKET_MAP_PROPERTIES", "PACKET_MAP_FILL", "PACKET_MAP_DATA", "PACKET_MAP_TOTAL_RESET", "PACKET_MAP_TOTAL_DATA", "PACKET_BUFFER_RESET", "PACKET_BUFFER_APPEND", "PACKET_BUFFER_IS_MAP_DATA", "PACKET_BUFFER_IS_FILE_DATA", "PACKET_MOVE_PLAYER", "PACKET_MAP_MODIFY", "PACKET_FILE_REQUEST", "PACKET_TEXTURE_SETUP", "PACKET_GROUP_SETUP", "PACKET_BLOCK_SETUP", "PACKET_MENU_REQUEST", "PACKET_MENU_RESPONSE", "PACKET_MENU_RESET", "PACKET_MENU_TEXT", "PACKET_MENU_LINK", "PACKET_MENU_CHECK", "PACKET_MENU_RADIO", "PACKET_MENU_INPUT", "PACKET_MENU_BUTTON", "PACKET_MENU_NULL", "PACKET_MENU_PASSWORD", "PACKET_MENU_RESIZE", "PACKET_SKY_BOX", "PACKET_CLIENT_VERSION", "PACKET_PORTAL_COOKIE", "PACKET_PLAYER_SKIN", "PACKET_MENU_TEXTURE", "PACKET_SERVER_NAME", "PACKET_PROJECTILE_CREATE", "PACKET_PROJECTILE_COLLISION"};

static char available_packet_types[8192];
static char requested_packet_types[8192];
static char our_packet_selections[8192];
static int communication_error_flag;

static char *dynamic_error_message;

//--page-split-- packet_reset

void packet_reset() {
  communication_error_flag = 0;
  // Clear packet selection bit vectors...
  memset(available_packet_types, 0, 8192);
  memset(requested_packet_types, 0, 8192);
  memset(our_packet_selections, 0, 8192);
  // Set PACKET_AVAILABLE_PACKET_TYPES and PACKET_SELECT_PACKET_TYPES...
  available_packet_types[0] = 0x03;
  requested_packet_types[0] = 0x03;
  our_packet_selections[0] = 0x03;
};

//--page-split-- packet_type_to_string

const char *packet_type_to_string(int packet_type) {
  static char unknown[64];
  if (packet_type >= 0 && packet_type < PACKET_TYPE_LIMIT) {
    return packet_type_string[packet_type];
  } else {
    sprintf(unknown, "[Unknown Packet Type %d]", packet_type);
    return unknown;
  };
};

//--page-split-- available

static int available(int packet_type) {
  int index = packet_type / 8;
  int bit = 1 << (packet_type % 8);
  return (available_packet_types[index] & bit) != 0;
};

//--page-split-- choose

static int choose(int packet_type) {
  int index = packet_type / 8;
  int bit = 1 << (packet_type % 8);
  if (available_packet_types[index] & bit) {
    our_packet_selections[index] |= bit;
    if (display_packet_dumps) printf("Choosing %s\n", packet_type_to_string(packet_type));
    return 1;
  } else {
    if (display_packet_dumps) printf("%s is unavailable...\n", packet_type_to_string(packet_type));
    return 0;
  };
};

static const int sendable[] = {

  // List of packet types this code sends on its own.

  PACKET_AVAILABLE_PACKET_TYPES,
  PACKET_SELECT_PACKET_TYPES,
  PACKET_PING_RESPONSE,

  //  List of the packet types your project is willing to send through
  //  the packet_send() function, if the remote system selects them.
  //  Adjust this list to match your project's abilities.

  PACKET_PROJECTILE_COLLISION,
  PACKET_PROJECTILE_CREATE,
  PACKET_MENU_REQUEST,
  PACKET_MENU_RESIZE,
  PACKET_MENU_RESPONSE,
  PACKET_FILE_REQUEST,
  PACKET_PING_REQUEST,
  PACKET_AUTHENTICATE,
  PACKET_MAP_MODIFY,
  PACKET_MAP_FILL,
  PACKET_MAP_DATA,
  PACKET_BUFFER_RESET,
  PACKET_BUFFER_APPEND,
  PACKET_BUFFER_IS_MAP_DATA,
  PACKET_MOVE_PLAYER,
  PACKET_CHAT_MESSAGE,
  PACKET_DISCONNECT,
  PACKET_CLIENT_VERSION,
  PACKET_PORTAL_COOKIE,
  PACKET_IDLE_PING

};

//--page-split-- packet_is_sendable

int packet_is_sendable(int packet_type) {

  //  This function returns true if a packet type is accepted by the server, or
  //  false if it is not.  It is not necessary to use this function, since the
  //  packet_send() function below does a similar check, and so you can typically
  //  just do this:
  //
  //  packet_send(PACKET_THIS) || packet_send(PACKET_THAT) || whatever();
  //
  //  However, in some cases it is desireable to know beforehand whether or not a
  //  particular packet type will be sent, and that is what this function is for.

  if (packet_type < 0 || packet_type >= PACKET_TYPE_LIMIT) return 0;
  int index = packet_type / 8;
  int bit = 1 << (packet_type % 8);
  return (requested_packet_types[index] & bit) != 0;
};

//--page-split-- send_packet

static void send_packet(char *packet, int size) {
  if (size == 0) return;
  if (display_packet_dumps) {
    // Must always be sent a full header, but
    // can deal with fragmented payloads.
    static int missing = 0;
    if (missing == 0) {
      int packet_type = *((unsigned short*) (packet + 0));
      missing = *((unsigned short*) (packet + 2)) + 4;
      missing -= size;
      printf("\e[1;32mSending packet type %s\e[0m\n", packet_type_to_string(packet_type));
      printf("Packet dump:");
    } else {
      missing -= size;
    };
    for (int i = 0; i < size; i++) {
      printf(" %02x", (unsigned char) packet[i]);
    };
    if (missing <= 0) printf("\n");
    send_raw_data(packet, size);
    if (missing < 0) easy_fuck("I seem to have sent an incorrect packet!");
  } else {
    send_raw_data(packet, size);
  };
};

//--page-split-- packet_send

int packet_send(int packet_type, ...) {

  //  This function sends all packet types.  Arguments vary by packet type.
  //  Packets will not be sent if the packet type was not requested.
  //  Return value is 1 if the packet was sent or 0 if it was not sent.
  //  So you can choose which type of packet to send like this:
  //  packet_send(PACKET_THIS, data, data) || packet_send(PACKET_THAT, data);

  if (packet_type < 0 || packet_type > PACKET_TYPE_LIMIT) {
    printf("packet_send(): Packet type %d is out of range!\n", packet_type);
    easy_fuck("I tried to send an invalid packet!");
  };

  va_list ooo;
  va_start(ooo, packet_type);

  int index = packet_type / 8;
  int bit = 1 << (packet_type % 8);
  if (!(requested_packet_types[index] & bit)) {
    if (display_packet_dumps) printf("\e[1;31mServer does not want packet type %s\e[0m\n", packet_type_to_string(packet_type));
    return 0;
  };

  char packet[4096];
  #define header_size 4
  #define data_size (*((unsigned short*) (packet + 2)))
  #define packet_size (header_size + data_size)
  #define packet_data (packet + header_size)

  *((unsigned short*) (packet + 0)) = packet_type;
  data_size = 0; // In case it isn't set below.

  #define DATA(OFFSET, TYPE) *((TYPE*) (packet_data + OFFSET))

  if (packet_type == PACKET_IDLE_PING) {

    // uses no parameters

    send_packet(packet, packet_size);

  } else if (packet_type == PACKET_AVAILABLE_PACKET_TYPES) {

    // uses no parameters, except "sendable" array from above

    unsigned char vector[8192];
    memset(vector, 0, 8192);
    for (int i = 0; i < sizeof(sendable) / sizeof(int); i++) {
      if (sendable[i] == PACKET_PORTAL_COOKIE && strcmp(server_address, PORTAL_ADDRESS)) continue;
      int index = sendable[i] / 8;
      int bit = 1 << (sendable[i] % 8);
      vector[index] |= bit;
    };
    for (data_size = 8192; data_size > 0; data_size--) {
      if (vector[data_size - 1] != 0) break;
    };
    send_packet(packet, header_size);
    send_packet(vector, data_size);

  } else if (packet_type == PACKET_SELECT_PACKET_TYPES) {

    // parameter is a pointer to bit vector of packet types

    unsigned char *vector = va_arg(ooo, char*);
    for (data_size = 8192; data_size > 0; data_size--) {
      if (vector[data_size - 1] != 0) break;
    };
    send_packet(packet, header_size);
    send_packet(vector, data_size);

  } else if (packet_type == PACKET_PING_REQUEST) {

    // parameters:  char *data, int size

    char *data_pointer = va_arg(ooo, char*);
    data_size = va_arg(ooo, int);
    send_packet(packet, header_size);
    send_packet(data_pointer, data_size);

  } else if (packet_type == PACKET_PING_RESPONSE) {

    // parameters:  char *data, int size

    char *data_pointer = va_arg(ooo, char*);
    data_size = va_arg(ooo, int);
    send_packet(packet, header_size);
    send_packet(data_pointer, data_size);

  } else if (packet_type == PACKET_MOVE_PLAYER) {

    // parameters: 5 floats, the values that go in packet
    // plus a zero, for the player ID number

    DATA(0, float) = va_arg(ooo, double);
    DATA(4, float) = va_arg(ooo, double);
    DATA(8, float) = va_arg(ooo, double);
    DATA(12, float) = va_arg(ooo, double);
    DATA(16, float) = va_arg(ooo, double);
    DATA(20, unsigned char) = va_arg(ooo, int);
    data_size = 20 + (DATA(20, unsigned char) != 0);
    send_packet(packet, packet_size);

  } else if (packet_type == PACKET_MAP_MODIFY) {

    // parameters: map coordinates, block type and player ID

    DATA(0, unsigned short) = va_arg(ooo, int);
    DATA(2, unsigned short) = va_arg(ooo, int);
    DATA(4, unsigned short) = va_arg(ooo, int);
    DATA(6, unsigned char) = va_arg(ooo, int);
    DATA(7, unsigned char) = va_arg(ooo, int);
    data_size = 7 + (DATA(7, unsigned char) != 0);
    send_packet(packet, packet_size);

  } else if (packet_type == PACKET_MAP_FILL) {

    // parameters: x1, y1, z1, x2, y2, z2, t

    DATA(0, short) = va_arg(ooo, int);
    DATA(2, short) = va_arg(ooo, int);
    DATA(4, short) = va_arg(ooo, int);
    DATA(6, short) = va_arg(ooo, int);
    DATA(8, short) = va_arg(ooo, int);
    DATA(10, short) = va_arg(ooo, int);
    DATA(12, unsigned char) = va_arg(ooo, int);
    data_size = 13;
    send_packet(packet, packet_size);

  } else if (packet_type == PACKET_CHAT_MESSAGE) {

    // parameters: int user_id, char *message
    // message must be null-terminated

    DATA(0, unsigned char) = va_arg(ooo, int);
    char *message = va_arg(ooo, char*);
    data_size = 1 + strlen(message);
    send_packet(packet, header_size + 1);
    send_packet(message, data_size - 1);

  } else if (packet_type == PACKET_AUTHENTICATE) {

    // Since data must come from login server, the parameter here is
    // just a pointer to the 52-byte data that goes in the packet.

    char *pointer = va_arg(ooo, char*);
    if (pointer != NULL) {
      memmove(packet_data, pointer, 52);
      data_size = 52;
    };
    send_packet(packet, packet_size);

  } else if (packet_type == PACKET_PORTAL_COOKIE) {

    // Since data must come from login server, the parameter here is
    // just a pointer to the 20-byte data that goes in the packet.

    char *pointer = va_arg(ooo, char*);
    if (pointer != NULL) {
      memmove(packet_data, pointer, 20);
      data_size = 20;
    };
    send_packet(packet, packet_size);

  } else if (packet_type == PACKET_CLIENT_VERSION) {

    // Accepts no parameters.

    DATA(0, unsigned char) = 0;
    DATA(1, unsigned int) = build_svn_revision;
    data_size = 5;
    send_packet(packet, packet_size);

  } else if (packet_type == PACKET_DISCONNECT) {

    // parameters: reason code, reconnect delay, message

    DATA(0, unsigned char) = va_arg(ooo, int);
    DATA(1, unsigned char) = va_arg(ooo, int);
    char *message = va_arg(ooo, char*);
    data_size = 2 + strlen(message);
    send_packet(packet, header_size + 2);
    send_packet(message, data_size - 2);

  } else if (packet_type == PACKET_FILE_REQUEST) {

    // parameters: binary sha-1 message digest

    data_size = 20;
    memmove(packet_data, va_arg(ooo, char*), 20);
    send_packet(packet, packet_size);

  } else if (packet_type == PACKET_MENU_REQUEST) {

    // parameters: width, height

    DATA(0, unsigned char) = va_arg(ooo, int);
    DATA(1, unsigned char) = va_arg(ooo, int);
    data_size = 2;

    send_packet(packet, packet_size);

  } else if (packet_type == PACKET_MENU_RESIZE) {

    // parameters: width, height, menu number

    DATA(0, unsigned char) = va_arg(ooo, int);
    DATA(1, unsigned char) = va_arg(ooo, int);
    DATA(2, unsigned char) = va_arg(ooo, int);
    data_size = 3;

    send_packet(packet, packet_size);

  } else if (packet_type == PACKET_MENU_RESPONSE) {

    // parameters: name, value, flags, text
    // sends empty packet when !menus_server_menu_active

    if (menus_server_menu_active) {
      int flags;
      DATA(0, unsigned char) = menus_server_menu_number;
      DATA(1, unsigned char) = va_arg(ooo, int);
      DATA(2, unsigned char) = va_arg(ooo, int);
      DATA(3, unsigned char) = flags = va_arg(ooo, int);
      if (flags & MENU_FLAG_STARS) {
        char *data = va_arg(ooo, char*);
        data_size = 44;
        send_packet(packet, packet_size - 40);
        send_packet(data, 40);
      } else {
        char *text = va_arg(ooo, char*);
        if (text == NULL) {
          data_size = 4;
          send_packet(packet, packet_size);
        } else {
          int length = strlen(text);
          data_size = 4 + length;
          send_packet(packet, packet_size - length);
          send_packet(text, length);
        };
      };
    } else {
      DATA(0, unsigned char) = menus_server_menu_number;
      data_size = 1;
      send_packet(packet, packet_size);
    };

  } else if (packet_type == PACKET_MAP_DATA) {

    struct int_xyz lc = va_arg(ooo, struct int_xyz);
    struct int_xyz uc = va_arg(ooo, struct int_xyz);
    int size = va_arg(ooo, int);
    char *pointer = va_arg(ooo, char *);
    DATA(0, unsigned short) = lc.x;
    DATA(2, unsigned short) = lc.y;
    DATA(4, unsigned short) = lc.z;
    DATA(6, unsigned short) = uc.x;
    DATA(8, unsigned short) = uc.y;
    DATA(10, unsigned short) = uc.z;
    data_size = 12 + size;
    send_packet(packet, header_size + 12);
    send_packet(pointer, size);

  } else if (packet_type == PACKET_BUFFER_RESET) {

    data_size = va_arg(ooo, int);
    char *pointer = va_arg(ooo, char *);
    send_packet(packet, header_size);
    send_packet(pointer, data_size);

  } else if (packet_type == PACKET_BUFFER_APPEND) {

    data_size = va_arg(ooo, int);
    char *pointer = va_arg(ooo, char *);
    send_packet(packet, header_size);
    send_packet(pointer, data_size);

  } else if (packet_type == PACKET_BUFFER_IS_MAP_DATA) {

    struct int_xyz lc = va_arg(ooo, struct int_xyz);
    struct int_xyz uc = va_arg(ooo, struct int_xyz);
    DATA(0, unsigned short) = lc.x;
    DATA(2, unsigned short) = lc.y;
    DATA(4, unsigned short) = lc.z;
    DATA(6, unsigned short) = uc.x;
    DATA(8, unsigned short) = uc.y;
    DATA(10, unsigned short) = uc.z;
    data_size = 12;
    send_packet(packet, header_size + 12);

  } else if (packet_type == PACKET_PROJECTILE_CREATE) {

    DATA(0, unsigned char) = va_arg(ooo, int);
    DATA(1, float) = va_arg(ooo, double);
    DATA(5, float) = va_arg(ooo, double);
    DATA(9, float) = va_arg(ooo, double);
    DATA(13, float) = va_arg(ooo, double);
    DATA(17, float) = va_arg(ooo, double);
    data_size = 21;
    send_packet(packet, packet_size);

  } else if (packet_type == PACKET_PROJECTILE_COLLISION) {
    DATA(0, unsigned char) = va_arg(ooo, int);
    DATA(1, float) = va_arg(ooo, double);
    DATA(5, float) = va_arg(ooo, double);
    DATA(9, float) = va_arg(ooo, double);
    DATA(13, unsigned char) = va_arg(ooo, int);
    DATA(14, unsigned char) = va_arg(ooo, int);
    data_size = 15;
    send_packet(packet, packet_size);

  } else {
    easy_fuck("Tried to send a packet type I don't know how to send.");
  };

  #undef header_size
  #undef packet_size
  #undef packet_data
  #undef data_size

  return 1;

  va_end(ooo);
};

//--page-split-- packet_parse_stream

char *packet_parse_stream(char *buffer, int length) {
  DEBUG("enter packet_splice_packets()");

  //  "buffer" points to the incomming network buffer
  //  "length" is the number of bytes in this buffer
  //
  //  This function parses the packet stream and then calls...
  //
  //      process_packet(int packet_type, char *data, int size)
  //
  //  ...to process the packets.
  //
  //  The return value is a pointer to the unused portion of the buffer,
  //  which will contain any partial packet not yet fully received.
  //
  //  In the event of error, return value is instead a pointer to an
  //  error string.  (string pointer is outside valid range of buffer)

  if (communication_error_flag) return buffer;

  char *pointer = buffer;
  int remain = length;

  int packet_type;
  int packet_size;
  char *packet_data;
  int data_size;

  double start_time = easy_time();

  while (remain >= 1 && easy_time() < start_time + 0.01) {

    // First, determine if we have a full packet header, and if so, determine
    // the size of the packet and calculate a pointer to the packet data.

    if (remain < 4) return pointer;
    packet_type = *((unsigned short*) &pointer[0]);
    data_size = *((unsigned short*) &pointer[2]);
    packet_size = 4 + data_size;
    packet_data = pointer + 4;

    // Make sure it's one we requested.  The code below will happily ignore
    // unrequested packets, but catching them here will help people find
    // mistakes in protocol implementation by displaying an error message,
    // in particular since desychronization can cause large packet sizes
    // to be read, which will take some time to receive the invalid packet.

    int index = packet_type / 8;
    int bit = 1 << (packet_type % 8);
    if (!(our_packet_selections[index] & bit)) {
      communication_error_flag = 1;
      dynamic_error_message = realloc(dynamic_error_message, 4096);
      sprintf(dynamic_error_message, "Received unselected packet type %d.", packet_type);
      dynamic_error_message = realloc(dynamic_error_message, 1 + strlen(dynamic_error_message));
      printf("\e[1;31mProtocol error: %s\e[0m\n", dynamic_error_message);
      if (display_packet_dumps) {
        if (remain > 256) remain = 256;
        printf("Remaining unprocessed packet stream:");
        for (int i = 0; i < remain; i++) {
          printf(" %02x", (unsigned char) pointer[i]);
        };
        printf("\n");
      } else {
        printf("Use --packets to see packet dumps of all received packets.\n");
      };
      return dynamic_error_message;
    };

    // Now that we know it's a valid packet type, see if we have it all.

    if (remain < packet_size) return pointer;

    // Now we know that we have a full packet to work with, so let's
    // display a dump of the packet, for debugging purposes.

    if (display_packet_dumps) {
      printf("\e[1;34mReceived a %s (%d)\e[0m\n", packet_type_to_string(packet_type), packet_type);
      printf("Packet dump:");
      for (int i = 0; i < packet_size; i++) {
        if (i == 4) printf(",");
        printf(" %02x", (unsigned char) pointer[i]);
      };
      printf("\n");
    };

    // We process a few packet types here, the two necessary for
    // choosing which packet types are used for communication,
    // and ping request since the response is simple.

    if (packet_type == PACKET_AVAILABLE_PACKET_TYPES) {

      if (data_size > 8192) {
        communication_error_flag = 1;
        dynamic_error_message = realloc(dynamic_error_message, 4096);
        sprintf(dynamic_error_message, "PACKET_SELECT_PACKET_TYPES data should be 8192 or fewer bytes, not %d bytes.", data_size);
        dynamic_error_message = realloc(dynamic_error_message, 1 + strlen(dynamic_error_message));
        printf("\e[1;31mProtocol error: %s\e[0m\n", dynamic_error_message);
        return dynamic_error_message;
      };

      memset(available_packet_types, 0, 8192);
      memmove(available_packet_types, packet_data, data_size);
      memset(our_packet_selections, 0, 8192);

      if (display_packet_dumps) {
        printf("Server has these packet types:\n");
        for (int i = 0; i < 8 * data_size; i++) {
          int index = i / 8;
          int bit = 1 << (i % 8);
          if (available_packet_types[index] & bit) {
            printf("  %s (%d)\n", packet_type_to_string(i), i);
          };
        };
      };

      // Packet type selections made here.  A function call is used to simplify
      // the process, so that you can do junk like this:
      //
      // choose(PACKET_THIS) || choose(PACKET_THAT);
      //
      // ...and if PACKET_THIS isn't available, PACKET_THAT will be selected,
      // since the C compiler's code won't call the second function if the
      // first function returns true.  Also, you can do this:
      //
      // choose(THIS) || choose(THAT) || error("What?!");

      // These packet types are handled in this file:

      choose(PACKET_AVAILABLE_PACKET_TYPES);
      choose(PACKET_SELECT_PACKET_TYPES);
      choose(PACKET_PING_REQUEST);
      choose(PACKET_PING_RESPONSE);
      choose(PACKET_IDLE_PING);

      //  The following packet types are handled by process_packet(),
      //  so adjust this code to meet your project's requirements.

      choose(PACKET_CONNECT);
      choose(PACKET_DISCONNECT);
      choose(PACKET_CHAT_MESSAGE);
      choose(PACKET_ANNOUNCE_PLAYER) && choose(PACKET_DENOUNCE_PLAYER);
      choose(PACKET_PLAYER_SKIN);
      choose(PACKET_SERVER_NAME);
      if (choose(PACKET_MAP_SETUP)) {
        choose(PACKET_MAP_PROPERTIES);
        choose(PACKET_MAP_LOADING);
        if (available(PACKET_MAP_TOTAL_RESET) && available(PACKET_MAP_TOTAL_DATA)) choose(PACKET_MAP_TOTAL_RESET), choose(PACKET_MAP_TOTAL_DATA);
        if (available(PACKET_BUFFER_RESET) && available(PACKET_BUFFER_APPEND) && available(PACKET_BUFFER_IS_MAP_DATA)) choose(PACKET_BUFFER_RESET), choose(PACKET_BUFFER_APPEND), choose(PACKET_BUFFER_IS_MAP_DATA);
        choose(PACKET_MAP_DATA);
        choose(PACKET_MAP_FILL);
        choose(PACKET_MAP_MODIFY);
        choose(PACKET_PLAYER_ABILITIES);
        choose(PACKET_MOVE_PLAYER);
        if (choose(PACKET_BLOCK_SETUP) && choose(PACKET_GROUP_SETUP) && choose(PACKET_TEXTURE_SETUP)) {
          if (available(PACKET_BUFFER_RESET) && available(PACKET_BUFFER_APPEND) && available(PACKET_BUFFER_IS_FILE_DATA))  choose(PACKET_BUFFER_RESET), choose(PACKET_BUFFER_APPEND), choose(PACKET_BUFFER_IS_FILE_DATA);
        };
        if (choose(PACKET_SKY_BOX)) choose(PACKET_TEXTURE_SETUP);
      };
      if (choose(PACKET_MENU_RESET)) {
        choose(PACKET_MENU_TEXT);
        choose(PACKET_MENU_BUTTON);
        choose(PACKET_MENU_RADIO);
        choose(PACKET_MENU_CHECK);
        choose(PACKET_MENU_INPUT);
        choose(PACKET_MENU_LINK);
        choose(PACKET_MENU_PASSWORD);
        choose(PACKET_MENU_TEXTURE);
      };

      choose(PACKET_PROJECTILE_CREATE);
      choose(PACKET_PROJECTILE_COLLISION);

      if (!strcmp(server_address, PORTAL_ADDRESS)) choose(PACKET_PORTAL_COOKIE);

      // Send our selections to the remote system...

      packet_send(PACKET_SELECT_PACKET_TYPES, our_packet_selections);

    } else if (packet_type == PACKET_SELECT_PACKET_TYPES) {

      if (data_size > 8192) {
        communication_error_flag = 1;
        dynamic_error_message = realloc(dynamic_error_message, 4096);
        sprintf(dynamic_error_message, "PACKET_SELECT_PACKET_TYPES data should be 8192 or fewer bytes, not %d bytes.", data_size);
        dynamic_error_message = realloc(dynamic_error_message, 1 + strlen(dynamic_error_message));
        printf("\e[1;31mProtocol error: %s\e[0m\n", dynamic_error_message);
        return dynamic_error_message;
      };

      memset(requested_packet_types, 0, 8192);
      memmove(requested_packet_types, packet_data, data_size);
      requested_packet_types[0] |= 0x03;

      if (display_packet_dumps) {
        printf("Server wants these packet types:\n");
        for (int i = 0; i < 8 * data_size; i++) {
          int index = i / 8;
          int bit = 1 << (i % 8);
          if (requested_packet_types[index] & bit) {
            printf("  %s (%d)\n", packet_type_to_string(i), i);
          };
        };
      };

      // We pass this along to process_packet(), but it isn't necessary that
      // process_packet() do anything with it aside from noticing that the
      // packet has been received, and so communication may now begin.

      process_packet(packet_type, packet_data, data_size);

    } else if (packet_type == PACKET_PING_REQUEST) {

      // We'll send the response here...

      packet_send(PACKET_PING_RESPONSE, packet_data, data_size);

      // Pass it along as a keep-alive, so no one erronously sends
      // a duplicate response, since we already sent one.

      process_packet(PACKET_IDLE_PING, NULL, 0);

    } else {

      // All other packets are processed by process_packet()

      process_packet(packet_type, packet_data, data_size);

    };

    pointer += packet_size; remain -= packet_size;

  };

  return pointer;

  DEBUG("leave packet_splice_packets()");
};
