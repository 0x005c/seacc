#include "test/test_preprocessor.h"

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

int f(int x, int y) {
  return x*y;
}

int main() {
  x = 12345;
  assert_equal(f(3, 4), 12);
  assert_equal(x, 12345);
  return 0;
}

// int f(int x, int y) {
//   return x*y;
// }
