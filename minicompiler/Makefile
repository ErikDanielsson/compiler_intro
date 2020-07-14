CC = gcc-10
CFLAGS = -Wall
DEPS =
OBJ= parser.o lexer.o symbol_table.o table_generator.o

disassemble: $(wildcard *.out)
	objdump -d --disassembler-options=intel-mnemonic $< \
	>$(basename $<).asm



%.o : %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.out : %.o
	$(CC) -o $@ $<


a.out : $(OBJ)
	$(CC) $(CFLAGS) -o  a.out $^
	rm -f *.o

a_op.out : $(OBJ)
	$(CC) $(CFLAGS) -o  a_op.out $^
	rm -f *.o



clean:
	rm -f disassembled.asm
	rm -f *.out
	rm -f *.o