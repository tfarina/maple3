/* https://vcansimplify.wordpress.com/2013/03/14/c-socket-tutorial-echo-server/ */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>

#include "ec_net.h"
#include "ec_utils.h"

#define BUFSIZE 4096

#define CRLF "\x0d\x0a"

#define EC_TCP_PORT 7

static char const *progname;

static char short_options[] =
    "h"  /* help */
    "a"  /* server address */
    "p:" /* tcp port number of the server */
    ;

static struct option long_options[] = {
    { "help",        no_argument,       NULL, 'h' }, /* help */
    { "address",     required_argument, NULL, 'a' }, /* server address */
    { "port",        required_argument, NULL, 'p' }, /* tcp port number of the server */
    { NULL,          0,                 NULL,  0  }
};

static void ec_show_usage(char const *program_name) {
  fprintf(stderr,
	  "usage: %s [-h] [-a server address] [-p port]" CRLF CRLF,
	  program_name);
  fprintf(stderr,
	  "options:" CRLF
          "  -h, --help              show usage, options and exit" CRLF
          "  -a, --address=          server address" CRLF
          "  -p, --port=N            set the tcp port of the server (default: %d)" CRLF
	  "",
          EC_TCP_PORT);
}

int main(int argc, char **argv) {
  int c, value;
  int sockfd;
  char sendline[BUFSIZE];
  char recvline[BUFSIZE];
  char *address;
  int port;

  progname = ec_get_progname(argv[0]);

  port = EC_TCP_PORT;

  for (;;) {
    c = getopt_long(argc, argv, short_options, long_options, NULL);
    if (c == -1) {
      /* no more options */
      break;
    }

    switch (c) {
    case 'h':
      ec_show_usage(progname);
      return EXIT_SUCCESS;

    case 'a':
      address = optarg;
      break;

    case 'p':
      value = atoi(optarg);
      if (value <= 0) {
	fprintf(stderr, "%s: option -p requires a non zero number\n", progname);
	return EXIT_FAILURE;
      }
      if (!ec_valid_port(value)) {
	fprintf(stderr, "%s: option -s value %d is not a valid port\n", progname, value);
	return EXIT_FAILURE;
      }

      port = value;
      break;

    case '?':
      ec_show_usage(progname);
      return EXIT_SUCCESS;

    default:
      fprintf(stderr, "%s: invalid option -- '%c'", progname, optopt);
      return -1;
      /* NOTREACHED */
    }
  }

  sockfd = ec_net_tcp_socket_connect(address, port);

  memset(sendline, 0, sizeof(sendline));
  memset(recvline, 0, sizeof(recvline));

  while (1) {
    fgets(sendline, BUFSIZE, stdin); /*stdin = 0 , for standard input */

    write(sockfd, sendline, strlen(sendline) + 1);

    read(sockfd, recvline, BUFSIZE);
    fputs(recvline, stdout);
  }

  return 0;
}
