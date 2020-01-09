#include "test/test_preprocessor.h"

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
