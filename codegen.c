#include <string.h>

#include "seacc.h"

#define SIZEOF(Node) (calc_type(Node)->size)
#define REG_NODE(Node, Kind) (reg(SIZEOF(Node), Kind))
#define PSIZE_NODE(Node) (psize(SIZEOF(Node)))

// (r|e)sp, (r|e)bp is excluded: always use rsp, rbp
char reg_table[][6][4] = {
  { "al", "dil", "sil",  "dl",  "cl",  "bl"},
  { "ax",  "di",  "si",  "dx",  "cx",  "bx"},
  {"eax", "edi", "esi", "edx", "ecx", "ebx"},
  {"rax", "rdi", "rsi", "rdx", "rcx", "rbx"}};

typedef enum {
  RK_AX,
  RK_DI,
  RK_SI,
  RK_DX,
  RK_CX,
  RK_BX,
} RegKind;

void any_reg_to_r_reg(char *reg, RegKind kind) {
  if(strcmp(reg, reg_table[3][kind]) == 0) return;
  printf("  movsx %s, %s\n", reg_table[3][kind], reg);
  return;
}

char *reg(int size, RegKind kind) {
  if(size == 1) return reg_table[0][kind];
  if(size == 2) return reg_table[1][kind];
  if(size == 4) return reg_table[2][kind];
  if(size == 8) return reg_table[3][kind];
  return "WRONG_REG";
}

char *psize(int size) {
  if(size == 1) return "BYTE PTR";
  if(size == 2) return "WORD PTR";
  if(size == 4) return "DWORD PTR";
  if(size == 8) return "QWORD PTR";
  return "WRONG PTR";
}

void gen(Node *node);

void gen_lit(StringLiteral *lit) {
  printf(".Lstr%d:\n", lit->id);
  printf("  .string \"%s\"\n", lit->str);
}

void gen_global(Var *var) {
  char vname[var->len+1];
  strncpy(vname, var->name, var->len);
  vname[var->len] = '\0';
  printf("_%s:\n", vname);
  if(var->initial) printf("  .long %ld\n", var->initial);
  else printf("  .zero %d\n", var->type->size);
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
  if(kind == ND_GVAR) {
    Var *var = node->var;
    char vname[var->len+1];
    strncpy(vname, var->name, var->len);
    vname[var->len] = '\0';
    printf("  push _%s@GOTPCREL[rip]\n", vname);
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
    printf("  call %s@PLT\n", fname);
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
      int size = cur->type->size;
      printf("  mov rax, %s\n", param_reg[i]);
      printf("  mov %s [rbp-%d], %s\n", psize(cur->type->size), cur->offset, reg(size, RK_AX));
      cur = cur->next;
    }

    gen(func->body);

    // epilogue
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }
  if(node->kind == ND_DEREF) {
    gen(node->lhs);
    Type *type = calc_type(node->lhs);
    int size;
    if(type->ty == ARY) size = 8;
    else size = type->size;
    printf("  pop rax\n");
    printf("  mov %s, %s [rax]\n", reg(size, RK_AX), psize(size));
    printf("  push rax\n");
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
    // XXX: RETURN TYPE IS ALWAYS INT NOW
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
    case ND_STR:
      printf("  push .Lstr%d@GOTPCREL[rip]\n", node->id);
      return;
    case ND_LVAR:
    case ND_GVAR:
      gen_lval(node);
      if(node->var->type->ty == ARY) return;
      printf("  pop rax\n");
      printf("  mov %s, %s [rax]\n", REG_NODE(node, RK_AX), PSIZE_NODE(node));
      any_reg_to_r_reg(REG_NODE(node, RK_AX), RK_AX);
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov %s [rax], %s\n", PSIZE_NODE(node), REG_NODE(node, RK_DI));
      printf("  mov %s, %s\n", REG_NODE(node, RK_AX), REG_NODE(node, RK_DI));
      any_reg_to_r_reg(REG_NODE(node, RK_AX), RK_AX);
      printf("  push rax\n");
      return;
    case ND_ADDR:
      gen_lval(node->lhs);
      return;
    default:
      break;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  // TODO: see node->rhs
  Type *type = calc_type(node->lhs);
  int size;
  if(type->ty == ARY) size = 8;
  else size = type->size;

  char *lreg = reg(size, RK_AX);
  char *rreg = reg(size, RK_DI);

  char *cqo = "  WRONG CQO\n";
  switch(SIZEOF(node)) {
    case 1:
      cqo="  movsx ax, al\n  cqo\n";
      break;
    case 2:
    case 4:
    case 8:
      cqo="  cqo\n";
      break;
    default:
      break;
  }

  switch(node->kind) {
    case ND_ADD:
      printf("  add %s, %s\n", lreg, rreg);
      break;
    case ND_SUB:
      printf("  sub %s, %s\n", lreg, rreg);
      break;
    case ND_MUL:
      any_reg_to_r_reg(lreg, RK_AX);
      any_reg_to_r_reg(rreg, RK_DI);
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("%s", cqo);
      printf("  idiv %s\n", rreg);
      break;
    case ND_EQ:
      printf("  cmp %s, %s\n", lreg, rreg);
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp %s, %s\n", lreg, rreg);
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp %s, %s\n", lreg, rreg);
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp %s, %s\n", lreg, rreg);
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }
  any_reg_to_r_reg(lreg, RK_AX);

  printf("  push rax\n");
}

