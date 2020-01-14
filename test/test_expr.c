int count = 0;

void exit(int code);

int printf(char *fmt, ...);

void assert_true(int result) {
  count = count+1;
  if(result == 0) {
    printf("%dth assertion failed\n", count);
    exit(1);
  }
}

void assert_equal(int l, int r) {
  count = count+1;
  if(l!=r) {
    printf("%dth assertion failed\n", count);
    printf("%d expected, but got %d\n", r, l);
    exit(1);
  }
}

int x = 0;

int f() {
  return ++x;
}

int g() {
  return ++x;
}

int main() {
  assert_equal(0, 0);
  assert_equal(-42, -42);
  assert_equal(42, 42);
  assert_equal(5+20-4, 21);
  assert_equal(12 + 34 - 5, 41);
  assert_equal(5+6*7, 47);
  assert_equal(5*(9-6), 15);
  assert_equal((3+5)/2, 4);
  assert_equal(-10+13, 3);
  assert_equal(+5, 5);
  assert_equal(-(-3), 3);
  assert_true(3>-3);
  assert_true(50*2>=100);
  assert_true(50/20<100);
  assert_true(123!=456);
  assert_equal(!123, 0);
  assert_equal(!0, 1);
  assert_equal(-1&&1, 1);
  assert_equal(-1&&0, 0);
  assert_equal(0&&1, 0);
  assert_equal(0&&0, 0);
  assert_equal(-1||1, 1);
  assert_equal(-1||0, 1);
  assert_equal(0||1, 1);
  assert_equal(0||0, 0);

  assert_equal(f() && g(), 1);
  assert_equal(x, 2);
  x=-1;
  assert_equal(f() && g(), 0);
  assert_equal(x, 0);

  x=0;
  assert_equal(f() || g(), 1);
  assert_equal(x, 1);
  x=-1;
  assert_equal(f() || g(), 1);
  assert_equal(x, 1);
  assert_true(1 && 1 && 1);

  x=0;
  assert_equal(x++, 0);
  assert_equal(x, 1);
  assert_equal(x--, 1);
  assert_equal(x, 0);

  char c = 'a';
  assert_true(!!c);
  assert_true(1 || (0 && 0));
  return 0;
}

