CC = gcc
CFLAGS = -Wall
DEPS =
OBJ=


%.o : %.c
	gcc -c -o $@ $<

%.out : %.o
	gcc -o $@ $<


a.out : $(OBJ)
	gcc $(CFLAGS) -o  $@ $^


clean:
	rm -f *.out
	rm -f *.o
