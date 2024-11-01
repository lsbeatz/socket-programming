CC = gcc
CFLAGS = -g -Wall

BUILD = build

SRCS  = client.c
SRCS += server_single.c
OBJS  = $(SRCS:%.c=$(BUILD)/%)

all: dir $(OBJS)

dir:
	mkdir -p $(BUILD)

$(BUILD)/%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BUILD)
