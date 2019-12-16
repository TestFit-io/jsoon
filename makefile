example: example.c json.c
	gcc -g -std=c99 -Wall -pedantic -Werror example.c json.c -o example

clean:
	rm -f example
	rm -f out.json
