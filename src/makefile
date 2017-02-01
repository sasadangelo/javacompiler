CC= g++
OPT = -fPIC -c -g
LIB = -lfl
LEX = flex
BISON = bison
NAME_EXE = jcc
MV = mv
OBJS = main.o lex.yy.o hash.o tree.o compile.o environment.o cstring.o \
	java.tab.o table.o expr.o parse.o descriptor.o errors.o jvm.o \
	pcode.o gen.o genexpr.o backpatch.o local.o cphash.o

DIR_INCLUDE=./include
DIR_CC=./

jcc: $(OBJS)
	$(CC) $(OBJS) $(LIB) -o $(NAME_EXE);

main.o: main.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) main.cc 

hash.o: hash.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) hash.cc

tree.o: tree.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) tree.cc

cstring.o: cstring.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) cstring.cc

table.o: table.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) table.cc

lex.yy.o: java.lex
	$(LEX) java.lex 
	$(MV)  lex.yy.c lex.yy.cc
	$(CC)  $(OPT) -I$(DIR_INCLUDE) lex.yy.cc

compile.o: compile.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) compile.cc

environment.o: environment.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) environment.cc

java.tab.o: java.y
	$(BISON) java.y 
	$(MV) java.tab.c java.tab.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) java.tab.cc

expr.o: expr.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) expr.cc

parse.o: parse.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) parse.cc

descriptor.o: descriptor.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) descriptor.cc

errors.o: errors.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) errors.cc

jvm.o: jvm.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) jvm.cc

pcode.o: pcode.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) pcode.cc

gen.o: gen.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) gen.cc

genexpr.o: genexpr.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) genexpr.cc

backpatch.o: backpatch.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) backpatch.cc

local.o: local.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) local.cc

cphash.o: cphash.cc
	$(CC) $(OPT) -I$(DIR_INCLUDE) cphash.cc
