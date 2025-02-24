CC = gcc
CFLAGS = -Wall -Wextra -Werror

all: s21_grep

s21_grep: clean
	$(CC) $(CFLAGS) s21_grep.c -o s21_grep

test: s21_grep
	bash tests.sh

clean:
	rm -f s21_grep