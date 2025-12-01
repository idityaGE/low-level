CC = gcc
CFLAGS = -Wall -Wextra -g -pthread
# -Wall → enable all common warnings
# -Wextra → enable extra warnings
# -g → include debugging info for gdb

TARGET = server client server_pool

all: $(TARGET) # `make` == `make all` 

# If you don't understand ask AI to explain below pattern
%: %.c
	$(CC) $(CFLAGS) $< -o $@

run-server: server # this depend on server so it will run `make server` which will be caught by above pattern and it will run 
	./server

run-server-pool: server_pool
	./server_pool

run-client: client
	./client

clean:
	rm -f $(TARGET)

# .PHONY tells make:
# These names are not files.
# Always run the commands, do NOT check timestamps.
.PHONY: all clean run-server run-client run-server-pool