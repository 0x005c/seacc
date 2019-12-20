#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./seacc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

# test expressions
try 0 'int main() {return 0;}'
try 42 'int main() {return 42;}'
try 21 'int main() {return 5+20-4;}'
try 41 'int main() {return 12 + 34 - 5 ;}'
try 47 'int main() {return 5+6*7;}'
try 15 'int main() {return 5*(9-6);}'
try 4 'int main() {return (3+5)/2;}'
try 3 'int main() {return -10 + 13;}'
try 5 'int main() {return +5;}'
try 3 'int main() {return -(-3);}'
try 0 'int main() {return -3>3;}'
try 1 'int main() {return 50*2>=100;}'
try 1 'int main() {return -3<=3;}'
try 1 'int main() {return 50/20<100;}'
try 1 'int main() {return 100+23==123;}'
try 0 'int main() {return 123==456;}'
try 0 'int main() {return 123!=123;}'
try 1 'int main() {return 123!=456;}'

# test variables
try 10 'int main() {int x; x=10; return x;}'
try 101 'int main() {int x; int y; x=100; y=1; return x+y;}'
try 0 'int main() {int x; int y; x=y=1; return x-y;}'
try 10 'int main() {int x; int y; x=100;x=200;y=0;y=y+10;return y;}'

# test control flow
## if
try 10 'int main() {if(1>0) return 10; else return 20;}'
try 100 'int main() {int x; if(2>1) x=100; else x=200; return x;}'
try 16 'int main() {int x; x=4; if(x>0) if(x<10) return x*x;}'
try 10 'int main() {int x; x=0; if(1>0) if(1<0) x = x+1; else x = x+10; else x = x+100; return x;}'
try 3 'int main() {int x; x=3; if(x==1) return 1; else if(x==2) return 2; else if(x==3) return 3;}'
## while
try 10 'int main() {int x; x=0; while(x<10) x=x+1; return x;}'
try 1 'int main() {while(0) return 0; return 1;}'
try 100 'int main() {while(1) while(1) return 100;}'
## for
try 5 'int main() {for(;;) return 5; return 2;}'
try 123 'int main() {int x; for(x=123;;) return x;}'
try 11 'int main() {int x; x=0; for(;;x=x+1) if(x>10) return x;}'
try 10 'int main() {int x; int y; y=0; for(x=0; x<10; x=x+1) y=y+1; return y;}'

# test block
try 10 'int main() {int x; x=0; x=x+10; return x;}'
try 2 'int main() {int x; x=10; if(x==10) {x=x*2; return x/10;}}'
try 123 'int main() {int x; if(0==0) {} return 123;}'

# test functions
try 10 'int main    ()     { return 10   ; }'
try 20 'int f() { return 20; } int main() { return f(); }'
try 30 'int g() { return 30; } int f() { return g(); } int main() { return f(); }'
try 40 'int f(int x) { return x+x; } int main() { return f(f(10)); }'
try 50 'int add(int x,int y) { return x+y; } int main() { return add(22, 28); }'
try 60 'int f(int x) { if(x==0) return 60; else return f(x-1); } int main() { return f(5); }'
try 70 'int f(int a,int b,int c,int d,int e,int f) { return a+b+c+d+e+f; } int main() { return f(10,10,10,10,10,20); }'
try 6 'int fac(int x) { if(x>0) return x*fac(x-1); return 1; } int main() { return fac(3); }'
try 7 'int f(int x) { return x; } int main() { return f(7); }'
try 8 'int fib(int x) { if(x<3) return 1; return fib(x-1) + fib(x-2); } int main() { return fib(6); }'
try 10 'int main() {int x; x=0; if(x>1) return x; return x+10; }'

# test pointer
try 10 'int main() {int x; int y; x = 10; y = &x; return *y;}'
try 3 'int main() { int x; int *y; y = &x; *y = 3; return x; }'
try 10 'int main() { int x; int y; x=10; y=20; int *z; z=&y; return *(z+1); }'

# test sizeof
try 8 'int main() { int x; return sizeof(x); }'
try 8 'int main() { int *****x; return sizeof(x); }'
try 8 'int main() { return sizeof(8); }' # ILP64
try 8 'int main() { return sizeof(2+5); }'
try 8 'int main() { int x; int y; return sizeof(x+y); }'

# test array
try 16 'int main() { int x[2]; return sizeof(x); }'
try 12 'int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *x+*(x+1)+*(x+2); }'
try 12 'int main() { int x[3]; x[0]=3; x[1]=4; x[2]=5; return x[0]+x[1]+x[2]; }'
try 72 'int main() { int x[3][3]; return sizeof(x); }'
try 1 'int x; int main() { x=1; return x; }'
try 123 'int *x; int main() { int y; x=&y; *x=123; return y; }'
try 22 'int x; int main() { int *y; y=&x; *y=22; return x; }'

echo OK
