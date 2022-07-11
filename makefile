main:
	gcc --std=gnu99 -Wall -g -o smallsh main.c command.c

clean:
	rm -rf smallsh