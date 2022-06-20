#pragma once

/* Protocol definitions, intended for distribution! */

#define PACKET_AVAILABLE_PACKET_TYPES 0
#define PACKET_SELECT_PACKET_TYPES 1
#define PACKET_PING_REQUEST 2
#define PACKET_PING_RESPONSE 3
#define PACKET_IDLE_PING 4
#define PACKET_AUTHENTICATE 5
#define PACKET_CONNECT 6
#define PACKET_DISCONNECT 7
#define PACKET_CHAT_MESSAGE 8
#define PACKET_ANNOUNCE_PLAYER 9
#define PACKET_DENOUNCE_PLAYER 10
#define PACKET_PLAYER_ABILITIES 11
#define PACKET_MAP_LOADING 12
#define PACKET_RANDOM_GARBAGE 13
#define PACKET_MAP_SETUP 14
#define PACKET_MAP_PROPERTIES 15
#define PACKET_MAP_FILL 16
#define PACKET_MAP_DATA 17
#define PACKET_MAP_TOTAL_RESET 18
#define PACKET_MAP_TOTAL_DATA 19
#define PACKET_BUFFER_RESET 20
#define PACKET_BUFFER_APPEND 21
#define PACKET_BUFFER_IS_MAP_DATA 22
#define PACKET_BUFFER_IS_FILE_DATA 23
#define PACKET_MOVE_PLAYER 24
#define PACKET_MAP_MODIFY 25
#define PACKET_FILE_REQUEST 26
#define PACKET_TEXTURE_SETUP 27
#define PACKET_GROUP_SETUP 28
#define PACKET_BLOCK_SETUP 29
#define PACKET_MENU_REQUEST 30
#define PACKET_MENU_RESPONSE 31
#define PACKET_MENU_RESET 32
#define PACKET_MENU_TEXT 33
#define PACKET_MENU_LINK 34
#define PACKET_MENU_CHECK 35
#define PACKET_MENU_RADIO 36
#define PACKET_MENU_INPUT 37
#define PACKET_MENU_BUTTON 38
#define PACKET_MENU_ERASE 39
#define PACKET_MENU_PASSWORD 40
#define PACKET_MENU_RESIZE 41
#define PACKET_SKY_BOX 42
#define PACKET_CLIENT_VERSION 43
#define PACKET_PORTAL_COOKIE 44
#define PACKET_PLAYER_SKIN 45
#define PACKET_MENU_TEXTURE 46
#define PACKET_SERVER_NAME 47
#define PACKET_PROJECTILE_CREATE 48
#define PACKET_PROJECTILE_COLLISION 49
#define PACKET_TYPE_LIMIT 50

#define DISCONNECT_NULL 0
#define DISCONNECT_ERROR 1
#define DISCONNECT_REBOOT 2
#define DISCONNECT_FULL 3
#define DISCONNECT_IDLE 4
#define DISCONNECT_KICK 5
#define DISCONNECT_BAN 6
#define DISCONNECT_RANDOM 7
