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

int main() {
  int x;
  int *************y;

  assert_equal(sizeof(x), 4);
  assert_equal(sizeof(y), 8);
  // assert_equal(sizeof(2+5), 4);
  assert_equal(sizeof(x+y), 8);
  assert_equal(sizeof(x+*************y), 4);

  char c = 123;
  assert_equal(c, 123);

  x=24; c=2;
  assert_equal(x/c, 12);

  x=1024+123;
  x=c=x;
  assert_equal(x, 123);

  char b = 1;
  c=10;
  assert_equal(c*b/b+b-b, 10);

  assert_equal(sizeof c, 1);

  char text[3];
  assert_equal(sizeof(text), 3);

  char cs[12345];
  cs[1234] = 123;
  assert_equal(cs[1234], 123);

  assert_equal(sizeof(*cs), 1);
  assert_equal(sizeof(cs), 12345);
  assert_equal(sizeof(cs+1), 8);

  long lx;
  assert_equal(sizeof(lx), 8);
  assert_equal(sizeof(&lx), 8);
  return 0;
}

