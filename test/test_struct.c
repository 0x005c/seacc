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

struct Struct { int a; int b; } s;
struct Struct t;

int main() {
  struct Struct u;
  s.a = 123;
  s.b = 456;
  assert_equal(s.a, 123);
  assert_equal(s.b, 456);
  t = s;
  assert_equal(t.a, 123);
  assert_equal(t.b, 456);
  u = t;
  assert_equal(u.a, 123);
  assert_equal(u.b, 456);
  s.a = s.b - s.a;
  assert_equal(s.a, 333);
  return 0;
}

