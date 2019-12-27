int count;

int assert_true(int result) {
  if(result) {
    count = count+1;
    return 0;
  }
  else exit(count);
}

int assert_equal(int l, int r) {
  assert_true(l==r);
}

int main() {
  assert_equal('a', 97);
  assert_true('f'-'a'+'A'=='F');
  assert_equal('\123', 123);
  assert_equal('\a', 7);
  assert_equal('\\', 92);

  char *s;
  s="";
  assert_true(s[0] == '\0');

  s="abc";
  assert_true(s[0] == 'a');
  assert_true(s[1] == 'b');
  assert_true(s[2] == 'c');
  assert_true(s[3] == '\0');
  return 0;
}