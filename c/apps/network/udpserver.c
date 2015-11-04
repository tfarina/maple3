#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "die.h"

#define BUFLEN 512
#define PORT 5300

int main(void) {
  struct addrinfo hints, *addrlist;
  int rv;
  struct sockaddr_storage remoteaddr;
  socklen_t addrlen;
  int sockfd;
  int recvlen;
  char buf[BUFLEN];
  int msgcnt = 0;  /* count # of messages we received */

  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  if ((rv = getaddrinfo(NULL, "5300", &hints, &addrlist)) != 0) {
    fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(rv));
    exit(EXIT_FAILURE);
  }

  if ((sockfd = socket(addrlist->ai_family, addrlist->ai_socktype,
                       addrlist->ai_protocol)) == -1)
    die("cannot create socket");

  if (bind(sockfd, addrlist->ai_addr, addrlist->ai_addrlen) == -1)
    die("bind failed");

  printf("listening on port %d\n", PORT);

  addrlen = sizeof(remoteaddr);
  for (;;) {
    recvlen = recvfrom(sockfd, buf, sizeof(buf), 0,
                       (struct sockaddr *)&remoteaddr, &addrlen);
    if (recvlen <= 0) {
     continue;
    }
    buf[recvlen] = '\0';
    printf("received message: %s\n", buf);

    sprintf(buf, "ack %d", msgcnt++);
    if (sendto(sockfd, buf, strlen(buf), 0,
               (struct sockaddr *)&remoteaddr, addrlen) == -1)
      die("sendto failed: %s", strerror(errno));
    printf("response sent: %s\n", buf);
  }

  return 0;
}
