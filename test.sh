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
try 0 'return 0;'
try 42 'return 42;'
try 21 'return 5+20-4;'
try 41 'return 12 + 34 - 5 ;'
try 47 'return 5+6*7;'
try 15 'return 5*(9-6);'
try 4 'return (3+5)/2;'
try 3 'return -10 + 13;'
try 5 'return +5;'
try 3 'return -(-3);'
try 0 'return -3>3;'
try 1 'return 50*2>=100;'
try 1 'return -3<=3;'
try 1 'return 50/20<100;'
try 1 'return 100+23==123;'
try 0 'return 123==456;'
try 0 'return 123!=123;'
try 1 'return 123!=456;'

# test variables
try 10 'x=10; return x;'
try 101 'x=100; y=1; return x+y;'
try 0 'x=y=1; return x-y;'
try 0 'x=100;x=200;y=0;y=y+10;return 0;'

# test control flow
## if
try 10 'if(1>0) return 10; else return 20;'
try 100 'x=0; if(2>1) x=100; else x=200; return x;'
try 16 'x=4; if(x>0) if(x<10) return x*x;'
try 10 'x=0; if(1>0) if(1<0) x = x+1; else x = x+10; else x = x+100; return x;'
try 3 'x=3; if(x==1) return 1; else if(x==2) return 2; else if(x==3) return 3;'
## while
try 10 'x=0; while(x<10) x=x+1; return x;'
try 1 'while(0) return 0; return 1;'
try 100 'while(1) while(1) return 100;'

echo OK
