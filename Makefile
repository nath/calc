calc: calc.c
	gcc -o calc calc.c -Wall -lm

clean:
	rm calc
