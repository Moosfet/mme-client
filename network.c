#include "everything.h"

#ifdef WINDOWS
  #include <w32api.h>
  #include <winsock2.h>
  //#include <windows.h>
  #include <ws2tcpip.h>
  #define SOCKET_ERRNO WSAGetLastError()
  //#define ECONNREFUSED WSAECONNREFUSED
  //#define EHOSTUNREACH WSAEHOSTUNREACH
  //#define ENETUNREACH WSAENETUNREACH
  //#define ETIMEDOUT WSAETIMEDOUT
  //#define EINVAL WSAEINVAL
  //#define ECONNRESET WSAECONNRESET
  //#define ECONNABORTED WSAECONNABORTED
#else
  #include <sys/types.h> // recommended for socket, bind, recvfrom, getaddrinfo
  #include <sys/socket.h> // required for socket, bind, recvfrom, inet_ntoa, getaddrinfo
  #include <arpa/inet.h> // required for inet_ntoa
  #include <netinet/in.h> // required according to "man 7 ip" for the IPv4 protocol implementation, and for inet_ntoa
  #include <netinet/ip.h> // required according to "man 7 ip" for the IPv4 protocol implementation
  #include <termios.h>
  #include <unistd.h> // required for close
  #include <netdb.h> // required for getaddrinfo
  #include <errno.h>
  #define SOCKET_ERRNO errno
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR -1
  #define closesocket close
  #include <signal.h>
  #include <unistd.h>
  #include <fcntl.h>
#endif

static pthread_mutex_t mutex;

static struct easy_thread_state thread_state;

struct structure_socket_data {
  int socket;
  int status;
  char *name;
  char *port;
  int cancel;
};

double network_last_response = 0;
char *network_error_string = NULL;
static char *custom_error_string = NULL;

//--page-split-- network_initialize

void network_initialize(void) {
  easy_mutex_init(&mutex, NULL);
  #ifdef UNIX
    signal(SIGPIPE, SIG_IGN);
  #endif
  #ifdef WINDOWS
    WSADATA bullshit;
    WSAStartup(0x0202, &bullshit);
  #endif
};

//--page-split-- free_socket

static void free_socket(struct structure_socket_data **the_socket) {
  closesocket((*the_socket)->socket);
  memory_allocate(&(*the_socket)->name, 0);
  memory_allocate(&(*the_socket)->port, 0);
  memory_allocate(the_socket, 0);
};

//--page-split-- thread

static void *thread(void *parameter) {
  struct structure_socket_data *the_copy = parameter;
  struct structure_socket_data **the_socket = &the_copy;

  // Perform DNS lookup...

  printf("Finding map server's IP address...\n");

  struct addrinfo hints = {};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  struct addrinfo *results = NULL;

  int return_value = getaddrinfo((*the_socket)->name, (*the_socket)->port, &hints, &results);

  if (return_value != 0) {
    if (return_value == EAI_NONAME) {
      network_error_string = "Unable to resolve server address.";
    } else {
      memory_allocate(&custom_error_string, 4096);
      snprintf(custom_error_string, 4096, "Name resolution error: getaddrinfo(\"%s\", %s): %s", (*the_socket)->name, (*the_socket)->port, gai_strerror(return_value));
      custom_error_string[4095] = 0;
      memory_allocate(&custom_error_string, strlen(custom_error_string) + 1);
      network_error_string = custom_error_string;
      printf("%s\n", custom_error_string);
    };
    easy_mutex_lock(&mutex);
    if ((*the_socket)->cancel) {
      free_socket(the_socket);
    } else {
      (*the_socket)->status = SERVER_STATUS_ERROR;
    };
    easy_mutex_unlock(&mutex);
    return NULL;
  };

  int count = 0;
  for (struct addrinfo *p = results; p != NULL; p = p->ai_next) count++;
  printf("...getaddrinfo() returned %d result%s.\n", count, count == 1 ? "" : "s");

  // Connect to server...

  struct addrinfo *result = results;
  for (int index = 1; result != NULL; result = result->ai_next) {
    printf("Attempt %d: ", index++);
    //if (result->ai_family == AF_INET6) printf("Not attempting IPv6 connection.\n"), continue;
    return_value = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (return_value == INVALID_SOCKET) {
      printf("socket(): %s\n", error_string(SOCKET_ERRNO));
      if (network_error_string == NULL) network_error_string = "Unable to create socket.";
      continue;
    } else {
      (*the_socket)->socket = return_value;
    };
    return_value = connect((*the_socket)->socket, result->ai_addr, result->ai_addrlen);
    if (return_value == SOCKET_ERROR) {
      // We set error string here, but not STATUS_ERROR unless all connection attempts fail.
      if (SOCKET_ERRNO == ECONNREFUSED) {
        network_error_string = "The server is not accepting connections.";
      } else if (SOCKET_ERRNO == EHOSTUNREACH) {
        network_error_string = "Unable to contact the server.";
      } else if (SOCKET_ERRNO == ENETUNREACH) {
        network_error_string = "Unable to access the network.";
      } else if (SOCKET_ERRNO == ETIMEDOUT) {
        network_error_string = "Received no response from the server.";
      } else if (SOCKET_ERRNO == EINVAL) {
        network_error_string = "The connection failed for unexplained reasons.";
      } else {
        memory_allocate(&custom_error_string, 4096);
        sprintf(custom_error_string, "connect(): %s", error_string(SOCKET_ERRNO));
        memory_allocate(&custom_error_string, strlen(custom_error_string) + 1);
        network_error_string = custom_error_string;
      };
      printf("%s\n", network_error_string);
    } else {
      #ifdef UNIX
        int flags = fcntl((*the_socket)->socket, F_GETFL, 0);
        fcntl((*the_socket)->socket, F_SETFL, flags | O_NONBLOCK);
      #endif
      #ifdef WINDOWS
        int mode = 1;
        ioctlsocket((*the_socket)->socket, FIONBIO, (void *) &mode);
      #endif
      printf("Connection successfully established.\n");
      network_last_response = on_frame_time;
      easy_mutex_lock(&mutex);
      if ((*the_socket)->cancel) {
        free_socket(the_socket);
      } else {
        (*the_socket)->status = SERVER_STATUS_CONNECTED;
      };
      easy_mutex_unlock(&mutex);
      freeaddrinfo(results);
      return NULL;
    };
    closesocket((*the_socket)->socket);
  };

  // Errors suck!

  easy_mutex_lock(&mutex);
  if ((*the_socket)->cancel) {
    free_socket(the_socket);
  } else {
    (*the_socket)->status = SERVER_STATUS_ERROR;
  };
  easy_mutex_unlock(&mutex);
  freeaddrinfo(results);
  return NULL;
};

