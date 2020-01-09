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

enum Jname {
  TARO,
  JIRO,
  SABURO,
};

enum Ename {
  ALICE,
  BOB,
  CHARLIE
};

enum Jname jname;
enum Ename ename;

int main() {
  jname = TARO;
  ename = ALICE;
  enum Ename lename = ALICE;

  assert_equal(TARO, 0);
  assert_equal(JIRO, 1);
  assert_equal(SABURO, 2);
  assert_equal(jname, TARO);

  assert_equal(CHARLIE, 2);
  assert_equal(ename, lename);
  return 0;
}

