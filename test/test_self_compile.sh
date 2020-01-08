#!/bin/bash
./seacc 'main.c' > main.s
./seacc 'reader.c' > reader.s
./seacc 'preprocessor.c' > preprocessor.s
gcc -o seacc -fpack-struct=1 main.s reader.s preprocessor.s error.c codegen.c parser.c tokenizer.c
