#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "msg.h"

#define DEFAULT_PORT 13
#define MAX_LINE 1024

int main(int argc, char **argv) {
  struct sockaddr_in saddr;
  int sockfd;
  char buf[MAX_LINE];
  ssize_t n;

  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(DEFAULT_PORT);
  inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr);

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    error("socket failed: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (connect(sockfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1) {
    error("connect failed: %s", strerror(errno));
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  printf("The current time is: ");

  while ((n = read(sockfd, buf, MAX_LINE)) > 0 ) {
    buf[n] = 0;
    fputs(buf, stdout);
  }

  return 0;
}
