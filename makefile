CC=gcc
CFLAGS=-W -Wall -g
name=ijscanner

.PHONY: clean run y.tab.c zip

all : lex.yy.c y.tab.c ast.c symbols.c
	$(CC) $(CFLAGS) -o $(name) y.tab.c lex.yy.c structures.c ast.c symbols.c semantics.c codeGen.c  -ll -ly

lex.yy.c : ijscanner.l
	lex $(name).l

y.tab.c:
	bison -d -y -r all $(name).y

clean:
	rm -v y.tab.c y.tab.h lex.yy.c $(name) $(name).zip

run: all
	./${name}

zip: all
	zip ijscanner.zip ijscanner.l ijscanner.y structures.c structures.h ast.c ast.h symbols.h symbols.c semantics.h semantics.c codeGen.h codeGen.c
