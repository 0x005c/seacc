#include "seacc.h"

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

int isspace(int c);
int isdigit(int c);
char *strncpy(char *dest, char *src, long n);
int strncmp(char *s1, char *s2, long n);
int memcmp(void *s1, void *s2, long n);
void *calloc(long nmemb, long size);
long strtol(char *beg, char **end, int base);

struct Token *new_token(enum TokenKind kind, struct Token *cur, char *str, int len) {
  struct Token *tok = calloc(1, sizeof(struct Token));
  tok->kind = kind;
  tok->str = calloc(1, len+1);
  strncpy(tok->str, str, len);
  tok->pos = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

struct StringLiteral *find_lit(char *str) {
  return 0;
}

struct StringLiteral *get_lit(char *str, int len) {
  char *text = calloc(1, len+1);
  strncpy(text, str, len);
  text[len] = '\0';
  struct StringLiteral *lit = find_lit(text);
  if(lit) return lit;

  lit = calloc(1, sizeof(struct StringLiteral));
  lit->str = text;
  if(slit) lit->id = slit->id+1;
  else lit->id = 0;
  lit->next = slit;
  slit = lit;
  return lit;
}

struct Token *tokenize(char *p) {
  struct Token head;
  head.next = 0;
  struct Token *cur = &head;

