/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/* https://vcansimplify.wordpress.com/2013/03/14/c-socket-tutorial-echo-server/ */

#include <arpa/inet.h>
#include <errno.h>
#include <grp.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ed_cmdline.h"
#include "ed_config.h"
#include "ed_daemon.h"
#include "ed_log.h"
#include "ed_net.h"
#include "ed_path.h"
#include "ed_pidfile.h"
#include "ed_privs.h"
#include "ed_rcode.h"
#include "ed_signals.h"
#include "ed_utils.h"

#define ED_ROOT_UID 0
#define BUFSIZE 8129

static const char *progname;

static sig_atomic_t volatile quit;
static int unsigned connected_clients = 0; /* Number of child processes. */

static void print_stats(void) {
  ed_log_info("connected_clients=%d", connected_clients);
}

static void sigchld_handler(int sig) {
  pid_t pid;
  int status;

  for (;;) {
    pid = waitpid(-1, &status, WNOHANG);
    if (pid <= 0) {
      break;
    }

    ed_log_info("subprocess %lu finished with status %d", (unsigned long)pid, status);
    --connected_clients;
    print_stats();
  }

  signal(SIGCHLD, sigchld_handler);
}

static void ed_signal_handler(int sig) {
  if (sig == SIGTERM || sig == SIGINT) {
    quit = 1;
  }

  ed_log_info("signal %d (%s) received, shutting down...", sig, ed_sig2str(sig));
}

/**
 * Setup signals.
 */
static void ed_signal_init(void) {
  signal(SIGCHLD, sigchld_handler);
  signal(SIGINT, ed_signal_handler);
  signal(SIGTERM, ed_signal_handler);
}

static void echo_stream(int fd) {
  char buf[BUFSIZE];
  int ret;

  while ((ret = read(fd, buf, sizeof(buf))) > 0 && write(fd, buf, ret) > 0);

  sleep(1);  /* allow socket to drain before signalling the socket is closed */
  close(fd);

  exit(EXIT_SUCCESS);
}

static int ed_main_loop(int tcpfd) {
  fd_set rfds_in;
  /* We need to have a copy of the fd set as it's not safe to reuse FD sets
   * after select(). */
  fd_set rfds_out;
  int rc;
  char clientip[46];
  int clientport;
  int clientfd;
  pid_t pid;

  FD_ZERO(&rfds_in);

  FD_SET(tcpfd, &rfds_in);

  for (;;) {
    if (quit == 1) {
      break;
    }

    memcpy(&rfds_out, &rfds_in, sizeof(fd_set));

    rc = select(tcpfd + 1, &rfds_out, NULL, NULL, NULL);
    if (rc > 0) {
      if (FD_ISSET(tcpfd, &rfds_out)) {
        clientfd = ed_net_tcp_socket_accept(tcpfd, clientip, sizeof(clientip), &clientport);
	if (clientfd == ED_NET_ERR) {
	  return -1;
	}

	ed_log_info("Accepted connection from %s:%d", clientip, clientport);

        ++connected_clients;
        print_stats();

        pid = fork();
        switch (pid) {
        case -1:
          close(clientfd);
          --connected_clients;
          print_stats();
          break;

        case 0:
          ed_log_info("Child process forked with pid %d.", pid);
          close(tcpfd);
          echo_stream(clientfd);
          break;

        default:
          close(clientfd); /* we are the parent so look for another connection. */
          ed_log_info("pid: %d", pid);
        }
      }
    }
  }

  return 0;
}

int main(int argc, char **argv) {
  ed_config_t config;
  int rc;
  int tcpfd;

  progname = ed_path_basename(argv[0]);

  ed_config_init(&config);

  rc = ed_cmdline_parse(argc, argv, progname, &config);
  if (rc != ED_OK) {
    ed_cmdline_display_help(progname);
    return EXIT_FAILURE;
  }

  if (show_help) {
    ed_cmdline_display_help(progname);
    return EXIT_SUCCESS;
  }

  if (show_version) {
    ed_cmdline_display_version(progname);
    return EXIT_SUCCESS;
  }

  if (geteuid() != ED_ROOT_UID) {
    fprintf(stderr, "%s: need root privileges\n", progname);
    return EXIT_FAILURE;
  } else {
    rc = setgroups(0, NULL);
    if (rc < 0) {
      ed_log_error("Failed to drop supplementary groups, %s", strerror(errno));
      return EXIT_FAILURE;
    }
  }

  ed_config_load(&config);

  rc = ed_log_init(progname, config.logfile);
  if (rc != ED_OK) {
    return rc;
  }

  ed_log_set_flag(ED_LOG_PRINT_TIME);
  ed_log_set_flag(ED_LOG_PRINT_LEVEL);

  ed_log_info("username = %s", config.username);
  ed_log_info("logfile  = %s", config.logfile);
  ed_log_info("pidfile  = %s", config.pidfile);
  ed_log_info("port     = %d", config.port);
  ed_log_info("backlog  = %d", config.backlog);

  /* Change the current working directory. */
  rc = chdir("/");
  if (rc < 0) {
    ed_log_error("Unable to chdir to '/': %s", strerror(errno));
    return ED_ERROR;
  }

  if (config.daemonize) {
    rc = ed_daemon_detach();
    if (rc != ED_OK) {
      ed_log_error("Couldn't daemonize %s: %s\n", progname, strerror(errno));
      return rc;
    }
  }

  config.pid = getpid();

  if (config.pidfile != NULL) {
    rc = ed_pidfile_write(config.pidfile, config.pid);
    if (rc != ED_OK) {
      return rc;
    }
  }

  ed_signal_init();

  tcpfd = ed_net_tcp_socket_listen(config.interface, config.port, config.backlog);
  if (tcpfd == ED_NET_ERR) {
    return EXIT_FAILURE;
  }

  rc = ed_change_user(config.username);
  if (rc != ED_OK) {
    return rc;
  }

  ed_log_info("%s started on %d", progname, config.pid);

  ed_main_loop(tcpfd);

  ed_log_info("Shutdown completed");
  ed_log_fini();
  ed_pidfile_remove(config.pidfile);

  return EXIT_SUCCESS;
}
