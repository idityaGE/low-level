CC = gcc
CFLAGS = -Wall -Wextra -g
# -Wall → enable all common warnings
# -Wextra → enable extra warnings
# -g → include debugging info for gdb

TARGET = server client

all: $(TARGET) # `make` == `make all` 

# If you don't understand ask AI to explain below pattern
%: %.c
	$(CC) $(CFLAGS) $< -o $@

run-server: server # this depend on server so it will call `make server` which will be cought by above pattern and it will run 
	./server

run-client: client
	./client

clean:
	rm -f $(TARGET)

# .PHONY tells make:
# These names are not files.
# Always run the commands, do NOT check timestamps.
.PHONY: all clean run-server run-client