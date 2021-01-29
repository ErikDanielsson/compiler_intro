# Pyrate to x86 assembly
A small (and not fully finished) compiler from my language pyrate to  
x86 assembly. To compile the compiler run  
`make compiler`  
Simply run `./compiler <file>` to compile the pyrate file to assembly.
However, you need an assembler for Intel syntax assembly to translate
the .asm file to a binary file, for example: https://yasm.tortall.net/. 

DISCLAIMER: Neither functions nor IO is fully supported -- though you  
can compile a file with functions they are not translated to assembly.
Hence you will have to execute your program in gdb :).


