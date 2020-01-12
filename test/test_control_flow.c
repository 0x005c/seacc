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

int f() {
  int y=0;
  while(y<10) ++y;
  return y;
}

int main() {
  int x = 0;

  if(1>0) x=1;
  assert_true(x);

  x=4;
  if(x>0)
    if(x<10) x=x*x;
  assert_equal(x, 16);

  {
    int x = 123;
    assert_equal(x, 123);
  }

  assert_equal(x, 16);

  x=0;
  if(1>0)
    if(1<0) x=x+1;
    else x=x+10;
  else x=x+100;
  assert_equal(x, 10);

  x=0;
  while(x<10) ++x;
  assert_equal(x, 10);

  while(0) assert_true(1);

  assert_equal(f(), 10);

  x=10;
  if(x==10) {
    x=x*2;
    x=x/10;
  }
  assert_equal(x, 2);

  x=-1;
  for(int i=0; i<10; ++i) assert_equal(i, ++x);

  for(int i=0; i<10; ++i) x=i;
  assert_equal(x, 9);

  // FIXME: for(x=0; x<10; x=x+1); assert_equal(x, 10);
  {}{{{{{}}}}}{}{}{{{{{}}}}}

  return 0;
}

