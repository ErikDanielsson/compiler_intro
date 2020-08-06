CC = clang
CFLAGS = -Wall
DEPS = includes/
GEN_OBJ = gen/io.o gen/hashing.o
LEX_OBJ = lexing/keyword_table.o lexing/lexer.o
PARSE_OBJ = parsing/table_generator.o parsing/parser.o
SEM_OBJ = types/type_checker.o types/type_table.o types/symbol_table.o
IC_OBJ = IC/IC_gen.o  IC/IC_table.o
MAIN_OBJ =  main.o

all:
	make table
	make a.out

table:
	python ../shitty_compiler_compiler.py grammar -w parsing_table.txt token_types.txt node_types.txt -c -s

disassemble: $(wildcard *.out)
	objdump -d --disassembler-options=intel-mnemonic $< \
	>$(basename $<).asm



%.o : %.c
	$(CC) -c -I $(DEPS) $(CFLAGS) -o $@ $<

%.out : %.o
	$(CC) -o  $@ $<


a.out : $(GEN_OBJ) $(LEX_OBJ) $(PARSE_OBJ) $(SEM_OBJ) $(IC_OBJ) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o  a.out $^

clean:
	rm -f disassembled.asm
	rm -f *.out
	rm -f *.o
	rm -f *.tmp

new_profile:
	make clean
	make a.out
	./a.out big.pyr
	gprof a.out gmon.out > analysis.txt
	cat analysis.txt
