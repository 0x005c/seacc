#include "seacc.h"

void gen_lval(Node *node) {
  if(node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen_if(Node *node, int id) {
  if(node->elsebody) {
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lelse%d\n", id);
    gen(node->body);
    printf("  jmp .Lend%d\n", id);
    printf(".Lelse%d:\n", id);
    gen(node->elsebody);
    printf(".Lend%d:\n", id);
  }
  else {
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lend%d\n", id);
    gen(node->body);
    printf(".Lend%d:\n", id);
  }
}

void gen_while(Node *node, int id) {
  printf(".Lbegin%d:\n", id);
  gen(node->cond);
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  printf("  je  .Lend%d\n", id);
  gen(node->body);
  printf("  jmp .Lbegin%d\n", id);
  printf(".Lend%d:\n", id);
}

void gen_for(Node *node, int id) {
  if(node->lhs) gen(node->lhs);
  printf(".Lbegin%d:\n", id);
  if(node->cond) gen(node->cond);
  else printf("  push 1\n");
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  printf("  je  .Lend%d\n", id);
  gen(node->body);
  if(node->rhs) gen(node->rhs);
  printf("  jmp .Lbegin%d\n", id);
  printf(".Lend%d:\n", id);
}

int label_id = 0;

void gen(Node *node) {
  if(node->kind == ND_RETURN) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }
  if(node->kind == ND_IF) {
    gen_if(node, label_id++);
    return;
  }
  if(node->kind == ND_WHILE) {
    gen_while(node, label_id++);
    return;
  }
  if(node->kind == ND_FOR) {
    gen_for(node, label_id++);
    return;
  }

  switch(node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch(node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}

