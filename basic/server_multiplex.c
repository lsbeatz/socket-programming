#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <arpa/inet.h>

#define PORT 8080
#define MSG_SIZE 1024

void handle_client(int client, fd_set *sockets)
{
	struct sockaddr_in addr;
	char msg[MSG_SIZE];
	char client_ip[INET_ADDRSTRLEN];
	int client_port;
	int bytes;

	memset(&addr, 0, sizeof(addr));

	inet_ntop(AF_INET, &addr.sin_addr, client_ip, INET_ADDRSTRLEN);
	client_port = ntohs(addr.sin_port);

	bytes = recv(client, msg, MSG_SIZE, 0);
	if (bytes <= 0) {
		printf("Failed to recv from client[%s:%d]\n", client_ip, client_port);
		return;
	}

	msg[bytes] = '\0';
	printf("message from [%s:%d]: %s\n", client_ip, client_port, msg);

	if (!strcmp(msg, "bye")) {
		printf("Client[%s:%d] connection closed!\n", client_ip, client_port);
		FD_CLR(client, sockets);
		close(client);
	}
}

int main(void)
{
	struct sockaddr_in addr;
	fd_set sockets, cpy_sockets;
	int max_socket, nr_socket;
	int client, server;
	int addr_len = sizeof(addr);

	server = socket(AF_INET, SOCK_STREAM, 0);
	if (server == 0) {
		printf("Failed to create socket\n");
		return EXIT_FAILURE;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(PORT);

	if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		printf("Failed to bind\n");
		close(server);
		return EXIT_FAILURE;
	}

	if (listen(server, 5) < 0) {
		printf("Failed to listen\n");
		close(server);
		return EXIT_FAILURE;
	}

	printf("Server %d wait for connections...\n", server);

	FD_ZERO(&sockets);
	FD_SET(server, &sockets);
	max_socket = server;

	while (1) {
		cpy_sockets = sockets;
		nr_socket = select(max_socket + 1, &cpy_sockets, NULL, NULL, NULL);
		if (nr_socket < 0) {
			printf("Failed to select\n");
			break;
		} else if (nr_socket == 0) {
			continue;
		}

		for (int i = 0; i <= max_socket; i++) {
			if (!FD_ISSET(i, &cpy_sockets)) {
				continue;
			}

			if (i == server) {
				client = accept(server, (struct sockaddr *)&addr, (socklen_t *)&addr_len);
				if (client < 0) {
					printf("Filed to accept\n");
					break;
				}

				printf("Client %d Accepted\n", client);

				FD_SET(client, &sockets);
				max_socket = (max_socket < client) ? client : max_socket;
				continue;
			}

			handle_client(i, &sockets);
		}
	}

	close(server);

	return EXIT_SUCCESS;
}
