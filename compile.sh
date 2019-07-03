rm -f gocompiler.out
flex gocompiler.l
yacc -d gocompiler.y
clang-3.8 -g -Wpedantic  -Wall -Wno-unused-function -o gocompiler.out *.c
rm -f y.tab.c lex.yy.c y.tab.h
