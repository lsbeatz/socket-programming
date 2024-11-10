CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

BUILD = build

SRCS += server.c
SRCS += client.c
OBJS  = $(SRCS:%.c=$(BUILD)/%)

all: dir $(OBJS)

dir:
	mkdir -p $(BUILD)

$(BUILD)/%: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILD)
