#!/bin/bash

path=`dirname $0`

for i in `find $1 -name "*.cpp" -o -name "*.h"`
do
	python $path/cpplint_tizen.py $i
done
