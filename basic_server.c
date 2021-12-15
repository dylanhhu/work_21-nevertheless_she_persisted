#include "pipe_networking.h"


static void sighandler(int signo);
void reverse(char *str);


int main() {
  signal(SIGINT, sighandler);

  int to_client;
  int from_client;
  
  char data[500];

  while (1) {
    from_client = server_handshake(&to_client);

    while (1) {
      int r = read(from_client, data, sizeof(data));
      data[r] = 0;

      reverse(data);
      write(to_client, data, strlen(data));
    }
  }

  return 0;
}


static void sighandler(int signo) {
  if (signo == SIGINT) {
    remove(WKP);
    printf("\n");
    exit(EXIT_SUCCESS);
  }
}


void reverse(char *str) {
  int len = strlen(str);

  int i;
  for (i = 0; i < len / 2; i++) {
    int temp = str[len-i-1];
    str[len-i-1] = str[i];
    str[i] = temp;
  }
}
