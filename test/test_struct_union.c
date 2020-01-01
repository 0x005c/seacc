int count = 0;

int assert_true(int result) {
  count = count+1;
  if(result) {
    return 0;
  }
  else exit(count);
}

int assert_equal(int l, int r) {
  assert_true(l==r);
}

struct Struct { char x; int a; int b; int c; } s;
struct Struct t;

union Union { char c; int x; } u1;
// union Union2 { char c[2]; int x; } uu;
union Union u2;

int main() {
  struct Struct u;
  union Union u3;
  s.a = 123;
  s.b = 456;
  s.c = 789;
  s.x = 'z';
  assert_equal(s.a, 123);
  assert_equal(s.b, 456);
  assert_equal(s.c, 789);
  t = s;
  assert_equal(t.a, 123);
  assert_equal(t.b, 456);
  assert_equal(t.c, 789);
  u = t;
  assert_equal(u.a, 123);
  assert_equal(u.b, 456);
  assert_equal(u.c, 789);
  assert_equal(u.x, 'z');
  s.a = s.b - s.a;
  assert_equal(s.a, 333);

  u1.x = 'a'*256 + 'b';
  u2 = u1;
  u3 = u1;
  assert_equal(u1.x, u2.x);
  assert_equal(u2.x, u3.x);
  assert_equal(u1.c, 'b');
  assert_equal(u2.c, 'b');
  assert_equal(u3.c, 'b');

  struct Struct *sp = &u;
  assert_equal(sp->a, 123);
  assert_equal(sp->b, 456);
  assert_equal(sp->c, 789);

  struct Struct *sp2 = &s;
  assert_equal(sp2->a, 333);
  assert_equal(sp2->b, 456);
  assert_equal(sp2->c, 789);
  return 0;
}

