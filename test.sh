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

echo OK
