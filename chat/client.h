#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include <pthread.h>

#define CHAT_SERVER_PORT 3000
#define CHAT_CLIENT_MAX_THREADS 2

struct thread_info {
	pthread_t id;
	void *(*func)(void *);
	void *arg;
};

int client_init(void);
int client_connect(void);
int client_loop(void);
int client_exit(void);

#endif /* CHAT_CLIENT_H */
