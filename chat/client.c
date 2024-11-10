#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "client.h"

#define LOCALHOST "127.0.0.1"
#define MSG_SIZE 1024
#define MAX_MSGS 20
#define MSG_HEIGHT 22

#define move_cursor(x, y) do {					\
			printf("\033[%d;%dH", (y), (x));	\
			fflush(stdout);						\
		} while (0)

#define clear_screen() do {						\
			printf("\033[2J");					\
			fflush(stdout);						\
		} while (0)

#define clear_line(y) do {						\
			move_cursor(1, (y));				\
			printf("\033[K");					\
			fflush(stdout);						\
		} while (0)

static struct thread_info threads[CHAT_CLIENT_MAX_THREADS];
static pthread_mutex_t io_mutex = PTHREAD_MUTEX_INITIALIZER;
static int client_fd;

static void *thread_sender(void *arg)
{
	char msg[MSG_SIZE];

	while (1) {
		pthread_mutex_lock(&io_mutex);
		clear_line(1);
		move_cursor(1, 1);
		printf("> ");
		fflush(stdout);
		pthread_mutex_unlock(&io_mutex);

		fgets(msg, MSG_SIZE, stdin);
		msg[strcspn(msg, "\n")] = '\0';

		send(client_fd, msg, strlen(msg), 0);

		if (!strcmp(msg, "bye")) {
			printf("[%s] Close connection\n", __func__);
			break;
		}
	}

	return NULL;
}

static void *thread_receiver(void *arg)
{
	char msg[MSG_SIZE];
	int msg_cnt = 0;
	int msg_y_start = 3;
	int bytes;

	while (1) {
		bytes = recv(client_fd, msg, MSG_SIZE, 0);
		if (bytes <= 0) {
			printf("[%s] Failed to recv\n", __func__);
			break;
		}
		msg[bytes] = '\0';

		pthread_mutex_lock(&io_mutex);
		if (msg_cnt < MSG_HEIGHT) {
			move_cursor(1, msg_y_start + msg_cnt);
			printf("%s\n", msg);
			msg_cnt++;
		} else {
			for (int i = 0; i < MSG_HEIGHT - 1; i++) {
				move_cursor(1, msg_y_start + i);
				clear_line(msg_y_start + i);
				move_cursor(1, msg_y_start + i);
				move_cursor(1, msg_y_start + i + 1);
			}
			move_cursor(1, msg_y_start + MSG_HEIGHT - 1);
			printf("%s\n", msg);
		}
		move_cursor(3, 1);
		fflush(stdout);
		pthread_mutex_unlock(&io_mutex);
	}

	return NULL;
}

int client_init(void)
{
	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_fd < 0) {
		printf("[%s] Failed to create socket\n", __func__);
		exit(EXIT_FAILURE);
	}

	return 0;
}

int client_connect(void)
{
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(CHAT_SERVER_PORT);

	if (inet_pton(AF_INET, LOCALHOST, &addr.sin_addr) <= 0) {
		printf("[%s] Invalid address not allowed\n", __func__);
		close(client_fd);
		exit(EXIT_FAILURE);
	}

	if (connect(client_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		printf("[%s] Failed to connect\n", __func__);
		close(client_fd);
		exit(EXIT_FAILURE);
	}

	printf("[%s] Connected to chat server\n", __func__);

	return 0;
}

int client_loop(void)
{
	int rc;
	void *status;

	clear_screen();

	threads[0].func = &thread_sender;
	threads[1].func = &thread_receiver;

	for (int i = 0; i < CHAT_CLIENT_MAX_THREADS; i++) {
		rc = pthread_create(&threads[i].id, NULL, threads[i].func, NULL);
		if (rc != 0) {
			printf("[%s] Failed to create thread %d\n", __func__, i);
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 0; i < CHAT_CLIENT_MAX_THREADS; i++) {
		rc = pthread_join(threads[i].id, &status);
		if (rc != 0) {
			printf("[%s] Failed join thread %d\n", __func__, i);
		}

		printf("[%s] Joined thread %d with status(%s)\n", __func__, i, (char *)status);
		free(status);
	}

	return 0;
}

int client_exit(void)
{
	close(client_fd);
	pthread_mutex_destroy(&io_mutex);

	return 0;
}

int main(void)
{
	client_init();
	client_connect();
	client_loop();
	client_exit();

	return EXIT_SUCCESS;
}
