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
try 0 'main() {return 0;}'
try 42 'main() {return 42;}'
try 21 'main() {return 5+20-4;}'
try 41 'main() {return 12 + 34 - 5 ;}'
try 47 'main() {return 5+6*7;}'
try 15 'main() {return 5*(9-6);}'
try 4 'main() {return (3+5)/2;}'
try 3 'main() {return -10 + 13;}'
try 5 'main() {return +5;}'
try 3 'main() {return -(-3);}'
try 0 'main() {return -3>3;}'
try 1 'main() {return 50*2>=100;}'
try 1 'main() {return -3<=3;}'
try 1 'main() {return 50/20<100;}'
try 1 'main() {return 100+23==123;}'
try 0 'main() {return 123==456;}'
try 0 'main() {return 123!=123;}'
try 1 'main() {return 123!=456;}'

# test variables
try 10 'main() {x=10; return x;}'
try 101 'main() {x=100; y=1; return x+y;}'
try 0 'main() {x=y=1; return x-y;}'
try 10 'main() {x=100;x=200;y=0;y=y+10;return y;}'

# test control flow
## if
try 10 'main() {if(1>0) return 10; else return 20;}'
try 100 'main() {x=0; if(2>1) x=100; else x=200; return x;}'
try 16 'main() {x=4; if(x>0) if(x<10) return x*x;}'
try 10 'main() {x=0; if(1>0) if(1<0) x = x+1; else x = x+10; else x = x+100; return x;}'
try 3 'main() {x=3; if(x==1) return 1; else if(x==2) return 2; else if(x==3) return 3;}'
## while
try 10 'main() {x=0; while(x<10) x=x+1; return x;}'
try 1 'main() {while(0) return 0; return 1;}'
try 100 'main() {while(1) while(1) return 100;}'
## for
try 5 'main() {for(;;) return 5; return 2;}'
try 123 'main() {for(x=123;;) return x;}'
try 11 'main() {x=0; for(;;x=x+1) if(x>10) return x;}'
try 10 'main() {y=0; for(x=0; x<10; x=x+1) y=y+1; return y;}'

# test block
try 10 'main() {x=0; x=x+10; return x;}'
try 2 'main() {x=10; if(x==10) {x=x*2; return x/10;}}'
try 123 'main() {if(0==0) {} return 123;}'

# test functions
try 10 'main    ()     { return 10   ; }'
try 20 'f() { return 20; } main() { return f(); }'
try 30 'g() { return 30; } f() { return g(); } main() { return f(); }'
try 40 'f(x) { return x+x; } main() { return f(f(10)); }'
try 50 'add(x,y) { return x+y; } main() { return add(22, 28); }'
try 60 'f(x) { if(x==0) return 60; else return f(x-1); } main() { return f(5); }'
try 70 'f(a,b,c,d,e,f) { return a+b+c+d+e+f; } main() { return f(10,10,10,10,10,20); }'
try 6 'fac(x) { if(x>0) return x*fac(x-1); return 1; } main() { return fac(3); }'
try 7 'f(x) { return x; } main() { return f(7); }'
try 8 'fib(x) { if(x<3) return 1; return fib(x-1) + fib(x-2); } main() { return fib(6); }'
try 10 'main() {x=0; if(x>1) return x; return x+10; }'

# test pointer
try 10 'main() {x = 10; y = &x; return *y;}'

echo OK
