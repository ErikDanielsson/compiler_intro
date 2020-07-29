CC = gcc-10
CFLAGS = -Wall
DEPS =
OBJ= parser.o lexer.o keyword_table.o table_generator.o hashing.o

all:
	python ../shitty_compiler_compiler.py grammar -w parsing_table.txt token_types.txt node_types.txt -c -s
	make a.out


disassemble: $(wildcard *.out)
	objdump -d --disassembler-options=intel-mnemonic $< \
	>$(basename $<).asm



%.o : %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.out : %.o
	$(CC) -o $@ $<


a.out : $(OBJ)
	$(CC) $(CFLAGS) -o  a.out $^


clean:
	rm -f disassembled.asm
	rm -f *.out
	rm -f *.o
