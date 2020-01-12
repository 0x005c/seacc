#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "seacc.h"

int ty_size(enum TType ty) {
  switch(ty) {
    case CHAR:
      return 1;
    case INT:
      return 4;
    case LONG:
    case PTR:
      return 8;
    default:
      error("cannot calculate type size");
      return -1; // not reached
  }
}

bool ptr_like(struct Node *node) {
  struct Type *t = calc_type(node);
  if(t->ty == PTR || t->ty == ARY) return true;
  return false;
}

struct Type anonymous_char = {.ty = CHAR, .size = 1};
struct Type anonymous_char_ptr = {.ty = PTR, .size = 8, .ptr_to = &anonymous_char};
struct Type anonymous_int = {.ty = INT, .size = 4};
struct Type anonymous_long = {.ty = LONG, .size = 8};

struct Type *larger_type(struct Type *a, struct Type *b) {
  return a->ty > b->ty ? a : b;
}

struct Type *gen_type(enum TType ty, struct Type *ptr_to, int size) {
  struct Type *type = calloc(1, sizeof(struct Type));
  type->ty = ty;
  type->ptr_to = ptr_to;
  type->size = size;
  return type;
}

struct Type *calc_type(struct Node *node) {
  struct Type *l, *r, *type;
  switch(node->kind) {
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
      l = calc_type(node->lhs);
      r = calc_type(node->rhs);
      if(l->ty == ARY) {
        struct Type *typ = calloc(1, sizeof(struct Type));
        typ->ty = PTR;
        typ->ptr_to = l->ptr_to;
        typ->size = 8;
        return typ;
      }
      else if(r->ty == ARY) {
        struct Type *typ = calloc(1, sizeof(struct Type));
        typ->ty = PTR;
        typ->ptr_to = r->ptr_to;
        typ->size = 8;
        return typ;
      }
      else return larger_type(l, r);
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
    case ND_LAND:
    case ND_LOR:
      return &anonymous_char;
    case ND_LVAR:
    case ND_GVAR:
      return node->var->type;
    case ND_INIT:
    case ND_ASSIGN:
      return calc_type(node->lhs);
    case ND_CALL:
      return node->func->type;
    case ND_ADDR:
      return type = gen_type(PTR, calc_type(node->lhs), ty_size(PTR));
    case ND_NUM:
      return &anonymous_long;
    case ND_DEREF:
      return calc_type(node->lhs)->ptr_to;
    case ND_DOT: {
      struct Type *typ = calc_type(node->lhs);
      struct StructUnion *su;
      if(typ->ty == STRUCT) su = find_struct(typ->struct_union->name);
      else su = find_union(typ->struct_union->name);
      return find_member(su, node->rhs->token)->type;
     }
    case ND_STR:
      return &anonymous_char_ptr;
    case ND_NOT:
      return &anonymous_int;
    default:
      error("Cannot calculate type on compiliation");
      return NULL; // not reached
  }
}

int compute_const_expr(struct Node *exp) {
  switch(exp->kind) {
    case ND_ADD:
      return compute_const_expr(exp->lhs) + compute_const_expr(exp->rhs);
    case ND_SUB:
      return compute_const_expr(exp->lhs) - compute_const_expr(exp->rhs);
    case ND_MUL:
      return compute_const_expr(exp->lhs) * compute_const_expr(exp->rhs);
    case ND_DIV:
      return compute_const_expr(exp->lhs) / compute_const_expr(exp->rhs);
    case ND_NUM:
      return exp->val;
    case ND_LVAR:
    case ND_CALL:
    case ND_DEREF:
      error("struct Variable length array is not supported");
    case ND_ASSIGN:
      return compute_const_expr(exp->rhs);
    default:
      error("Unexpected node. Cannot compute expression value on compile");
      return -1;
  }
}

int consume(char *op) {
  if(token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      strcmp(token->str, op))
    return false;
  token = token->next;
  return true;
}

int consume_kind(enum TokenKind kind) {
  if(token->kind == kind) {
    token = token->next;
    return true;
  }
  return false;
}

bool check_specifier() {
  switch(token->kind) {
    case TK_VOID:
    case TK_INT:
    case TK_LONG:
    case TK_CHAR:
    case TK_ENUM:
    case TK_STRUCT:
    case TK_UNION:
      return true;
    default:
      return false;
  }
}

