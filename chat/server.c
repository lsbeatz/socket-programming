#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

#include "server.h"

#define MSG_SIZE 1024

static int server_fd;
static struct client_head *head;

static int init_client_list(void)
{
	head = (struct client_head *)malloc(sizeof(struct client_head));
	if (head == NULL) {
		printf("[%s] Failed to malloc\n", __func__);
		exit(EXIT_FAILURE);
	}

	head->next = NULL;
	head->prev = NULL;

	return 0;
}

static int add_client(struct client_info *info)
{
	struct client_head *node;

	node = (struct client_head *)malloc(sizeof(struct client_head));
	if (node == NULL) {
		printf("[%s] Failed to malloc\n", __func__);
		exit(EXIT_FAILURE);
	}

	memcpy(&node->info, info, sizeof(struct client_info));
	node->next = head->next;
	node->prev = head;
	if (head->next) {
		head->next->prev = node;
	}
	head->next = node;

	return 0;
}

static int delete_client(struct client_info *info, fd_set *fds)
{
	struct client_head *node = head->next;

	while (node) {
		if (node->info.fd != info->fd) {
			node = node->next;
			continue;
		}

		node->prev->next = node->next;
		if (node->next) {
			node->next->prev = node->prev;
		}

		FD_CLR(info->fd, fds);
		free(node);

		break;
	}

	return 0;
}

static int broadcast_msg_to_clients(const char *msg)
{
	struct client_head *node = head->next;

	printf("[%s] msg: %s\n" , __func__, msg);

	while (node) {
		send(node->info.fd, msg, strlen(msg), 0);
		node = node->next;
	}

	return 0;
}

static int get_max_fd(void)
{
	struct client_head *node = head->next;
	int fd = server_fd;

	while (node) {
		fd = (node->info.fd > fd) ? node->info.fd : fd;
		node = node->next;
	}

	return fd;
}

int server_init(void)
{
	init_client_list();

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == 0) {
		printf("[%s] Failed to create socket\n", __func__);
		exit(EXIT_FAILURE);
	}

	printf("[%s] Server socket %d created\n", __func__, server_fd);

	return 0;
}

int server_bind(void)
{
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(CHAT_SERVER_PORT);

	if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		printf("Failed to bind\n");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	printf("[%s] Server bound to port %d\n", __func__, CHAT_SERVER_PORT);

	return 0;
}


int server_listen(void)
{
	if (listen(server_fd, CHAT_SERVER_BACKLOG) < 0) {
		printf("Failed to listen\n");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	printf("[%s] Server %d wait for connections...\n", __func__, server_fd);

	return 0;
}

int server_accept(fd_set *fds)
{
	struct client_info info;
	struct sockaddr_in addr;
	socklen_t addr_len;

	info.fd = accept(server_fd, (struct sockaddr *)&addr, (socklen_t *)&addr_len);
	if (info.fd < 0) {
		printf("[%s] Failed to accept\n", __func__);
		return EXIT_FAILURE;
	}

	printf("[%s] Client %d accepted\n", __func__, info.fd);

	memset(&addr, 0, sizeof(addr));
	inet_ntop(AF_INET, &addr.sin_addr, info.ip, INET_ADDRSTRLEN);
	info.port = ntohs(addr.sin_port);

	FD_SET(info.fd, fds);

	add_client(&info);

	return 0;
}

int server_loop(void)
{
	fd_set fds;
	int num_fd;

	FD_ZERO(&fds);
	FD_SET(server_fd, &fds);

	while (1) {
		fd_set new_fds = fds;
		int max_fd = get_max_fd();

		num_fd = select(max_fd + 1, &new_fds, NULL, NULL, NULL);
		if (num_fd < 0) {
			printf("[%s] Failed to select\n", __func__);
			break;
		} else if (num_fd == 0) {
			printf("[%s] Timeout while select\n", __func__);
			continue;
		}

		if (FD_ISSET(server_fd, &new_fds)) {
			server_accept(&fds);
		}

		server_handle_client(&new_fds, &fds);
	}

	return 0;
}

int server_exit(void)
{
	close(server_fd);
	free(head);

	return 0;
}

void server_handle_client(fd_set *new_fds, fd_set *fds)
{
	struct client_head *node = head->next;
	char msg[MSG_SIZE];
	char buf[MSG_SIZE];
	int bytes;

	while (node) {
		int fd = node->info.fd;

		if (!FD_ISSET(fd, new_fds)) {
			goto cont;
		}

		bytes = recv(fd, msg, MSG_SIZE, 0);
		if (bytes <= 0) {
			printf("[%s] Failed to recv from client %d\n", __func__, fd);
		}
		msg[bytes] = '\0';

		printf("[%s] Msg from client %d: %s\n", __func__, fd, msg);
		snprintf(buf, MSG_SIZE, "Client %d: %s", fd, msg);
		broadcast_msg_to_clients(buf);

		if (!strcmp(msg, "bye")) {
			printf("[%s] Client %d connection closed\n", __func__, fd);
			snprintf(buf, MSG_SIZE, "Client %d exited", fd);
			broadcast_msg_to_clients(buf);
			delete_client(&node->info, fds);
			close(fd);
		}
cont:
		node = node->next;
	}
}

int main(void)
{
	server_init();
	server_bind();
	server_listen();
	server_loop();
	server_exit();

	return EXIT_SUCCESS;
}
