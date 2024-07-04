CC=gcc
CFLAGS=-Wall -Wextra -std=c11
DIST_DIR=dist

all: $(DIST_DIR)/lsh

$(DIST_DIR)/lsh: src/main.c | $(DIST_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(DIST_DIR):
	mkdir -p $(DIST_DIR)

clean:
	rm -rf $(DIST_DIR)