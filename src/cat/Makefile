CC = gcc
CFLAGS = -Wall -Wextra -Werror

all: s21_cat

s21_cat: clean
	$(CC) $(CFLAGS) s21_cat.c -o s21_cat 

test: s21_cat
	bash tests.sh

clean:
	rm -f s21_cat