#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(void) {
        int socket_fd;
        struct sockaddr_un unix_addr;
        size_t unix_addr_len;
        int accept_fd;

        socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (socket_fd == -1) {
                fprintf(stderr, "failed to create AF_UNIX socket\n");
                exit(EXIT_FAILURE);
        }

        memset(&unix_addr, 0, sizeof(unix_addr));
        unix_addr.sun_family = AF_UNIX;
        strncpy(unix_addr.sun_path, "server.socket", sizeof(unix_addr.sun_path));
        unix_addr_len = strlen("server.socket") + 1 + offsetof(struct sockaddr_un, sun_path);

        if (bind(socket_fd, (const struct sockaddr*)&unix_addr, unix_addr_len) == -1) {
        }

        if (listen(socket_fd, SOMAXCONN) == -1) {
        }

	for (;;) {
                accept_fd = accept(socket_fd, NULL, NULL);
        }

        return 0;
}
