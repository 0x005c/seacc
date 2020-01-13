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

int a=1;
int b='z';
int *c;
// int *c=&a;
int d[123];
int rbx;

int x = 123;

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
  long lx = 10;
  long ly = 1;
  long lz[10] = {1,2,3,4,5,6,7,8,9,10};
  assert_equal(lx, 10);
  assert_equal(lz[8], 9);
  lx=100;
  assert_equal(lx+ly, 101);
  ly=0; ly=ly+10;
  assert_equal(ly, 10);
  x=0;
  x+=123;
  assert_equal(x, 123);
  x-=456;
  assert_equal(x, 123-456);
  int *zp = z;
  zp += 3;
  assert_equal(*zp, 3);
  return 0;
}

