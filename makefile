all: example example2

example: example.c json.c
	gcc -g -std=c99 -Wall -pedantic -Werror example.c json.c -o example

example2: example2.c json.c
	gcc -g -std=c99 -Wall -pedantic -Werror example2.c json.c -o example2

clean:
	rm -f example
	rm -f example2
	rm -f out.json
