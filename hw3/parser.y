%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int Opt_Symbol;
extern int linenum;
extern FILE *yyin;
extern char *yytext;
extern char buf[256];

int yylex();
int yyerror( char *msg );

typedef struct e {
  struct e *next;
  struct e *parameter;
  char name[33];
  int kind;
  int type;
  int decl;
  int array_dim;
  int *array_size;
  union {
    int value;
    double dval;
    char *text;
  } attribute;
} Entry;

typedef struct t {
  struct t* prev;
  struct t* next;
  Entry* entry;
} Table;

int level = -1;
Table *head = NULL, *curr = NULL;
Entry *parameter;

enum Kind { Function, Parameter, Variable, Constant };
enum Type { Void, Int, Float, Double, Bool, String };
char kind_string[4][10] = { "function", "parameter", "variable", "constant" };
char scope_string[2][7] = { "global", "local" };
char type_string[6][7] = { "void", "int", "float", "double", "bool", "string" };
int type;
int array_dim = 0;
int* array_size = NULL;
union {
  int value;
  double dval;
  char* text;
} attribute;

void push_table();
void pop_table();
void push_entry(Entry *entry);
void pop_all_entries(Table *table);
void entry(int kind, int type, char* name, int decl);
void print_entry(Entry *entry);
void push_array_size(int size);
void reset_array();
void push_parameter(Entry *entry);
void reset_parameter();

%}

%union {
  int type;
  int value;
  double dval;
  char* text;
}

%type <text>  array_decl ID STR_CONST
%type <type>  scalar_type VOID INT FLOAT DOUBLE BOOL STRING
%type <value> INT_CONST
%type <dval>  FLOAT_CONST SCIENTIFIC

%token  ID
%token  INT_CONST
%token  FLOAT_CONST
%token  SCIENTIFIC
%token  STR_CONST

%token  LE_OP
%token  NE_OP
%token  GE_OP
%token  EQ_OP
%token  AND_OP
%token  OR_OP

%token  READ
%token  BOOLEAN
%token  WHILE
%token  DO
%token  IF
%token  ELSE
%token  TRUE
%token  FALSE
%token  FOR
%token  INT
%token  PRINT
%token  BOOL
%token  VOID
%token  FLOAT
%token  DOUBLE
%token  STRING
%token  CONTINUE
%token  BREAK
%token  RETURN
%token  CONST

%token  L_PAREN
%token  R_PAREN
%token  COMMA
%token  SEMICOLON
%token  ML_BRACE
%token  MR_BRACE
%token  L_BRACE
%token  R_BRACE
%token  ADD_OP
%token  SUB_OP
%token  MUL_OP
%token  DIV_OP
%token  MOD_OP
%token  ASSIGN_OP
%token  LT_OP
%token  GT_OP
%token  NOT_OP

/*  Program
    Function
    Array
    Const
    IF
    ELSE
    RETURN
    FOR
    WHILE
*/

%start program
%%

push
  : { push_table(); }
;

pop
  : { pop_table(); }
;

program
  : push decl_list funct_def decl_and_def_list pop
;

decl_list
  : decl_list var_decl
  | decl_list const_decl
  | decl_list funct_decl
  |
;


decl_and_def_list
  : decl_and_def_list var_decl
  | decl_and_def_list const_decl
  | decl_and_def_list funct_decl
  | decl_and_def_list funct_def
  |
;

funct_def
  : scalar_type ID L_PAREN R_PAREN compound_statement
    { entry(Function, $1, $2, 0); reset_parameter(); }
  | scalar_type ID L_PAREN parameter_list R_PAREN compound_statement
    { entry(Function, $1, $2, 0); reset_parameter(); }
  | VOID ID L_PAREN R_PAREN compound_statement
    { entry(Function, $1, $2, 0); reset_parameter(); }
  | VOID ID L_PAREN parameter_list R_PAREN compound_statement
    { entry(Function, $1, $2, 0); reset_parameter(); }
;

funct_decl
  : scalar_type ID L_PAREN R_PAREN SEMICOLON
    { entry(Function, $1, $2, 1); }
  | scalar_type ID L_PAREN parameter_list R_PAREN SEMICOLON
    { entry(Function, $1, $2, 1); }
  | VOID ID L_PAREN R_PAREN SEMICOLON
    { entry(Function, $1, $2, 1); }
  | VOID ID L_PAREN parameter_list R_PAREN SEMICOLON
    { entry(Function, $1, $2, 1); }
;

parameter_list
  : parameter_list COMMA scalar_type ID
    { entry(Parameter, $3, $4, 0); }
  | parameter_list COMMA scalar_type array_decl
    { entry(Parameter, $3, $4, 0); reset_array(); }
  | scalar_type ID
    { entry(Parameter, $1, $2, 0); }
  | scalar_type array_decl
    { entry(Parameter, $1, $2, 0); reset_array(); }
;

var_decl
  : scalar_type identifier_list SEMICOLON
;

