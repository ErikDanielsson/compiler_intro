CC = clang
CFLAGS = -Wall -O3
DEPS =
OBJ= io.o IC_gen.o lexer.o keyword_table.o table_generator.o parser.o symbol_table.o type_checker.o hashing.o  main.o

all:
	make table
	make a.out

table:
	python ../shitty_compiler_compiler.py grammar -w parsing_table.txt token_types.txt node_types.txt -c -s

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
	rm -f *.tmp

new_profile:
	make clean
	make a.out
	./a.out test.pyr
	gprof a.out gmon.out > analysis.txt
	cat analysis.txt
