%{
#include <stdio.h>
#include <stdlib.h>

extern int linenum;             /* declared in lex.l */
extern FILE *yyin;              /* declared by lex */
extern char *yytext;            /* declared by lex */
extern char buf[256];           /* declared in lex.l */

int has_func_def = 0;

%}

%token IDENTIFIER                                    /* identifier */
%token CONST                                         /* constant */
%token INT FLOAT DOUBLE BOOL STRING                  /* type */
%token VOID                                          /* void */
%token I_CONST F_CONST STRING_LITERAL                /* value */
%token LOGIC                                         /* logical value*/
%token EQ NE LT LE GE GT                             /* relational operator */
%token AND OR                                        /* logical operator */
%token IF ELSE                                       /* conditional statement */
%token FOR WHILE DO                                  /* loop statement */
%token READ PRINT                                    /* function statement */
%token RETURN BREAK CONTINUE                         /* jump statement */

%left OR
%left AND
%right '!'
%left EQ NE
%left LT LE GE GT
%left '+' '-'
%left '*' '/' '%'
%right '='

%%

program
  : func_def program
  | func_decl program
  | decl_stmt program
  |
;

func_decl
  : scalar_type IDENTIFIER '(' opt_params ')' ';'
  | VOID IDENTIFIER '(' opt_params ')' ';'
;

func_def
  : scalar_type IDENTIFIER '(' opt_params ')' compound_stmt { has_func_def = 1; }
  | VOID IDENTIFIER '(' opt_params ')' compound_stmt { has_func_def = 1; }
;

opt_params
  : param_list
  |
;

param_list
  : param_list ',' scalar_type IDENTIFIER opt_indices
  | scalar_type IDENTIFIER opt_indices
;

opt_indices
  : '[' expr ']' opt_indices
  |
;

stmt
  : decl_stmt
  | compound_stmt
  | assign_stmt
  | simple_stmt
  | cond_stmt
  | while_stmt
  | for_stmt
  | jump_stmt
  | func_stmt
;

decl_stmt
  : var_decl
  | const_decl
;

var_decl
  : scalar_type identifier_list ';'
;

identifier_list
  : identifier_list ',' identifier_decl
  | identifier_list ',' identifier_def
  | identifier_decl
  | identifier_def
;

identifier_decl
  : IDENTIFIER
  | IDENTIFIER indices
;

identifier_def
  : IDENTIFIER '=' expr
  | IDENTIFIER indices '=' '{' opt_expr_list '}'
;

const_decl
  : CONST scalar_type const_list ';'
;

const_list
  : const_list ',' const_identifier_def
  | const_identifier_def
;

const_identifier_def
  : IDENTIFIER '=' literal
;

indices
  : '[' I_CONST ']' indices
  | '[' I_CONST ']'
;

opt_expr_list
  : expr_list
  |
;

expr_list
  : expr_list ',' expr
  | expr
;

compound_stmt
  : '{' stmt_list '}'

stmt_list
  : stmt_list stmt
  |
;

assign_stmt
  : assign_expr ';'
;

simple_stmt
  : PRINT expr ';'
  | READ expr ';'
;

cond_stmt
  : IF '(' expr ')' compound_stmt ELSE compound_stmt
  | IF '(' expr ')' compound_stmt
;

while_stmt
  : WHILE '(' expr ')' compound_stmt
  | DO compound_stmt WHILE '(' expr ')' ';'
;

for_stmt
  : FOR '(' for_opt_expr_list ';' for_opt_expr_list ';' for_opt_expr_list ')' compound_stmt
;

for_opt_expr_list
  : for_expr_list
  |
;

for_expr_list
  : for_expr_list ',' expr
  | for_expr_list ',' assign_expr
  | expr
  | assign_expr
;

jump_stmt
  : RETURN expr ';'
  | BREAK ';'
  | CONTINUE ';'
;

func_stmt
  : call ';'
;

expr
  : '-' expr %prec '*'
  | expr '*' expr
  | expr '/' expr
  | expr '%' expr
  | expr '+' expr
  | expr '-' expr
  | expr LT expr
  | expr LE expr
  | expr GT expr
  | expr GE expr
  | expr EQ expr
  | expr NE expr
  | '!' expr
  | expr AND expr
  | expr OR expr
  | factor
;

factor
  : '(' expr ')'
  | IDENTIFIER
  | IDENTIFIER opt_indices
  | call
  | literal
;

assign_expr
  : IDENTIFIER opt_indices '=' expr
;

call
  : IDENTIFIER '(' opt_expr_list ')'
;

scalar_type
  : INT
  | FLOAT
  | DOUBLE
  | BOOL
  | STRING
;

literal
  : I_CONST
  | F_CONST
  | STRING_LITERAL
  | LOGIC
;

%%

int yyerror( char *msg )
{
  fprintf( stderr, "\n|--------------------------------------------------------------------------\n" );
  fprintf( stderr, "| Error found in Line #%d: %s\n", linenum, buf );
  fprintf( stderr, "|\n" );
  fprintf( stderr, "| Unmatched token: %s\n", yytext );
  fprintf( stderr, "|--------------------------------------------------------------------------\n" );
  exit(-1);
}

int  main( int argc, char **argv )
{
  if( argc != 2 ) {
    fprintf(  stdout,  "Usage:  ./parser  [filename]\n"  );
    exit(0);
  }

  FILE *fp = fopen( argv[1], "r" );

  if( fp == NULL )  {
    fprintf( stdout, "Open  file  error\n" );
    exit(-1);
  }

  yyin = fp;
  yyparse();

  if ( has_func_def == 0 ) yyerror("");

  fprintf( stdout, "\n" );
  fprintf( stdout, "|--------------------------------|\n" );
  fprintf( stdout, "|  There is no syntactic error!  |\n" );
  fprintf( stdout, "|--------------------------------|\n" );
  exit(0);
}