identifier_list
  : identifier_list COMMA ID
    { entry(Variable, type, $3, 0); }
  | identifier_list COMMA ID ASSIGN_OP logical_expression
    { entry(Variable, type, $3, 0); }
  | identifier_list COMMA array_decl ASSIGN_OP initial_array
    { entry(Variable, type, $3, 0); reset_array(); }
  | identifier_list COMMA array_decl
    { entry(Variable, type, $3, 0); reset_array(); }
  | array_decl ASSIGN_OP initial_array
    { entry(Variable, type, $1, 0); reset_array(); }
  | array_decl
    { entry(Variable, type, $1, 0); reset_array(); }
  | ID ASSIGN_OP logical_expression
    { entry(Variable, type, $1, 0); }
  | ID
    { entry(Variable, type, $1, 0); }
;

initial_array
  : L_BRACE literal_list R_BRACE
;

literal_list
  : literal_list COMMA logical_expression
  | logical_expression
  |
;

const_decl
  : CONST scalar_type const_list SEMICOLON
;

const_list
  : const_list COMMA ID ASSIGN_OP literal_const
    { entry(Constant, type, $3, 0); }
  | ID ASSIGN_OP literal_const
    { entry(Constant, type, $1, 0); }
;

array_decl
  : ID dim { $$ = $1; }
;

dim
  : dim ML_BRACE INT_CONST MR_BRACE
    { push_array_size($3); }
  | ML_BRACE INT_CONST MR_BRACE
    { push_array_size($2); }
;

compound_statement
  : L_BRACE push var_const_stmt_list pop R_BRACE
;

var_const_stmt_list
  : var_const_stmt_list statement
  | var_const_stmt_list var_decl
  | var_const_stmt_list const_decl
  |
;

statement
  : compound_statement
  | simple_statement
  | conditional_statement
  | while_statement
  | for_statement
  | function_invoke_statement
  | jump_statement
;

simple_statement
  : variable_reference ASSIGN_OP logical_expression SEMICOLON
  | PRINT logical_expression SEMICOLON
  | READ variable_reference SEMICOLON
;

conditional_statement
  : IF L_PAREN logical_expression R_PAREN compound_statement
  | IF L_PAREN logical_expression R_PAREN
      compound_statement
    ELSE
      compound_statement
;

while_statement
  : WHILE L_PAREN logical_expression R_PAREN
      compound_statement
  | DO L_BRACE
      var_const_stmt_list
    R_BRACE WHILE L_PAREN logical_expression R_PAREN SEMICOLON
;

for_statement :
  FOR L_PAREN initial_expression_list SEMICOLON
    control_expression_list SEMICOLON increment_expression_list R_PAREN
    compound_statement
;

initial_expression_list
  : initial_expression
  |
;

initial_expression
  : initial_expression COMMA variable_reference ASSIGN_OP logical_expression
  | initial_expression COMMA logical_expression
  | logical_expression
  | variable_reference ASSIGN_OP logical_expression
;

control_expression_list
  : control_expression
  |
;

control_expression
  : control_expression COMMA variable_reference ASSIGN_OP logical_expression
  | control_expression COMMA logical_expression
  | logical_expression
  | variable_reference ASSIGN_OP logical_expression
;

increment_expression_list
  : increment_expression
  |
;

increment_expression
  : increment_expression COMMA variable_reference ASSIGN_OP logical_expression
  | increment_expression COMMA logical_expression
  | logical_expression
  | variable_reference ASSIGN_OP logical_expression
;

function_invoke_statement
  : ID L_PAREN logical_expression_list R_PAREN SEMICOLON
  | ID L_PAREN R_PAREN SEMICOLON
;

jump_statement
  : CONTINUE SEMICOLON
  | BREAK SEMICOLON
  | RETURN logical_expression SEMICOLON
;

variable_reference
  : array_list
  | ID
;


logical_expression
  : logical_expression OR_OP logical_term
  | logical_term
;

logical_term
  : logical_term AND_OP logical_factor
  | logical_factor
;

logical_factor
  : NOT_OP logical_factor
  | relation_expression
;

relation_expression
  : relation_expression relation_operator arithmetic_expression
  | arithmetic_expression
;

relation_operator
  : LT_OP
  | LE_OP
  | EQ_OP
  | GE_OP
  | GT_OP
  | NE_OP
;

arithmetic_expression
  : arithmetic_expression ADD_OP term
  | arithmetic_expression SUB_OP term
  | term
;

term
  : term MUL_OP factor
  | term DIV_OP factor
  | term MOD_OP factor
  | factor
;

factor
  : SUB_OP factor
  | literal_const
  | variable_reference
  | L_PAREN logical_expression R_PAREN
  | ID L_PAREN logical_expression_list R_PAREN
  | ID L_PAREN R_PAREN
;

logical_expression_list
  : logical_expression_list COMMA logical_expression
  | logical_expression
;

array_list
  : ID dimension
;

dimension
  : dimension ML_BRACE logical_expression MR_BRACE
  | ML_BRACE logical_expression MR_BRACE
;

scalar_type
  : INT    { $$ = $1; type = $1; }
  | DOUBLE { $$ = $1; type = $1; }
  | STRING { $$ = $1; type = $1; }
  | BOOL   { $$ = $1; type = $1; }
  | FLOAT  { $$ = $1; type = $1; }
