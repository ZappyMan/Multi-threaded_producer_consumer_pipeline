#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main(void) {
  pid_t spawnpid = fork();
  switch (spawnpid){
    case -1: exit(1); break;
    case 0: break;
    default: break;
  }
  printf("XYZZY\n");
//  printf("%d", counter);
}
