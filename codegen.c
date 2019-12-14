#include <string.h>

#include "seacc.h"

void gen_node(Node *node);

void gen_lval(Node *node) {
  if(node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen_if(Node *node, int id) {
  if(node->elsebody) {
    gen_node(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lelse%d\n", id);
    gen_node(node->body);
    printf("  jmp .Lend%d\n", id);
    printf(".Lelse%d:\n", id);
    gen_node(node->elsebody);
    printf(".Lend%d:\n", id);
  }
  else {
    gen_node(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lend%d\n", id);
    gen_node(node->body);
    printf(".Lend%d:\n", id);
  }
}

void gen_while(Node *node, int id) {
  printf(".Lbegin%d:\n", id);
  gen_node(node->cond);
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  printf("  je  .Lend%d\n", id);
  gen_node(node->body);
  printf("  jmp .Lbegin%d\n", id);
  printf(".Lend%d:\n", id);
}

void gen_for(Node *node, int id) {
  if(node->lhs) gen_node(node->lhs);
  printf(".Lbegin%d:\n", id);
  if(node->cond) gen_node(node->cond);
  else printf("  push 1\n");
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  printf("  je  .Lend%d\n", id);
  gen_node(node->body);
  if(node->rhs) gen_node(node->rhs);
  printf("  jmp .Lbegin%d\n", id);
  printf(".Lend%d:\n", id);
}

int label_id = 0;

void gen_node(Node *node) {
  if(node->kind == ND_RETURN) {
    gen_node(node->lhs);
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
  if(node->kind == ND_BLOCK) {
    Node *cur = node->body;
    while(cur) {
      gen_node(cur);
      cur = cur->next;
      printf("  pop rax\n");
    }
    printf("  push rax\n");
    return;
  }
  if(node->kind == ND_CALL) {
    char fname[255];
    strncpy(fname, node->func->name, node->func->len);
    fname[node->func->len] = '\0';
    int padding = functions->locals ? (16-(functions->locals->offset%16)) % 16 : 0;
    printf("  sub rsp, %d\n", padding);
    printf("  call %s\n", fname);
    printf("  add rsp, %d\n", padding);
    printf("  push rax\n");
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
      gen_node(node->rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
  }

  gen_node(node->lhs);
  gen_node(node->rhs);

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

void gen(Function *func) {
  char fname[255];
  strncpy(fname, func->name, func->len);
  fname[func->len] = '\0';
  printf("%s:\n", fname);

  // prologue
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", functions->locals ? functions->locals->offset : 0);

  gen_node(func->body);

  // epilogue
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

