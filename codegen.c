#include <stdio.h>
#include <string.h>

#include "seacc.h"

// (r|e)sp, (r|e)bp is excluded: always use rsp, rbp
char reg_table[5][6][4] = {
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

char *reg(int size, RegKind kind) {
  if(size == 1) return reg_table[0][kind];
  if(size == 2) return reg_table[1][kind];
  if(size == 4) return reg_table[2][kind];
  if(size == 8) return reg_table[3][kind];
  return "WRONG_REG";
}

void to_64bit(int size, RegKind kind) {
  if(size == 8) return;
  printf("  movsx %%%s, %%%s\n", reg(size, kind), reg(8, kind));
  return;
}

char *mov(int size) {
  if(size == 1) return "movb";
  if(size == 2) return "movw";
  if(size == 4) return "movl";
  if(size == 8) return "movq";
  return "WRONG_MOV";
}

int size_of(struct Node *node) {
  struct Type *typ = calc_type(node);
  if(typ->ty == STRUCT) return find_struct(typ->struct_union->name)->size;
  if(typ->ty == UNION) return find_union(typ->struct_union->name)->size;
  return typ->size;
}

char *mov_node(struct Node *node) {
  return mov(size_of(node));
}

char *reg_node(struct Node *node, RegKind kind) {
  return reg(size_of(node), kind);
}

void gen(struct Node *node);

void gen_lit(struct StringLiteral *lit) {
  printf(".Lstr%d:\n", lit->id);
  printf("  .string \"%s\"\n", lit->str);
}

void gen_global(struct Var *var) {
  char vname[var->len+1];
  strncpy(vname, var->name, var->len);
  vname[var->len] = '\0';
  printf(".global %s\n", vname);
  printf("%s:\n", vname);
  if(var->initial) printf("  .long %ld\n", var->initial);
  else {
    if(var->type->ty == STRUCT) {
      struct StructUnion *su = find_struct(var->type->struct_union->name);
      printf("  .zero %d\n", su->size);
    }
    else if(var->type->ty == UNION) {
      struct StructUnion *su = find_union(var->type->struct_union->name);
      printf("  .zero %d\n", su->size);
    }
    else printf("  .zero %d\n", var->type->size);
  }
}

void gen_lval(struct Node *node) {
  enum NodeKind kind = node->kind;
  if(kind == ND_DEREF) {
    gen(node->lhs);
    return;
  }
  if(kind == ND_LVAR) {
    printf("  movq %%rbp, %%rax\n");
    printf("  sub $%d, %%rax\n", node->offset);
    printf("  pushq %%rax\n");
    return;
  }
  if(kind == ND_GVAR) {
    struct Var *var = node->var;
    char vname[var->len+1];
    strncpy(vname, var->name, var->len);
    vname[var->len] = '\0';
    printf("  pushq %s@GOTPCREL(%%rip)\n", vname);
    return;
  }
  if(node->kind == ND_DOT) {
    gen_lval(node->lhs);
    struct Type *typ = calc_type(node->lhs);
    printf("  popq %%rax\n");
    if(typ->ty == STRUCT)
      printf("  add $%d, %%rax\n",
          find_member(find_struct(typ->struct_union->name),
          node->rhs->token)->offset);
    printf("  pushq %%rax\n");
    return;
  }
  error("代入の左辺が変数ではありません");
}

void gen_if(struct Node *node, int id) {
  if(node->elsebody) {
    gen(node->cond);
    printf("  popq %%rax\n");
    printf("  cmp $0, %%rax\n");
    printf("  je  .Lelse%d\n", id);
    gen(node->body);
    printf("  jmp .Lend%d\n", id);
    printf(".Lelse%d:\n", id);
    gen(node->elsebody);
    printf(".Lend%d:\n", id);
  }
  else {
    gen(node->cond);
    printf("  popq %%rax\n");
    printf("  cmp $0, %%rax\n");
    printf("  je  .Lend%d\n", id);
    gen(node->body);
    printf(".Lend%d:\n", id);
  }
}

void gen_while(struct Node *node, int id) {
  printf(".Lbegin%d:\n", id);
  gen(node->cond);
  printf("  popq %%rax\n");
  printf("  cmp $0, %%rax\n");
  printf("  je  .Lend%d\n", id);
  gen(node->body);
  printf("  jmp .Lbegin%d\n", id);
  printf(".Lend%d:\n", id);
}

void gen_for(struct Node *node, int id) {
  if(node->lhs) gen(node->lhs);
  printf(".Lbegin%d:\n", id);
  if(node->cond) gen(node->cond);
  else printf("  pushq $1\n");
  printf("  popq %%rax\n");
  printf("  cmp $0, %%rax\n");
  printf("  je  .Lend%d\n", id);
  gen(node->body);
  if(node->rhs) gen(node->rhs);
  printf("  jmp .Lbegin%d\n", id);
  printf(".Lend%d:\n", id);
}

void gen_init_array(struct Node *node, int elem_size) {
  printf("  popq %%rax\n");
  printf("  pushq %%rax\n");
  int count=0;
  for(struct Node *cur=node; cur; cur=cur->rhs) {
    gen(cur->lhs);
    printf("  popq %%rdi\n");
    printf("  popq %%rax\n");
    printf("  %s %%%s, %d(%%rax)\n",
        mov(elem_size),
        reg(elem_size, RK_DI),
        count*elem_size);
    count++;
    printf("  pushq %%rax\n");
  }
  return;
}

int label_id = 0;

void gen(struct Node *node) {
  if(node->kind == ND_BLOCK) {
    struct Node *cur = node->body;
    while(cur) {
      gen(cur);
      switch(cur->kind) {
        case ND_IF:
        case ND_WHILE:
        case ND_FOR:
          break;
        default:
          printf("  popq %%rax\n");
      }
      cur = cur->next;
    }
    printf("  pushq %%rax\n");
    return;
  }
  if(node->kind == ND_CALL) {
    char fname[255];
    strncpy(fname, node->func->name, node->func->len);
    fname[node->func->len] = '\0';
    int padding = 16-(nodes->offset%16);

    RegKind arg_reg[4] = {RK_DI, RK_SI, RK_DX, RK_CX};
    struct Node *cur = node->func->args;
    int imax;
    for(imax=0; imax<4; imax++) {
      if(!cur) break;
      gen(cur);
      printf("  popq %%rax\n");
      to_64bit(size_of(cur), RK_AX);
      printf("  pushq %%rax\n");
      cur = cur->next;
    }
    for(int i=imax-1; i>=0; i--) {
      printf("  popq %%%s\n", reg(8, arg_reg[i]));
    }

    printf("  sub $%d, %%rsp\n", padding);
    printf("  movb $0, %%al\n");
    printf("  call %s@PLT\n", fname);
    printf("  add $%d, %%rsp\n", padding);
    printf("  pushq %%rax\n");
    return;
  }
  if(node->kind == ND_DEFUN) {
    char fname[255];
    struct Function *func = node->func;
    if(!func->body) return;
    strncpy(fname, func->name, func->len);
    fname[func->len] = '\0';
    printf(".global %s\n", fname);
    printf("%s:\n", fname);

    int offset = node->offset;
    if(func->args) offset += func->args->offset;

    // prologue
    printf("  pushq %%rbp\n");
    printf("  movq %%rsp, %%rbp\n");
    printf("  sub $%d, %%rsp\n", offset);

    char *param_reg[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    struct Var *cur = func->params;
    for(int i=0; i<6; i++) {
      if(!cur) break;
      if(!cur->type) break;
      int size = cur->type->size;
      printf("  movq %%%s, %%rax\n", param_reg[i]);
      printf("  %s %%%s, -%d(%%rbp)\n", mov(size), reg(size, RK_AX), cur->offset);
      cur = cur->next;
    }

    gen(func->body);

    // epilogue
    printf("  movq %%rbp, %%rsp\n");
    printf("  popq %%rbp\n");
    printf("  ret\n");
    return;
  }
  if(node->kind == ND_DEREF) {
    gen(node->lhs);
    struct Type *type = calc_type(node->lhs)->ptr_to;
    int size;
    if(type->ty == ARY) size = 8;
    else size = type->size;
    printf("  popq %%rax\n");
    printf("  %s (%%rax), %%%s\n", mov(size), reg(size, RK_AX));
    printf("  pushq %%rax\n");
    return;
  }


  if(node->kind == ND_DOT) {
    gen_lval(node);
    printf("  popq %%rax\n");
    struct Type *typ = calc_type(node->lhs);
    struct Var *member;
    if(typ->ty == STRUCT)
      member = find_member(
          find_struct(typ->struct_union->name),
          node->rhs->token);
    else
      member = find_member(
          find_union(typ->struct_union->name),
          node->rhs->token);
    printf("  %s (%%rax), %%%s\n", mov(member->type->size), reg(member->type->size, RK_AX));
    printf("  pushq %%rax\n");
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
      printf("  popq %%rax\n");
      printf("  movq %%rbp, %%rsp\n");
      printf("  popq %%rbp\n");
      printf("  ret\n");
      return;
    case ND_NUM:
      printf("  pushq $%d\n", node->val);
      return;
    case ND_STR:
      printf("  pushq .Lstr%d@GOTPCREL(%%rip)\n", node->id);
      return;
    case ND_LVAR:
    case ND_GVAR:
      gen_lval(node);
      if(node->var->type->ty == ARY) return;
      printf("  popq %%rax\n");
      printf("  %s (%%rax), %%%s\n", mov_node(node), reg_node(node, RK_AX));
      to_64bit(size_of(node), RK_AX);
      printf("  pushq %%rax\n");
      return;
    case ND_INIT:
    case ND_ASSIGN:
      gen_lval(node->lhs);
      if(node->rhs->kind == ND_INIT_ARRAY) {
        gen_init_array(node->rhs, calc_type(node->lhs)->ptr_to->size);
        return;
      }

      struct Type *ltype = calc_type(node->lhs);
      // TODO: need more strict struct comparation
      if(ltype->ty == STRUCT && ltype->size == calc_type(node->rhs)->size) {
        gen_lval(node->rhs);
        int offset = 0;
        printf("  popq %%rdi\n");
        printf("  popq %%rax\n");
        for(;;) {
          int diff = ltype->size - offset;
          if(diff >= 8) {
            printf("  movq %%rcx, %d(%%rdi)\n", offset);
            printf("  movq %d(%%rax), %%rcx\n", offset);
            offset += 8;
            continue;
          }
          if(diff >= 4) {
            printf("  movq %%ecx, %d(%%rdi)\n", offset);
            printf("  movq %d(%%rax), %%ecx\n", offset);
            offset += 4;
            continue;
          }
          if(diff >= 2) {
            printf("  movq %%cx, %d(%%rdi)\n", offset);
            printf("  movq %d(%%rax), %%cx\n", offset);
            offset += 2;
            continue;
          }
          if(diff == 1) {
            printf("  movq %%cl, %d(%%rdi)\n", offset);
            printf("  movq %d(%%rax), %%cl\n", offset);
            offset += 1;
            break;
          }
          break;
        }
        printf("  pushq %%rax\n");
        return;
      }

      gen(node->rhs);
      printf("  popq %%rdi\n");
      printf("  popq %%rax\n");
      printf("  %s %%%s, (%%rax)\n", mov_node(node), reg_node(node, RK_DI));
      printf("  %s %%%s, %%%s\n", mov_node(node), reg_node(node, RK_DI), reg_node(node, RK_AX));
      to_64bit(size_of(node), RK_AX);
      printf("  pushq %%rax\n");
      return;
    case ND_ADDR:
      gen_lval(node->lhs);
      return;
    default:
      break;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  popq %%rdi\n");
  printf("  popq %%rax\n");

  // TODO: see node->rhs
  struct Type *type = calc_type(node->lhs);
  int size;
  if(type->ty == ARY) size = 8;
  else size = type->size;

  char *lreg = reg(size, RK_DI);
  char *rreg = reg(size, RK_AX);

  char *cqo = "  WRONG CQO\n";
  switch(size_of(node)) {
    case 1:
      cqo="  movsx %al, %ax\n  cqo\n";
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
      printf("  add %%%s, %%%s\n", lreg, rreg);
      break;
    case ND_SUB:
      printf("  sub %%%s, %%%s\n", lreg, rreg);
      break;
    case ND_MUL:
      to_64bit(size, RK_AX);
      to_64bit(size, RK_DI);
      printf("  imul %%rdi, %%rax\n");
      break;
    case ND_DIV:
      printf("%s", cqo);
      printf("  idiv %%%s\n", lreg);
      break;
    case ND_EQ:
      printf("  cmp %%%s, %%%s\n", lreg, rreg);
      printf("  sete %%al\n");
      printf("  movzb %%al, %%rax\n");
      break;
    case ND_NE:
      printf("  cmp %%%s, %%%s\n", lreg, rreg);
      printf("  setne %%al\n");
      printf("  movzb %%al, %%rax\n");
      break;
    case ND_LT:
      printf("  cmp %%%s, %%%s\n", lreg, rreg);
      printf("  setl %%al\n");
      printf("  movzb %%al, %%rax\n");
      break;
    case ND_LE:
      printf("  cmp %%%s, %%%s\n", lreg, rreg);
      printf("  setle %%al\n");
      printf("  movzb %%al, %%rax\n");
      break;
    default:
      error("Unexpected binary operator");
  }
  to_64bit(size, RK_AX);

  printf("  pushq %%rax\n");
}

