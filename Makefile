all: codegen

codegen: idl.o idl.tab.o lex.yy.o codegen.o
	g++ -g -o codegen idl.o idl.tab.o lex.yy.o codegen.o

idl.tab.c:
	bison -d idl.y

lex.yy.c: idl.tab.c
	flex idl.l

idl.o: idl.cpp
	g++ -g -c -o idl.o idl.cpp

idl.tab.o: idl.tab.c
	gcc -g -c -w -o idl.tab.o idl.tab.c

lex.yy.o: lex.yy.c
	gcc -g -c -w -o lex.yy.o lex.yy.c

codegen.o: codegen.cpp
	g++ -g -c -o codegen.o codegen.cpp

clean:
	-rm -f idl.o idl.tab.o lex.yy.o codegen.o idl.tab.c lex.yy.c codegen codegen.exe idl.tab.h
