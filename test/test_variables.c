int count;

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

int a=1;
int b='z';
int *c;
// int *c=&a;
int d[123];
int rbx;

int main() {
  int x; int y; int z[10]; int rax;
  x=10;
  assert_equal(x, 10);
  x=100; y=1;
  assert_equal(x+y, 101);
  y=0; y=y+10;
  assert_equal(y, 10);
  rax=100;
  assert_equal(rax, 100);
  assert_equal(a, 1);
  assert_equal(b, 'z');
  a=123; b=111;
  assert_equal(-b+a, 12);
  c=&a;
  *c=123;
  assert_equal(a, 123);
  *z = a;
  assert_equal(*z, a);
  for(y=0; y<10; y=y+1) z[y]=y;
  assert_equal(z[9], 9);
  rbx=100;
  assert_equal(rbx, 100);
  return 0;
}

