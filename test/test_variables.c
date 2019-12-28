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

int a=1;
int b='z';
int *c;
// int *c=&a;
int d[123];
int rbx;

int main() {
  int x=10;
  int y=1;
  int z[10] = {1,3,5,7,9};
  int rax='z';
  assert_equal(x, 10);
  assert_equal(z[2], 5);
  x=100;
  assert_equal(x+y, 101);
  y=0; y=y+10;
  assert_equal(y, 10);
  assert_equal(rax, 'z');
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
  for(y=0; y<10; ++y) z[y]=y;
  assert_equal(z[9], 9);
  rbx=100;
  assert_equal(rbx, 100);
  return 0;
}

