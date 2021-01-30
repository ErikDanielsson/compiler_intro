CC = clang
CFLAGS = -Wall -g
DIRS := . $(shell find . -type d)
DEPS = includes/
GEN_OBJ = gen/io.o gen/hashing.o
LEX_OBJ = lexing/keyword_table.o lexing/lexer.o
PARSE_OBJ = parsing/table_generator.o parsing/parser.o parsing/treedrawer.o
TYPE_OBJ = types/type_checker.o types/type_table.o types/symbol_table.o types/constant_table.o
IC_OBJ = IC/tree_climber.o  IC/IC_table.o IC/intermediate_code.o
CODEGEN_OBJ = final/live_and_use.o final/write_asm.o final/code_generation.o final/registers.o final/instruction_set.o 
MAIN_OBJ =  main.o

all:
	make table
	make compiler

table:
	cd parsing && python shitty_compiler_compiler.py grammar -w parsing_table.txt ../lexing/token_types.txt node_types.txt -c -s


disassemble: $(wildcard *.out)
	objdump -d -s --disassembler-options=intel-mnemonic $< \
	>$(basename $<).asm

%.asm : %.pyr
	./compiler $<

%.o : %.c
	$(CC) -c -I $(DEPS) $(CFLAGS) -o $@ $<

%.o : %.asm
	yasm -Worphan-labels -g dwarf2 -f elf64 $<

%.out : %.o
	$(CC) -o  $@ $<


compiler : $(GEN_OBJ) $(LEX_OBJ) $(PARSE_OBJ) $(TYPE_OBJ) $(IC_OBJ) $(CODEGEN_OBJ) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o  compiler $^

GARBAGE_PATTERNS := compiler disassembled.asm *.out *.o *.tmp .*.dot *.ll *.s *.gch parsing_table.txt
GARBAGE := $(foreach DIR,$(DIRS),$(addprefix $(DIR)/,$(GARBAGE_PATTERNS)))

clean:
	rm -f $(GARBAGE)

new_profile:
	make clean
	make compiler
	./compiler big.pyr
	gprof compiler gmon.out > analysis.txt
	cat analysis.txt

TEMPLATE = simple
SUBDIR =
%.c :
	cp ~/dev/ed_templates/c/$(TEMPLATE).c $(SUBDIR)/$(basename $@).c
	touch $(SUBDIR)/$(basename $@).h
	cd includes && ln -s ../$(SUBDIR)/$(basename $@).h


%.dot : %.c
	clang -emit-llvm -S -I $(DEPS) $< -o $(basename $@).ll
	opt -f --dot-cfg $(basename $@).ll
%.png : %.gv
	dot -Tpng  $< -o $@

cfg:
	CFLAGS= -emit-llvm -S

cl:
	find . -name '*.[py|c|h]' -type f | xargs wc
