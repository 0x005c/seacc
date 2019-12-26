#!/bin/bash
try() {
  filename="test/$1"

  if ! ./seacc "$filename" > ./tmp.s ; then
    echo "$filename: compile failed"
    exit 1
  fi
  gcc -o tmp tmp.s
  ./tmp
  result="$?"

  if [ "$result" == 0 ]; then
    echo "$filename: test passed"
  else
    echo "$filename: test not passed"
    echo "$result th assertion failed"
    exit 1
  fi
}

# test expressions
try 'test_expr.c'
try 'test_variables.c'
try 'test_control_flow.c'
try 'test_functions.c'
try 'test_types.c'
try 'test_literals.c'

echo OK
