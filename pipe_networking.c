#include "pipe_networking.h"


/*=========================
  server_handshake
  args: int * to_client

  Performs the client side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {
  printf("creating well known pipe...\n");
  int wkp = mkfifo(WKP, 0644);
  if (wkp) {
    printf("couldn't create well known pipe (%s, %d)\n", strerror(errno), errno);
    exit(EXIT_FAILURE);
  }

  printf("created well known pipe. opening the pipe for reading...\n");
  wkp = open(WKP, O_RDONLY);
  if (wkp < 0) {
    printf("couldn't open well known pipe (%s, %d)\n", strerror(errno), errno);
    exit(EXIT_FAILURE);
  }

  printf("opened well known pipe, reading...\n");
  char handshake[HANDSHAKE_BUFFER_SIZE];
  int r = read(wkp, handshake, sizeof(handshake));

  printf("recieved secret pipe info, removing well known pipe...\n");
  remove(WKP);

  printf("removed well known pipe, trying to open secret pipe...\n");
  printf("secret pipe: %s\n", handshake);
  *to_client = open(handshake, O_WRONLY);
  if (*to_client < 0) {
    printf("couldn't open secret pipe (%s, %d)\n", strerror(errno), errno);
    exit(EXIT_FAILURE);
  }

  printf("opened secret pipe, sending ACK...\n");
  int w = write(*to_client, ACK, sizeof(ACK));
  if (w != sizeof(ACK)) {
    printf("couldn't send full ACK...\n");
    exit(EXIT_FAILURE);
  }

  printf("sent ACK... looking for final ACK from client...\n");
  r = read(wkp, handshake, sizeof(handshake));
  if (strcmp(handshake, ACK) != 0) {
    printf("recieved final handshake not as expected\n");
    printf("recieved: %s\n", handshake);
    printf("expected: %s\n", ACK);
    
    exit(EXIT_FAILURE);
  }

  printf("final handshake recieved. all done\n");
  int from_client = wkp;
  return from_client;
}


/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
  printf("creating secret pipe...\n");

  char pid[100];
  sprintf(pid, "%d", getpid());
  int from_server = mkfifo(pid, 0644);
  if (from_server) {
    printf("couldn't create secret pipe (%s, %d)\n", strerror(errno), errno);
    printf("tried creating secret pipe using name: %s\n", pid);
    exit(EXIT_FAILURE);
  }
  
  printf("opening well known pipe...\n");
  *to_server = open(WKP, O_WRONLY);
  if (*to_server < 0) {
    printf("couldn't open well known pipe (%s, %d)\n", strerror(errno), errno);
    exit(EXIT_FAILURE);
  }

  return from_server;
}
