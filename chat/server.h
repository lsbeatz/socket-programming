#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define CHAT_SERVER_PORT 3000
#define CHAT_SERVER_BACKLOG 5

struct client_info {
	int fd;
	int port;
	char ip[INET_ADDRSTRLEN];
};

struct client_head {
	struct client_info info;
	struct client_head *next;
	struct client_head *prev;
};

int server_init(void);
int server_bind(void);
int server_listen(void);
int server_accept(fd_set *fds);
int server_loop(void);
int server_exit(void);

void server_handle_client(fd_set *new_fds, fd_set *fds);

#endif /* CHAT_SERVER_H */
