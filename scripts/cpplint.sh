#!/bin/bash

path=`dirname $0`
sources=`find $1 -name "*.cpp" -o -name "*.h"`

python $path/cpplint_tizen.py $sources
