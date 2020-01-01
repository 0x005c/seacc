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

enum Jname {
  TARO,
  JIRO,
  SABURO,
} jname;

enum Ename {
  ALICE,
  BOB,
  CHARLIE
} ename;

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

