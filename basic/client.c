#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MSG_SIZE 1024
#define LOCALHOST "127.0.0.1"

int main(void)
{
	struct sockaddr_in addr;
	char msg[MSG_SIZE];
	int client;

	client = socket(AF_INET, SOCK_STREAM, 0);
	if (client < 0) {
		printf("Failed to create socket\n");
		return EXIT_FAILURE;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);

	if (inet_pton(AF_INET, LOCALHOST, &addr.sin_addr) <= 0) {
		printf("Invalid address not allowed\n");
		return EXIT_FAILURE;
	}

	if (connect(client, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		printf("Failed to connect\n");
		return EXIT_FAILURE;
	}

	printf("Connected to server!\n");

	while (1) {
		printf("Send message: ");
		
		fgets(msg, MSG_SIZE, stdin);
		msg[strcspn(msg, "\n")] = '\0';

		send(client, msg, strlen(msg), 0);

		if (!strcmp(msg, "bye")) {
			printf("Close connection!\n");
			break;
		}
	}

	close(client);

	return EXIT_SUCCESS;
}