  while(*p) {
    if(isspace(*p)) {
      p++;
      continue;
    }

    // "//" comment out
    if(strncmp(p, "//", 2) == 0) {
      p+=2;
      while(*p != '\n') p++;
      continue;
    }

    // "/* */" comment out
    if(p[0]=='/' && p[1]=='*') {
      char *q=p;
      p+=2;
      for(;;) {
        if(p[0]=='\0') error_at(q, "Comment is not closed");
        if(p[0]=='*' && p[1]=='/') {
          p=p+2;
          break;
        }
        p++;
      }
      continue;
    }

    if(*p == '*'
        || *p == '/'
        || *p == '('
        || *p == ')'
        || *p == '{'
        || *p == '}'
        || *p == ';'
        || *p == ','
        || *p == '['
        || *p == ']'
        || *p == '#') {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if(*p == '&') {
      if(p[1] == '&') {
        cur = new_token(TK_RESERVED, cur, p, 2);
        p+=2;
        continue;
      }
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if(*p == '|' && p[1] == '|') {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p+=2;
      continue;
    }

    if(*p == '+') {
      if(p[1] == '+') {
        cur = new_token(TK_RESERVED, cur, p, 2);
        p+=2;
        continue;
      }
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }
    if(*p == '.') {
      if(p[1] == '.' && p[2] == '.') {
        cur = new_token(TK_RESERVED, cur, p, 3);
        p+=3;
        continue;
      }
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if(*p == '-') {
      if(p[1] == '-') {
        cur = new_token(TK_RESERVED, cur, p, 2);
        p+=2;
        continue;
      }
      if(p[1] == '>') {
        cur = new_token(TK_RESERVED, cur, p, 2);
        p+=2;
        continue;
      }

      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if(*p == '<') {
      if(p+1 && *(p+1) == '=') {
        cur = new_token(TK_RESERVED, cur, p, 2);
        p += 2;
        continue;
      }
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if(*p == '>') {
      if(*(p+1) == '=') {
        cur = new_token(TK_RESERVED, cur, p, 2);
        p += 2;
        continue;
      }
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if(*p == '=') {
      if(*(p+1) == '=') {
        cur = new_token(TK_RESERVED, cur, p, 2);
        p += 2;
        continue;
      }

      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if(*p == '!') {
      if(p[1] == '=') {
        cur = new_token(TK_RESERVED, cur, p, 2);
        p += 2;
        continue;
      }
      else {
        cur = new_token(TK_RESERVED, cur, p++, 1);
        continue;
      }
    }

    if(*p == '"') {
      int i;
      for(i=1; p[i]!='"'; i++) {
        if(p[i] == '\0') error_at(p+i, "Reached end of file before closing '\"'");
        if(p[i] == '\\') {
          i++;
          continue;
        }
        if(p[i] == '"') break;
      }
      i++;
      cur = new_token(TK_STRING_LITERAL, cur, p, i);
      cur->lit = get_lit(p+1, i-2); // exclude first and last '"'
      p=p+i;
      continue;
    }

    if(*p == '\'') {
      if(p[1] == '\\') {
        int val=0;
        if(p[2] == 'a') val = 0x07;
        if(p[2] == 'b') val = 0x08;
        if(p[2] == 'e') val = 0x1b;
        if(p[2] == 'f') val = 0x0c;
        if(p[2] == 'n') val = 0x0a;
        if(p[2] == 'r') val = 0x0d;
        if(p[2] == 't') val = 0x09;
        if(p[2] == 'v') val = 0x0b;
        if(p[2] == '\\') val = 0x5c;
        if(p[2] == '\'') val = 0x27;
        if(p[2] == '\"') val = 0x22;
        if(p[2] == '\?') val = 0x3f;
        if(val) {
          if(p[3] != '\'') error_at(p+3, "\"'\" expected, but got %c", p[3]);
          cur = new_token(TK_CHAR_LITERAL, cur, p, 4);
          cur->val = val;
          p+=4;
          continue;
        }
        if(isdigit(p[2])) {
          int digitlen = 0;
          while(isdigit(*(p+digitlen))) {
            digitlen++;
          }
          cur = new_token(TK_CHAR_LITERAL, cur, p, digitlen);
          p+=2;
          cur->val = strtol(p, &p, 10);
          if(*p != '\'') error_at(p, "\"'\" expected, but got %c", *p);
          p+=1;
          continue;
        }
        // TODO: tokenize escape like '\x5c'
        cur = new_token(TK_CHAR_LITERAL, cur, p, 4);
        cur->val = p[2];
        if(p[3] != '\'') error_at(p+3, "\"'\" expected, but got %c", p[3]);
        p+=4;
        continue;
      }
      cur = new_token(TK_CHAR_LITERAL, cur, p, 3);
      cur->val = *(p+1);
      p+=3;
      continue;
    }

    if(isdigit(*p)) {
      int digitlen = 0;
      while(isdigit(*(p+digitlen))) {
        digitlen++;
      }
      cur = new_token(TK_NUM, cur, p, digitlen);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    if(memcmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if(memcmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }

    if(memcmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }

    if(memcmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }

    if(memcmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
      continue;
    }

    if(memcmp(p, "enum", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TK_ENUM, cur, p, 4);
      p += 4;
      continue;
    }

    if(memcmp(p, "int", 3) == 0 && !is_alnum(p[3])) {
      cur = new_token(TK_INT, cur, p, 3);
      p += 3;
      continue;
    }

    if(memcmp(p, "long", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TK_LONG, cur, p, 4);
      p += 4;
      continue;
    }

    if(memcmp(p, "void", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TK_VOID, cur, p, 4);
      p += 4;
      continue;
    }

    if(memcmp(p, "struct", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_STRUCT, cur, p, 6);
      p += 6;
      continue;
    }

    if(memcmp(p, "union", 5) == 0 && !is_alnum(p[5])) {
      cur = new_token(TK_UNION, cur, p, 5);
      p += 5;
      continue;
    }

    if(memcmp(p, "sizeof", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_SIZEOF, cur, p, 6);
      p += 6;
      continue;
    }

    if(memcmp(p, "char", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TK_CHAR, cur, p, 4);
      p += 4;
      continue;
    }

    if(is_alnum(*p)) {
      int len;
      for(len=0; is_alnum(*(p+len)); len++);
      cur = new_token(TK_IDENT, cur, p, len);
      p += len;
      continue;
    }

    error_at(p, "Failed to tokenize");
  }

  new_token(TK_EOF, cur, p, 1);
  return head.next;
}

