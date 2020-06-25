CC = gcc
CFLAGS = -Wall
DEPS =
OBJ=lexer.o error.o symbol_table.o

disassemble: $(wildcard *.out)
	objdump -d --disassembler-options=intel-mnemonic $< \
	>disassembled.asm



%.o : %.c
	gcc -c $(CFLAGS) -o $@ $<

%.out : %.o
	gcc -o $@ $<


a.out : $(OBJ)
	gcc $(CFLAGS) -o  a.out $^

a_op.out : $(OBJ)
	gcc $(CFLAGS) -O3 -o  a_op.out $^


clean:
	rm -f disassembled.asm
	rm -f *.out
	rm -f *.o
