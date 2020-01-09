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
  return 0;
}

