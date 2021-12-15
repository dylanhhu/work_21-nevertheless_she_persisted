#include "pipe_networking.h"


/*=========================
  server_handshake
  args: int * to_client

  Performs the client side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {
  printf("\ncreating well known pipe...\n");
  int wkp = mkfifo(WKP, 0644);
  if (wkp) {
    printf("couldn't create well known pipe (%s, %d)\n", strerror(errno), errno);
    exit(EXIT_FAILURE);
  }

  printf("created well known pipe.\n\nopening the pipe for reading...\n");
  wkp = open(WKP, O_RDONLY);
  if (wkp < 0) {
    printf("couldn't open well known pipe (%s, %d)\n", strerror(errno), errno);
    exit(EXIT_FAILURE);
  }

  printf("opened well known pipe\n\nreading from well known pipe...\n");
  char handshake[HANDSHAKE_BUFFER_SIZE];
  int r = read(wkp, handshake, sizeof(handshake));
  handshake[r] = 0;

  printf("recieved secret pipe info.\n\nremoving well known pipe...\n");
  remove(WKP);

  printf("removed well known pipe\n\ntrying to open secret pipe...\n");
  printf("secret pipe: %s\n", handshake);
  *to_client = open(handshake, O_WRONLY);
  if (*to_client < 0) {
    printf("couldn't open secret pipe (%s, %d)\n", strerror(errno), errno);
    exit(EXIT_FAILURE);
  }

  printf("opened secret pipe.\n\nsending ACK...\n");
  int w = write(*to_client, ACK, sizeof(ACK));
  if (w != sizeof(ACK)) {
    printf("couldn't send full ACK\n");
    printf("exiting...\n");
    exit(EXIT_FAILURE);
  }

  printf("sent ACK.\n\nlooking for final ACK from client...\n");
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

  printf("secret pipe name: %s\n", pid);
  
  printf("secret pipe created.\n\nopening well known pipe...\n");
  *to_server = open(WKP, O_WRONLY);
  if (*to_server < 0) {
    printf("couldn't open well known pipe (%s, %d)\n", strerror(errno), errno);
    printf("removing secret pipe and exiting...\n");
    remove(pid);
    exit(EXIT_FAILURE);
  }

  printf("opened well known pipe.\n\nsending secret pipe info...\n");
  int w = write(*to_server, pid, strlen(pid));
  if (w != strlen(pid)) {  // TODO: actually check correct
    printf("couldn't write secret pipe info");
    printf("removing secret pipe and exiting...\n");
    remove(pid);
    exit(EXIT_FAILURE);
  }

  printf("sent secret pipe info.\n\nopening secret pipe and waiting for ACK...\n");
  from_server = open(pid, O_RDONLY);
  if (from_server < 0) {
    printf("couldn't open secret pipe (%s, %d)\n", strerror(errno), errno);
    exit(EXIT_FAILURE);
  }

  printf("opened secret pipe\n\nwaiting for ACK...\n");
  char handshake[HANDSHAKE_BUFFER_SIZE];
  int r = read(from_server, handshake, sizeof(handshake));
  if (strcmp(handshake, ACK) != 0) {
    printf("recieved final handshake not as expected\n");
    printf("recieved: %s\n", handshake);
    printf("expected: %s\n", ACK);

    printf("removing secret pipe and exiting...\n");
    remove(pid);
    
    exit(EXIT_FAILURE);
  }

  printf("recieved ACK.\n\nremoving secret pipe...\n");
  remove(pid);

  printf("removed secret pipe.\n\nsending final ACK...\n");
  w = write(*to_server, ACK, sizeof(ACK));
  if (w != sizeof(ACK)) {
    printf("couldn't send full ACK...\n");
    printf("exiting...\n");
    exit(EXIT_FAILURE);
  }

  printf("final ACK sent, all done.\n");
  return from_server;
}
