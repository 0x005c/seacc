#include <string.h>

#include "seacc.h"

void gen(Node *node);

void gen_global(Var *var) {
  char vname[var->len+1];
  strncpy(vname, var->name, var->len);
  vname[var->len] = '\0';
  printf("%s:\n", vname);
  printf("  .zero %d\n", var->type->size);
}

void gen_lval(Node *node) {
  NodeKind kind = node->kind;
  if(kind == ND_DEREF) {
    gen(node->lhs);
    return;
  }
  if(kind == ND_LVAR) {
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
    return;
  }
  error("代入の左辺が変数ではありません");
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
  if(node->kind == ND_BLOCK) {
    Node *cur = node->body;
    while(cur) {
      gen(cur);
      switch(cur->kind) {
        case ND_IF:
        case ND_WHILE:
        case ND_FOR:
          break;
        default:
          printf("  pop rax\n");
      }
      cur = cur->next;
    }
    printf("  push rax\n");
    return;
  }
  if(node->kind == ND_CALL) {
    char fname[255];
    strncpy(fname, node->func->name, node->func->len);
    fname[node->func->len] = '\0';
    int padding = 16-(nodes->offset%16);

    char *arg_reg[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    int *args[6] = {};
    Node *cur = node->func->args;
    for(int i=0; i<6; i++) {
      if(!cur) break;
      gen(cur);
      printf("  pop %s\n", arg_reg[i]);
      cur = cur->next;
    }

    printf("  sub rsp, %d\n", padding);
    printf("  call %s\n", fname);
    printf("  add rsp, %d\n", padding);
    printf("  push rax\n");
    return;
  }
  if(node->kind == ND_DEFUN) {
    char fname[255];
    Function *func = node->func;
    strncpy(fname, func->name, func->len);
    fname[func->len] = '\0';
    printf("%s:\n", fname);

    int offset = node->offset;
    if(func->args) offset += func->args->offset;

    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", offset);

    char *param_reg[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    Var *cur = func->params;
    for(int i=0; i<6; i++) {
      if(!cur) break;
      printf("  mov [rbp-%d], %s\n", cur->offset, param_reg[i]);
      cur = cur->next;
    }

    gen(func->body);

    // epilogue
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }
  if(node->kind == ND_GVAR) {
    Var *var = node->var;
    char vname[var->len+1];
    strncpy(vname, var->name, var->len);
    vname[var->len] = '\0';
    printf("  push %s[rip]\n", vname);
    return;
  }
  if(node->kind == ND_ASSIGN) {
    if(node->lhs->kind == ND_GVAR) {
      Var *var = node->lhs->var;
      char vname[var->len+1];
      strncpy(vname, var->name, var->len);
      vname[var->len] = '\0';
      gen(node->rhs);
      printf("  pop rax\n");
      printf("  mov %s[rip], rax\n", vname);
      return;
    }
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  }

  switch(node->kind) {
    case ND_IF:
      gen_if(node, label_id++);
      return;
    case ND_WHILE:
      gen_while(node, label_id++);
      return;
    case ND_FOR:
      gen_for(node, label_id++);
      return;
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      if(node->var->type->ty == ARY) return;
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ADDR:
      gen_lval(node->lhs);
      return;
    case ND_DEREF:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    default:
      break;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  Type *lty, *rty;
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

