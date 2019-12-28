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

struct Struct { int data; } s;

int main() {
  s.data = 123;
  assert_equal(s.data, 123);
  return 0;
}