struct Token *consume_ident() {
  if(token->kind != TK_IDENT) return NULL;

  struct Token *tok = token;
  token = token->next;
  return tok;
}

void expect(char *op) {
  if(token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      strcmp(token->str, op))
    error_at(token->pos, "'%s'ではありません", op);
  token = token->next;
}

int expect_number() {
  if(token->kind != TK_NUM && token->kind != TK_CHAR_LITERAL)
    error_at(token->pos, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

struct Function *find_func(struct Token *tok) {
  for(struct Function *func = functions; func; func = func->next)
    if(func->len == tok->len && !strcmp(tok->str, func->name))
      return func;
  return NULL;
}

struct Var *find_var_scope(struct Scope *scope, struct Token *tok) {
  for(struct Var *var=scope->variables; var; var=var->next)
    if(strcmp(var->name, tok->str)==0) return var;
}

struct Node *find_var(struct Scope *scope, struct Token *tok) {
  struct Node *node = calloc(1, sizeof(struct Node));
  if(scope->parent) node->kind = ND_LVAR;
  else node->kind = ND_GVAR;
  struct Var *var = find_var_scope(scope, tok);
  if(var) {
    node->var = var;
    if(scope->parent) node->offset = var->offset;
    return node;
  }
  if(scope->parent) return find_var(scope->parent, tok);
  return NULL;
}

struct StructUnion *find_struct(char *name) {
  for(struct StructUnion *su = structs; su; su = su->next)
    if(!strcmp(name, su->name))
      return su;
  return NULL;
}

struct StructUnion *find_union(char *name) {
  for(struct StructUnion *su = unions; su; su = su->next)
    if(!strcmp(name, su->name))
      return su;
  return NULL;
}

struct Var *find_member(struct StructUnion *struct_union, struct Token *tok) {
  for(struct Var *var=struct_union->declarators; var; var=var->next)
    if(var->len == tok->len && !strcmp(tok->str, var->name))
      return var;
  return NULL;
}

struct Enum *find_enum(struct Token *tok) {
  for(struct Enum *cur=enums; cur; cur=cur->next)
    if(strcmp(tok->str, cur->name) == 0) return cur;
  return NULL;
}

struct Node *new_node(enum NodeKind kind, struct Node *lhs, struct Node *rhs) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

struct Node *new_node_num(int val) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

/* primary = "(" expr ")"
 *         | ident "(" (expr ",")* expr? ")"
 *         | ident
 *         | number
 *         | string_literal
 */
struct Node *primary() {
  if(consume("(")) {
    struct Node *node = expr();
    expect(")");
    return node;
  }

  struct Token *tok = consume_ident();
  if(tok) {
    if(consume("(")) {
      struct Function *fdef = find_func(tok);
      struct Function *func = calloc(1, sizeof(struct Function));
      if(fdef) {
        memcpy(func, fdef, sizeof(struct Function));
      }
      else {
        func->name = tok->str;
        func->len = tok->len;
        func->type = &anonymous_long;
      }
      if(consume(")")) func->args = NULL;
      else {
        struct Node head = {.next = NULL};
        struct Node *cur = &head;
        for(;;) {
          cur->next = expr();
          cur = cur->next;
          if(!consume(",")) {
            expect(")");
            break;
          }
        }
        func->args = head.next;
      }
      struct Node *node = calloc(1, sizeof(struct Node));
      node->kind = ND_CALL;
      node->next = NULL;
      node->func = func;
      return node;
    }
    else {
      struct Node *node = find_var(current_scope, tok);
      if(node) return node;
      struct Enum *e = find_enum(tok);
      if(e) return new_node_num(e->value);
      error_at(tok->pos, "Unexpected token");
    }
  }

  if(token->kind == TK_STRING_LITERAL) {
    struct Node *node = calloc(1, sizeof(struct Node));
    node->kind = ND_STR;
    node->id = token->lit->id;
    token = token->next;
    return node;
  }

  return new_node_num(expect_number());
}

struct Node *new_node_add(struct Node *l, struct Node *r) {
  bool lp = ptr_like(l);
  bool rp = ptr_like(r);
  if(lp && !rp)
    r = new_node(ND_MUL, r, new_node_num(size_of(calc_type(l)->ptr_to)));
  else if(!lp && rp)
    l = new_node(ND_MUL, l, new_node_num(size_of(calc_type(r)->ptr_to)));
  else if(lp && rp)
    // NOTE: does this behaviour follow the standard?
    error("[Compile error] cannot add pointer to pointer");
  l = new_node(ND_ADD, l, r);
  return l;
}

struct Node *new_node_sub(struct Node *l, struct Node *r) {
  bool lp = ptr_like(l);
  bool rp = ptr_like(r);
  if(lp && !rp)
    r = new_node(ND_MUL, r, new_node_num(size_of(calc_type(l)->ptr_to)));
  else if(!lp && rp)
    l = new_node(ND_MUL, l, new_node_num(size_of(calc_type(r)->ptr_to)));
  else if(lp && rp)
    // NOTE: does this behaviour follow the standard?
    error("[Compile error] cannot add pointer to pointer");
  l = new_node(ND_SUB, l, r);
  return l;
}

struct Node *new_node_name(struct Token *tok) {
  struct Node *node = new_node(ND_NAME, NULL, NULL);
  node->token = tok;
  return node;
}

/*
 * postfix = primary ("[" expr "]")*
 */
struct Node *postfix() {
  struct Node *node = primary();
  for(;;) {
    if(consume("[")) {
      node = new_node_add(node, expr());
      node = new_node(ND_DEREF, node, NULL);
      expect("]");
      continue;
    }
    if(consume(".")) {
      struct Token *tok = consume_ident();
      if(tok) {
        node = new_node(ND_DOT, node, new_node_name(tok));
        continue;
      }
      else error("identifier expected");
    }
    if(consume("->")) {
      struct Token *tok = consume_ident();
      if(tok) {
        node = new_node(ND_DEREF, node, NULL);
        node = new_node(ND_DOT, node, new_node_name(tok));
        continue;
      }
      else error("identifier expected");
    }
    return node;
  }
}

/*
 * unary = "+"? postfix
 *       | "-"? postfix
 *       | "*" unary
 *       | "&" unary
 *       | "!" unary
 *       | "++" unary
 *       | "--" unary
 *       | "sizeof" unary
 *       | postfix
 */
struct Node *unary() {
  if(consume("+"))
    return postfix();
  if(consume("-"))
    return new_node(ND_SUB, new_node_num(0), postfix());
  if(consume("*"))
    return new_node(ND_DEREF, unary(), NULL);
  if(consume("&"))
    return new_node(ND_ADDR, unary(), NULL);
  if(consume("!"))
    return new_node(ND_NOT, unary(), NULL);
  if(consume("++")) {
    struct Node *node = unary();
    return new_node(ND_ASSIGN, node,
        new_node_add(node, new_node_num(1)));
  }
  if(consume("--")) {
    struct Node *node = unary();
    return new_node(ND_ASSIGN, node,
        new_node_sub(node, new_node_num(1)));
  }
  if(consume_kind(TK_SIZEOF))
    return new_node_num(size_of(calc_type(unary())));
  return postfix();
}

struct Node *mul() {
  struct Node *node = unary();

  for(;;) {
    if(consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if(consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

struct Node *add() {
  struct Node *node = mul();
  for(;;) {
    if(consume("+"))
      node = new_node_add(node, mul());
    else if(consume("-"))
      node = new_node_sub(node, mul());
    else return node;
  }
}

struct Node *relational() {
  struct Node *node = add();
  for(;;) {
    if(consume("<"))
      node = new_node(ND_LT, node, add());
    else if(consume("<="))
      node = new_node(ND_LE, node, add());
    else if(consume(">"))
      node = new_node(ND_LT, add(), node);
    else if(consume(">="))
      node = new_node(ND_LE, add(), node);
    else return node;
  }
}

struct Node *equality() {
  struct Node *node = relational();
  for(;;) {
    if(consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if(consume("!="))
      node = new_node(ND_NE, node, relational());
    else return node;
  }
}

/*
 * l_and = equality ("&&" l_and)*
 */
struct Node *l_and() {
  struct Node *node = equality();
  while(consume("&&")) {
    node = new_node(ND_LAND, node, l_and());
  }
  return node;
}

/*
 * l_or = l_and ("||" l_and)*
 */
struct Node *l_or() {
  struct Node *node = l_and();
  while(consume("||")) {
    node = new_node(ND_LOR, node, l_and());
  }
  return node;
}

/*
 * syntax:
 * assign = l_or
 *        | unary "=" assign
 *
 * ast:
 * assign = l_or ("=" assign)?
 */
struct Node *assign() {
  struct Node *node = l_or();
  if(consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

struct Node *expr() {
  return assign();
}

struct Type *specifier();
/*
 * struct_union = ident "{" (specifier ident ";")+ "}"
 *              | ident
 */
struct Type *struct_union(enum TokenKind kind) {
  struct Token *tok = consume_ident();
  if(tok) {
    struct Type *type = calloc(1, sizeof(struct Type));
    if(kind == TK_STRUCT) type->ty = STRUCT;
    else type->ty = UNION;
    type->size = 0;

    if(consume("{")) {
      struct StructUnion *struct_union = calloc(1, sizeof(struct StructUnion));
      type->struct_union = struct_union;
      struct_union->name = tok->str;
      struct_union->len  = tok->len;

      if(kind == TK_STRUCT) {
        struct_union->next = structs;
        structs = struct_union;
      }
      else { // union
        struct_union->next = unions;
        unions = struct_union;
      }
      // incomplete type now

      struct Var head = {.next = NULL, .offset = 0, .type = NULL};
      struct Var *var = &head;
      do {
        struct Type *ty = specifier();

        struct Token *vtok = consume_ident();
        var->next = calloc(1, sizeof(struct Var));
        if(kind == TK_STRUCT) {
          int delta_offset = var->type ? var->type->size : 0;
          var->next->offset = var->offset + delta_offset;
        }
        else { // union
          var->next->offset = 0;
        }
        var = var->next;

        var->type = ty;
        var->name = vtok->str;
        var->len  = vtok->len;

        if(kind == TK_STRUCT) type->size += ty->size;
        else // union
          if(type->size < ty->size) type->size = ty->size;

        expect(";");
      } while(!consume("}"));

      struct_union->declarators = head.next;
      struct_union->size = type->size;

      return type;
    }
    else {
      struct StructUnion *struct_union = calloc(1, sizeof(struct StructUnion));
      struct_union->name = tok->str;
      struct_union->len = tok->len;
      type->struct_union = struct_union;
      return type;
    }
  }
  else {
    error_at(token->pos, "anonymous struct is unimplemented");
    return NULL; // not reached
  }
}

/*
 * enumerable = ident "{" ident ("," ident)* ","? "}"
 *            | ident
 */
struct Type *enumerable() {
  struct Token *name = consume_ident();
  if(!name) error_at(token->pos, "Identifier expected");
  if(consume("{")) {
    struct Token *tok = consume_ident();
    if(!tok) error_at(token->pos, "Identifier expected");

    int value = 0;

    bool break_after_loop = false;
    while(tok) {
      if(!consume(",")) break_after_loop = true;
      struct Enum *e = calloc(1, sizeof(struct Enum));
      e->name = tok->str;
      e->value = value++;
      e->next = enums;
      enums = e;
      tok = consume_ident();
      if(break_after_loop) break;
    }
    expect("}");
    return &anonymous_int;
  }
  return &anonymous_int;
}

/*
 * specifier = ("int"|"char"|("struct"|"union") struct_union|"enum" enumerable) "*"*
 */
struct Type *specifier() {
  struct Type *type = NULL;
  // TODO: don't consider void as int
  if(consume_kind(TK_INT) || consume_kind(TK_VOID)) type = gen_type(INT, NULL, ty_size(INT));
  else if(consume_kind(TK_LONG)) type = gen_type(LONG, NULL, ty_size(LONG));
  else if(consume_kind(TK_CHAR)) type = gen_type(CHAR, NULL, ty_size(CHAR));
  else if(consume_kind(TK_STRUCT)) type = struct_union(TK_STRUCT);
  else if(consume_kind(TK_UNION)) type = struct_union(TK_UNION);
  else if(consume_kind(TK_ENUM)) type = enumerable();
  else error_at(token->pos, "type name expected");
  while(consume("*")) type = gen_type(PTR, type, ty_size(PTR));
  return type;
}

/*
 * initializer = "{" (initializer ",")* initializer ","? "}"
 *             | expr
 */
struct Node *initializer() {
  struct Node head = {.kind = ND_INIT_ARRAY, .lhs=NULL, .rhs=NULL};
  struct Node *array = &head;
  if(consume("{")) {
    for(;;) {
      array->rhs = new_node(ND_INIT_ARRAY, initializer(), NULL);
      array=array->rhs;
      if(consume(",")) {
        if(consume("}")) break;
        continue;
      }
      else {
        expect("}");
        break;
      }
    }
    return head.rhs;
  }
  else {
    return expr();
  }
}

/*
 * stmt = expr ";"
 *      | "return" expr ";"
 *      | "if" "(" expr ")" stmt ("else" stmt)?
 *      | "while" "(" expr ")" stmt
 *      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
 *      | "{" stmt* "}"
 *      | specifier ident ("[" expr "]")* ";"
 */
struct Node *stmt() {
  struct Node *node;

  if(consume("{")) {
    struct Scope *scope = calloc(1, sizeof(struct Scope));
    scope->parent = current_scope;
    current_scope = scope;

    node = calloc(1, sizeof(struct Node));
    node->kind = ND_BLOCK;
    node->next = NULL;
    struct Node head = {.next = NULL};
    struct Node *cur = &head;
    while(!consume("}")) {
      cur->next = stmt();
      cur = cur->next;
    }
    cur->next = NULL;
    node->body = head.next;

    current_scope = current_scope->parent;
    return node;
  }

  if(consume_kind(TK_IF)) {
    node = calloc(1, sizeof(struct Node));
    node->kind = ND_IF;
    expect("(");
    node->cond = expr();
    expect(")");
    node->body = stmt();
    if(consume_kind(TK_ELSE)) {
      node->elsebody = stmt();
    }
    return node;
  }

  if(consume_kind(TK_WHILE)) {
    node = calloc(1, sizeof(struct Node));
    node->kind = ND_WHILE;
    expect("(");
    node->cond = expr();
    expect(")");
    node->body = stmt();
    return node;
  }

  if(consume_kind(TK_FOR)) {
    node = calloc(1, sizeof(struct Node));
    node->kind = ND_FOR;
    expect("(");

    struct Scope *scope = calloc(1, sizeof(struct Scope));
    scope->parent = current_scope;
    current_scope = scope;

    if(!consume(";")) {
      if(check_specifier()) {
        struct Type *type = specifier();
        struct Token *tok = consume_ident();
        if(!tok) error_at(token->pos, "Identifier expected");
        struct Var *var = calloc(1, sizeof(struct Var));
        var->type = type;
        var->next = current_scope->variables;
        current_scope->variables = var;
        var->name = tok->str;
        var->len = tok->len;
        if(var->next) var->offset = var->next->offset + size_of(type);
        else var->offset = size_of(type) + current_scope->parent->variables->offset;

        if(nodes->offset < var->offset) nodes->offset = var->offset;

        if(consume("=")) {
          struct Node *asgn = calloc(1, sizeof(struct Node));
          struct Node *lvar = calloc(1, sizeof(struct Node));
          lvar->kind = ND_LVAR;
          lvar->var = var;
          lvar->offset = var->offset;
          asgn->kind = ND_ASSIGN;
          asgn->lhs = lvar;
          asgn->rhs = expr();
          node->lhs = asgn;
        }
        expect(";");
      }
      else {
        node->lhs = expr();
        expect(";");
      }
    }

    if(!consume(";")) {
      node->cond = expr();
      expect(";");
    }

    if(!consume(")")) {
      node->rhs = expr();
      expect(")");
    }
    node->body = stmt();

    current_scope = current_scope->parent;
    return node;
  }

  if(check_specifier()) {
    struct Var *var = calloc(1, sizeof(struct Var));
    var->type = specifier();
    struct Token *tok = consume_ident();
    if(!tok) error_at(token->pos, "Identifier expected");
    if(find_var_scope(current_scope, tok)) error_at(tok->pos, "Second declaration");
    var->next = current_scope->variables;
    var->name = tok->str;
    var->len = tok->len;
    current_scope->variables = var;

    while(consume("[")) {
      var->type = gen_type(ARY,
          var->type,
          size_of(var->type) * compute_const_expr(expr()));
      expect("]");
    }

    var->offset = nodes->offset + size_of(var->type);
    nodes->offset = var->offset;

    // XXX: { int x; } does not work
    // should add new node type ND_DECLARE
    if(consume(";")) return stmt();

    expect("=");
    struct Node *exp = initializer();
    struct Node *lvar = calloc(1, sizeof(struct Node));
    lvar->kind = ND_LVAR;
    lvar->var = var;
    lvar->offset = var->offset;
    struct Node *res = new_node(ND_INIT, lvar, exp);
    expect(";");
    return res;
  }

  if(consume_kind(TK_RETURN)) {
    node = calloc(1, sizeof(struct Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  }
  else {
    node = expr();
  }
  expect(";");
  return node;
}

/*
 * parameter_list = (specifier ident ",")* specifier ident
 * parameter_type_list = parameter_list ("," "...")?
 */
struct Var *parameter_type_list() {
  struct Var head = {.next = NULL};
  struct Var *cur = &head;
  for(;;) {
    if(consume("...")) {
      struct Var *var = calloc(1, sizeof(struct Var));
      var->next = NULL;
      var->type = NULL;
      var->offset = cur->offset;
      cur->next = var;
      cur = cur->next;
      return head.next;
    }
    struct Type *type = specifier();
    struct Token *tok = consume_ident();
    if(!tok) error("Identifier expected");
    struct Var *var = calloc(1, sizeof(struct Var));
    var->next = NULL;
    var->name = tok->str;
    var->len = tok->len;
    var->type = type;
    var->offset = cur->offset + size_of(type);
    cur->next = var;
    cur = cur->next;
    if(!consume(",")) break;
  }
  return head.next;
}

/*
 * program = specifier ident "(" parameter_type_list ")" (stmt|";")
 *         | specifier ident ( "[" expr "]" )* ";"
 *         | specifier ";"
 */
void program() {
  globals = calloc(1, sizeof(struct Scope));
  current_scope = globals;
  while(!at_eof()) {
    struct Type *type = specifier();
    struct Token *tok = consume_ident();
    if(!tok) {
      expect(";");
      continue;
    }
    if(consume("(")) {
      struct Function *func;
      if(func = find_func(tok)) {
        while(!consume(")")) token = token->next;
        func->body = stmt();
        continue;
      }
      func = calloc(1, sizeof(struct Function));
      func->locals = NULL;
      func->name = tok->str;
      func->len = tok->len;
      func->type = type;
      func->locals = calloc(1, sizeof(struct Scope));
      func->locals->parent = current_scope;
      struct Node *node = calloc(1, sizeof(struct Node));
      node->kind = ND_DEFUN;
      node->func = func;
      node->next = nodes;
      nodes = node;
      if(consume(")")) {
        func->params = NULL;
      }
      else {
        struct Var *var = parameter_type_list();
        for(struct Var *v=var; v; v=v->next)
          node->offset = v->offset;
        expect(")");
        func->locals->variables = var;
        func->params = var;
      }
      if(!consume(";")) {
        func->next = functions;
        functions = func;
        current_scope = func->locals;

        expect("{");
        struct Node *block = calloc(1, sizeof(struct Node));
        block->kind = ND_BLOCK;
        block->next = NULL;
        struct Node head = {.next = NULL};
        struct Node *cur = &head;
        while(!consume("}")) {
          cur->next = stmt();
          cur = cur->next;
        }
        cur->next = NULL;
        block->body = head.next;

        func->body = block;
        current_scope = globals;
      }
      else func->body = NULL;
      continue;
    }
    struct Var *var = calloc(1, sizeof(struct Var));
    var->type = type;
    while(consume("[")) {
      var->type = gen_type(ARY,
          var->type,
          size_of(var->type) * compute_const_expr(expr()));
      expect("]");
    }
    if(!consume(";")) {
      expect("=");
      var->initial = expr();
      expect(";");
    }
    var->next = globals->variables;
    globals->variables = var;
    var->name = tok->str;
    var->len = tok->len;
    continue;
  }
}

