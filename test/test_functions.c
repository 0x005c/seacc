int count = 0;

void exit(int code);

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

int func_with_arg(int x);
int f();
int g() {
  func_with_arg(3);
  return f()*2;
}
int func_with_arg(int x) {
  return x;
}
int f() { return 20; }
int id(int x) { return x; }
int many(int a, int b, int c, int d, int e, int f) {
  return a+b+c+d+e+f;
}
int few(int a, int b, int c, int d) {
  return a+b+c+d;
}

int add(int x, int y) {
  return x+y;
}

int fac(int x) {
  if(x>0) return x*fac(x-1);
  return 1;
}

int fib(int x) {
  if(x<3) return 1;
  return fib(x-1) + fib(x-2);
}

// declared but not defined
int h(int x);

int main() {
  assert_equal(f(), 20);
  assert_equal(g(), 40);
  assert_equal(add(10,20), 30);
  assert_equal(id(id(id(id(id(5))))), 5);
  // FIXME: assert_equal(many(1,2,3,4,5,6), 21);
  assert_equal(few(1,2,3,4), 10);
  assert_equal(fac(5), 120);
  assert_equal(fib(6), 8);
  return 0;
}
