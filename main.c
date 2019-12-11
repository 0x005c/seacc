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
  Node *node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}

