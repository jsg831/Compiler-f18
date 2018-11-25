all: yacc lex gcc clean

yacc: parser.y
	yacc -d parser.y

lex: scanner.l
	lex scanner.l

gcc: lex.yy.c
	gcc -o parser lex.yy.c y.tab.c -ly -ll

clean: lex.yy.c y.tab.c y.tab.h
	rm -f lex.yy.c y.tab.c y.tab.h
