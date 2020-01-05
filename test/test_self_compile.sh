#!/bin/bash
./seacc 'main.c' > main.s
gcc -o seacc -fpack-struct=1 main.s error.c codegen.c parser.c preprocessor.c reader.c tokenizer.c
