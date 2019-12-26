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

int f() {
  int y;
  for(y=0; y<10; y=y+1) 1;
  return y;
}

int main() {
  int x;

  x=0;
  if(1>0) x=1;
  assert_true(x);

  x=4;
  if(x>0)
    if(x<10) x=x*x;
  assert_equal(x, 16);

  x=0;
  if(1>0)
    if(1<0) x=x+1;
    else x=x+10;
  else x=x+100;
  assert_equal(x, 10);

  x=0;
  while(x<10) x=x+1;
  assert_equal(x, 10);

  while(0) assert_true(1);

  assert_equal(f(), 10);

  x=10;
  if(x==10) {
    x=x*2;
    x=x/10;
  }
  assert_equal(x, 2);

  {}{{{{{}}}}}{}{}{{{{{}}}}}

  return 0;
}

