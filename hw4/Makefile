TARGET = parser
OBJECT = lex.yy.c y.tab.c y.tab.o lex.yy.o y.output parser y.tab.h main.o symtable.o semcheck.o error.o
CC = gcc -g -O0
LEX = flex
YACC = yacc -v
YACCFLAG = -d
LIBS = -ll

all: $(TARGET)

parser: y.tab.o lex.yy.o main.o symtable.o semcheck.o error.o
	$(CC) -o $(TARGET) y.tab.o lex.yy.o main.o symtable.o semcheck.o error.o $(LIBS)

y.tab.o: y.tab.c
	$(CC) -c y.tab.c

y.tab.c: parser.y symtable.h datatype.h
	$(YACC) $(YACCFLAG) parser.y

lex.yy.o: lex.yy.c
	$(CC) -c lex.yy.c

lex.yy.c: lex.l datatype.h
	$(LEX) lex.l

main.o: main.c symtable.h datatype.h
	$(CC) -c main.c

symtable.o: symtable.c symtable.h
	$(CC) -c symtable.c

semcheck.o: semcheck.c semcheck.h
	$(CC) -c semcheck.c

error.o: error.c error.h
	$(CC) -c error.c

clean:
	rm -f $(TARGET) $(OBJECT)
