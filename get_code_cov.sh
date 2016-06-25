#!/bin/sh

for file in ./src/*; do
  gcov -n -o . ${file} > /dev/null
done