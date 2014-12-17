#!/bin/sh

find $(pwd) -name "*.h" -o -name "*.c" -o -name "*.cc" -o -name "*.hpp" -o -name "*.cpp" -o -name "*.java" -o -name "*.dd" |sed 's/^\(.*\)$/"\1"/g' > cscope.files
#find $(pwd) -name "*.java"|sed 's/^\(.*\)$/"\1"/g' > cscope.files
cscope -bkq -i cscope.files
ctags -R
