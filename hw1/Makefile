all: lex gcc clean

lex: scanner.l
	lex scanner.l

gcc: lex.yy.c
	gcc -o scanner lex.yy.c -ll

clean: lex.yy.c
	rm -f lex.yy.c
