#!/bin/bash
./seacc 'main.c' > main.s
./seacc 'reader.c' > reader.s
gcc -o seacc -fpack-struct=1 main.s reader.s error.c codegen.c parser.c preprocessor.c tokenizer.c
