CC = clang
CFLAGS = -Wall
SUBDIR = gen lexing parsing types IC
DEPS = includes/
GEN_OBJ = gen/io.o gen/hashing.o
LEX_OBJ = lexing/keyword_table.o lexing/lexer.o
PARSE_OBJ = parsing/table_generator.o parsing/parser.o
TYPE_OBJ = types/type_checker.o types/type_table.o types/symbol_table.o
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


a.out : $(GEN_OBJ) $(LEX_OBJ) $(PARSE_OBJ) $(TYPE_OBJ) $(IC_OBJ) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o  a.out $^

DIRS := . $(shell find . -type d)
GARBAGE_PATTERNS := disassembled.asm *.out *.o *.tmp
GARBAGE := $(foreach DIR,$(DIRS),$(addprefix $(DIR)/,$(GARBAGE_PATTERNS)))

clean:
	rm -f $(GARBAGE)

new_profile:
	make clean
	make a.out
	./a.out big.pyr
	gprof a.out gmon.out > analysis.txt
	cat analysis.txt
