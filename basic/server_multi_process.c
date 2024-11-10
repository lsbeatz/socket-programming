#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define PORT 8080
#define MSG_SIZE 1024

void handle_client(int client)
{
	struct sockaddr_in addr;
	char msg[MSG_SIZE];
	char client_ip[INET_ADDRSTRLEN];
	int client_port;
	int bytes;

	memset(&addr, 0, sizeof(addr));

	inet_ntop(AF_INET, &addr.sin_addr, client_ip, INET_ADDRSTRLEN);
	client_port = ntohs(addr.sin_port);

	printf("Client[%s:%d] connected!\n", client_ip, client_port);

	while (1) {
		bytes = recv(client, msg, MSG_SIZE, 0);
		if (bytes <= 0) {
			break;
		}

		msg[bytes] = '\0';
		printf("message from [%s:%d]: %s\n", client_ip, client_port, msg);

		if (!strcmp(msg, "bye")) {
			printf("Client connection closed!\n");
			break;
		}
	}
	
	close(client);
	exit(EXIT_SUCCESS);
}

int main(void)
{
	struct sockaddr_in addr;
	int client;
	int server;
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

	printf("Wait for connections...\n");

	while (1) {
		client = accept(server, (struct sockaddr *)&addr, (socklen_t *)&addr_len);
		if (client < 0) {
			printf("Failed to accept\n");
			continue;
		}

		pid_t pid = fork();
		if (pid == 0) { // child process
			close(server);
			handle_client(client);
		} else if (pid > 0) { // parent process
			close(client);
		} else {
			printf("Failed to fork\n");
			close(client);
		}
	}

	close(server);

	return EXIT_SUCCESS;
}