//--page-split-- network_initiate_connect

void *network_initiate_connect(struct structure_socket_data **the_socket, char *address) {
  DEBUG("enter network_initiate_connect()");

  // creates socket, connects to a "server:port"
  // does not wait for connection to complete
  // Errors are reported by network_connect_status()

  if (address == NULL) easy_fuck("Null network address!");

  memory_allocate(the_socket, sizeof(struct structure_socket_data));
  memset(*the_socket, 0, sizeof(**the_socket));

  char *string = NULL;
  memory_allocate(&string, strlen(address) + 1);
  strcpy(string, address);
  if (string[0] == '[') {
    char *match = strchr(string, ']');
    if (match == NULL) easy_fuck("Unmatched square bracket in network address!");
    int length = match - string - 1;
    memory_allocate(&(*the_socket)->name, length + 1);
    memmove((*the_socket)->name, string + 1, length);
    (*the_socket)->name[length] = 0;
    if (match[1] != ':' || match[2] == 0) easy_fuck("Port specifier missing from network address!");
    memory_allocate(&(*the_socket)->port, strlen(match+2) + 1);
    strcpy((*the_socket)->port, match+2);
  } else if (strchr(string, ':') != strrchr(string, ':')) {
    easy_fuck("Numerical IPv6 addresses must be enclosed in square brackets!");
  } else {
    char *split = strrchr(string, ':');
    if (split == NULL) easy_fuck("Port specifier missing from network address!");
    *(split++) = 0;
    memory_allocate(&(*the_socket)->name, strlen(string) + 1);
    memory_allocate(&(*the_socket)->port, strlen(split) + 1);
    strcpy((*the_socket)->name, string);
    strcpy((*the_socket)->port, split);
  };
  memory_allocate(&string, 0);

  printf("Connecting to %s on port %s...\n", (*the_socket)->name, (*the_socket)->port);

  (*the_socket)->status = SERVER_STATUS_CONNECTING;

  // Since DNS lookups seemingly require a thread in order to be non-blocking,
  // I'm just going to do the entire connect sequence in a thread.  Fuck me!

  easy_thread_fork(&thread_state, thread, (void *) *the_socket);

  DEBUG("leave network_initiate_connect()");
};

//  This shit should probably check the "cancel" flag
//  before setting the error string, but whatever.

//--page-split-- network_free_socket

