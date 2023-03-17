CC= gcc
CFLAGS= -std=c99 -Werror -Wall -Wextra -Wvla -fsanitize=address -g  -D_POSIX_C_SOURCE=200112L

SRC= src/http_parser.c src/server.c src/main.c

all: clean httpd

httpd:
	$(CC) $(CFLAGS) -o httpd $(SRC)

check:
	$(CC) $(CFLAGS) -lcriterion -o tests tests/tests.c $(SRC)
	./tests

clean:
	$(RM) httpd *.o *.a *.swp
