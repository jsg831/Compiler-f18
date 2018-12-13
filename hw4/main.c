#include <stdio.h>
#include <stdlib.h>
#include "datatype.h"
#include "symtable.h"

extern int yyparse();
extern FILE* yyin;
extern struct SymTableList *symbolTableList;
extern int errorCount;

int  main( int argc, char **argv )
{

  if( argc == 1 )
  {
    yyin = stdin;
  }
  else if( argc == 2 )
  {
    FILE *fp = fopen( argv[1], "r" );
    if( fp == NULL ) {
      fprintf( stderr, "Open file error\n" );
      exit(-1);
    }
    yyin = fp;
  }
  else
  {
    fprintf( stderr, "Usage: ./parser [filename]\n" );
    exit(0);
  }

  symbolTableList = (struct SymTableList*)malloc(sizeof(struct SymTableList));
  initSymTableList(symbolTableList);
  AddSymTable(symbolTableList);//global
  yyparse();  /* primary procedure of parser */

  destroySymTableList(symbolTableList);
  if (errorCount == 0) {
    fprintf( stdout, "\n|---------------------------------------------|\n" );
    fprintf( stdout, "|  There is no syntactic and semantic error!  |\n" );
    fprintf( stdout, "|---------------------------------------------|\n" );
  } else {
    fprintf( stdout, "\n|-------------------------------------|\n" );
    if (errorCount > 999)
      fprintf( stdout, "| There are too many semantic errors! |\n" );
    else if (errorCount > 99)
      fprintf( stdout, "|   There are %3d semantic errors!    |\n", errorCount );
    else if (errorCount > 9)
      fprintf( stdout, "|    There are %2d semantic errors!    |\n", errorCount );
    else if (errorCount > 1)
      fprintf( stdout, "|    There are %1d semantic errors!     |\n", errorCount );
    else
      fprintf( stdout, "|     There is 1 semantic error!      |\n");
    fprintf( stdout, "|-------------------------------------|\n" );
  }
  exit(0);
}
