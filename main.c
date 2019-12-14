#include <stdlib.h>

#include "seacc.h"

Token *token;

char *user_input;

int main(int argc, char *argv[]) {
  if(argc != 2) {
    error("引数の数が違います");
    return 1;
  }

  user_input = argv[1];
  token = tokenize(argv[1]);
  functions = NULL;
  program();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");

  for(Function *cur = functions; cur; cur = cur->next) gen(cur);

  return 0;
}

