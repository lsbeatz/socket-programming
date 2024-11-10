CC = gcc
CFLAGS = -g -Wall

BUILD = build

SRCS  = client.c
SRCS += server_single.c
SRCS += server_multi_process.c
SRCS += server_multiplex.c
OBJS  = $(SRCS:%.c=$(BUILD)/%)

all: dir $(OBJS)

dir:
	mkdir -p $(BUILD)

$(BUILD)/%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BUILD)
