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

int strncmp(char *l, char *r, long size);

// FIXME: char *str = "abc";
int main() {
  assert_equal('a', 97);
  assert_true('f'-'a'+'A'=='F');
  assert_equal('\123', 123);
  assert_equal('\a', 7);
  assert_equal('\\', 92);

  char *empty = "";
  assert_true(empty[0] == '\0');

  char *s = "abc";
  assert_true(s[0] == 'a');
  assert_true(s[1] == 'b');
  assert_true(s[2] == 'c');
  assert_true(s[3] == '\0');
  assert_true(strncmp(s, "abcd", 3) == 0);
  assert_true(strncmp(s, "abc", 3) == 0);
  assert_true(strncmp(s, "ab", 3));
  return 0;
}