void network_free_socket(struct structure_socket_data **the_socket) {
  DEBUG("enter network_free_socket()");

  easy_mutex_lock(&mutex);
  if (*the_socket != NULL) {
    if ((*the_socket)->status == SERVER_STATUS_CONNECTING) {
      (*the_socket)->cancel = 1;
      *the_socket = NULL;
    } else {
      free_socket(the_socket);
    };
  };
  easy_mutex_unlock(&mutex);

  DEBUG("leave network_free_socket()");
};

//--page-split-- network_status

int network_status(struct structure_socket_data **the_socket) {
  // tests whether a connection is complete
  // return values:
  //   -1 = disconnected / error
  //    0 = result not yet known
  //   +1 = connection successful
  // writes message to network_error_string upon error
  if (*the_socket != NULL) {
    if ((*the_socket)->status == SERVER_STATUS_CONNECTING) return 0;
    if ((*the_socket)->status == SERVER_STATUS_CONNECTED) return +1;
  };
  return -1;
};

//--page-split-- network_read

int network_read(struct structure_socket_data **the_socket, char **buffer, int *size) {
  if (*the_socket == NULL) return -1;
  if ((*the_socket)->status == SERVER_STATUS_CONNECTED) {
    struct timeval timeout = {};
    fd_set reads = {}, writes = {};
    FD_SET((*the_socket)->socket, &reads);
    int return_value = select(8 * sizeof(fd_set), &reads, &writes, NULL, &timeout);
    if (return_value == SOCKET_ERROR) {
      memory_allocate(&custom_error_string, 4096);
      sprintf(custom_error_string, "select(): %s", error_string(SOCKET_ERRNO));
      memory_allocate(&custom_error_string, strlen(custom_error_string) + 1);
      network_error_string = custom_error_string;
    };
    if (FD_ISSET((*the_socket)->socket, &reads)) {
      memory_allocate(buffer, *size + 65536);
      return_value = recv((*the_socket)->socket, *buffer + *size, 65536, 0);
      if (return_value == 0) {
        (*the_socket)->status = SERVER_STATUS_DISCONNECTED;
        network_error_string = "Socket closed via end-of-file on read.";
        return -1;
      } else if (return_value == SOCKET_ERROR) {
        (*the_socket)->status = SERVER_STATUS_ERROR;
        if (SOCKET_ERRNO == ECONNRESET || SOCKET_ERRNO == ECONNABORTED) {
          network_error_string = "Connection abruptly terminated!";
        } else {
          memory_allocate(&custom_error_string, 4096);
          sprintf(custom_error_string, "recv(): %s", error_string(SOCKET_ERRNO));
          memory_allocate(&custom_error_string, strlen(custom_error_string) + 1);
          network_error_string = custom_error_string;
        };
        return -1;
      } else {
        *size += return_value;
        network_last_response = on_frame_time;
        return +1;
      };
    };
    return 0;
  } else {
    return -1;
  };
};

//--page-split-- network_write

int network_write(struct structure_socket_data **the_socket, char **buffer, int *size) {
  if (*the_socket == NULL) return -1;
  if ((*the_socket)->status == SERVER_STATUS_CONNECTED && *size > 0) {
    struct timeval timeout = {};
    fd_set reads = {}, writes = {};
    FD_SET((*the_socket)->socket, &writes);
    int return_value = select(8 * sizeof(fd_set), &reads, &writes, NULL, &timeout);
    if (return_value == SOCKET_ERROR) {
      memory_allocate(&custom_error_string, 4096);
      sprintf(custom_error_string, "select(): %s", error_string(SOCKET_ERRNO));
      memory_allocate(&custom_error_string, strlen(custom_error_string) + 1);
      network_error_string = custom_error_string;
    };
    if (FD_ISSET((*the_socket)->socket, &writes)) {
      lag_push(1, "send()");
      return_value = send((*the_socket)->socket, *buffer, *size, 0);
      lag_pop();
      if (return_value == 0) {
        (*the_socket)->status = SERVER_STATUS_DISCONNECTED;
        network_error_string = "Socket closed via end-of-file on write.";
        return -1;
      } else if (return_value == SOCKET_ERROR) {
        (*the_socket)->status = SERVER_STATUS_ERROR;
        if (SOCKET_ERRNO == ECONNRESET || SOCKET_ERRNO == ECONNABORTED) {
          network_error_string = "Connection abruptly terminated!";
        } else {
          memory_allocate(&custom_error_string, 4096);
          sprintf(custom_error_string, "send(): %s", error_string(SOCKET_ERRNO));
          memory_allocate(&custom_error_string, strlen(custom_error_string) + 1);
          network_error_string = custom_error_string;
        };
        return -1;
      } else {
        memmove(*buffer, *buffer + return_value, *size - return_value);
        *size -= return_value;
        memory_allocate(buffer, *size);
        return +1;
      };
    };
    return 0;
  };
};