;

literal_const
  : INT_CONST   { attribute.value = $1; }
  | FLOAT_CONST { attribute.dval = $1; }
  | SCIENTIFIC  { attribute.dval = $1; }
  | STR_CONST   { attribute.text = $1; }
  | TRUE
  | FALSE
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

void push_table()
{
  if (head == NULL) {
    head = (Table*)malloc(sizeof(Table));
    curr = head;
    curr->prev = NULL;
  } else {
    curr->next = (Table*)malloc(sizeof(Table));
    curr->next->prev = curr;
    curr = curr->next;
  }
  curr->next = NULL;
  curr->entry = parameter;
  reset_parameter();
  level += 1;
}

void pop_table()
{
  if (head == NULL) exit(1);
  if (curr == head) {
    pop_all_entries(curr);
    curr = NULL;
    head = NULL;
  } else {
    curr = curr->prev;
    pop_all_entries(curr->next);
    curr->next = NULL;
  }
  level -= 1;
}

void push_entry(Entry *new_entry)
{
  if (curr->entry == NULL) {
    curr->entry = new_entry;
  } else {
    Entry *entry;
    for (entry = curr->entry; entry != NULL; entry = entry->next) {
      if (strcmp(entry->name, new_entry->name) == 0) {
        if (new_entry->kind == Function && entry->decl && !new_entry->decl)
          return;
        printf("##########Error at Line %d: %s redeclared.##########\n",
          linenum, entry->name);
        return;
      }
    }
    for (entry = curr->entry; entry->next != NULL; entry = entry->next);
    entry->next = new_entry;
  }
}

void pop_all_entries(Table *table)
{
  if (table->entry == NULL) return;
  Entry *curr = table->entry, *prev = NULL;
  printf("=======================================================================================\n");
  printf( "Name                             Kind       Level       Type               Attribute               \n");
  printf("---------------------------------------------------------------------------------------\n");
  while (curr != NULL) {
    print_entry(curr);
    curr = curr->next;
    prev = curr;
  }
  printf("=======================================================================================\n");
}

void push_array_size(int size)
{
  array_dim += 1;
  if (array_size == NULL)
    array_size = (int*)malloc(sizeof(int));
  else
    array_size = (int*)realloc(array_size, array_dim * sizeof(int));
  array_size[array_dim-1] = size;
}

void reset_array()
{
  array_dim = 0;
  array_size = NULL;
}

void push_parameter(Entry *new_entry)
{
  if (parameter == NULL) {
    parameter = new_entry;
  } else {
    Entry *entry;
    for (entry = parameter; entry->next != NULL; entry = entry->next) {
      if (strcmp(entry->name, new_entry->name) == 0) {
        printf("##########Error at Line %d: %s redeclared.##########\n",
          linenum, entry->name);
        return;
      }
    }
    entry->next = new_entry;
  }
}

void reset_parameter()
{
  parameter = NULL;
}

void entry(int kind, int type, char* name, int decl)
{
  Entry* entry = (Entry*)malloc(sizeof(Entry));
  entry->next = NULL;
  entry->kind = kind;
  entry->type = type;
  entry->decl = decl;
  strncpy(entry->name, name, 32);
  entry->name[32] = '\0';
  entry->array_dim = array_dim;
  entry->array_size = array_size;
  switch (kind) {
    case Parameter:
      push_parameter(entry);
      break;
    case Function:
      entry->parameter = parameter;
      push_entry(entry);
      reset_parameter();
      break;
    case Constant:
      switch (type) {
        case Int:
          entry->attribute.value = attribute.value;
          break;
        case Float:
        case Double:
          entry->attribute.dval = attribute.dval;
          break;
        case String:
          entry->attribute.text = attribute.text;
          break;
      }
    default:
      push_entry(entry);
  }
}

void print_entry(Entry *entry)
{
  char str[20];
  printf("%-33s", entry->name);
  printf("%-11s", kind_string[entry->kind]);
  sprintf(str, "%d(%s)", level, scope_string[(level != 0)]);
  printf("%-12s", str);
  sprintf(str, "%s", type_string[entry->type]);
  int i;
  for (i = 0; i < entry->array_dim; ++i) {
    sprintf(str, "%s[%d]", str, entry->array_size[i]);
  }
  printf("%-19s", str);
  if (entry->kind == Constant) {
    switch (entry->type) {
      case Int:
        printf("%-24d", entry->attribute.value);
        break;
      case Float:
      case Double:
        printf("%-24lf", entry->attribute.dval);
        break;
      case String:
        printf("%-24s", entry->attribute.text);
        break;
    }
  } else if (entry->kind == Function && entry->parameter != NULL) {
    Entry *p;
    for (p = entry->parameter; p != NULL; p = p->next) {
      if (p != entry->parameter) printf(",");
      int i;
      printf("%s", type_string[p->type]);
      for (i = 0; i < p->array_dim; ++i) {
        printf("[%d]", p->array_size[i]);
      }
    }
  }
  printf("\n");
}
