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

int main() {
  struct Struct u;
  // FIXME: s.x = 'z';
  s.a = 123;
  s.b = 456;
  s.c = 789;
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
  // FIXME: assert_equal(u.x, 'z');
  s.a = s.b - s.a;
  assert_equal(s.a, 333);
  return 0;
}

