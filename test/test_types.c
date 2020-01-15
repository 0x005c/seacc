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

struct Struct {
  long x;
  int y;
  char z;
};

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

  assert_equal(sizeof(long), 8);
  assert_equal(sizeof(int), 4);
  assert_equal(sizeof(char), 1);
  assert_equal(sizeof(struct Struct), 8+4+1);

  char *s;
  s = "a";
  assert_true(s[0]);
  assert_true(!s[1]); // s[1] is zero here
  x=0;
  if(s[1]) x=1; // s[1] must not be true
  assert_true(!x);

  return 0;
}

