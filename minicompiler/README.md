# Pyrate to x86 assembly
A small (and not fully finished) compiler from my language pyrate to
x86 assembly. Pyrate is a C-like language based on the Swedish 
language game "Rövarspråket": every consonant of every word is doubled 
and an 'o' is inserted in between.
To compile the compiler run `make compiler`. 

Run `./compiler <file>` to compile the pyrate file to assembly.
However, you need an assembler for Intel syntax assembly to translate
the .asm file to a binary file, for example: https://yasm.tortall.net/. 

DISCLAIMER: Neither functions nor IO is fully supported -- though you
can compile a file with functions they are not translated to assembly.
Hence you will have to execute your program in gdb :).

An example of a pyrate program is given in fib.pyr. For the full syntax
read the files 'tokens' in 'lexing/' and 'grammar' in 'parsing/'. 
