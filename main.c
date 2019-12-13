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
  locals = NULL;
  program();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", locals ? locals->offset : 0);

  for(Node *cur = nodes; cur; cur = cur->next) {
    gen(cur);

    printf("  pop rax\n");
  }

  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}

