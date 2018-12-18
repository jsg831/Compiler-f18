%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "datatype.h"
#include "symtable.h"
#include "semcheck.h"
#include "error.h"

extern int linenum;
extern FILE  *yyin;
extern char  *yytext;
extern char buf[256];
extern int Opt_SymTable; // declared in lex.l
int scope = 0; // default is 0(global)
struct SymTableList *symbolTableList; // create and initialize in main.c
struct ExtType *funcReturnType;
int errorCount = 0;
bool lastReturn = false;
int insideLoop = 0;
BTYPE baseType = VOID_t;

%}
%union{
  int                  intVal;
  float                floatVal;
  double               doubleVal;
  char                 *stringVal;
  char                 *idName;
  //struct ExtType     *extType;
  struct Variable      *variable;
  struct VariableList  *variableList;
  struct InitArray *initialArray;
  struct ArrayDimNode  *arrayDimNode;
  //struct ConstAttr   *constAttr;
  struct FuncAttrNode  *funcAttrNode;
  //struct FuncAttr    *funcAttr;
  struct Attribute     *attribute;
  struct SymTableNode  *symTableNode;
  //struct SymTable    *symTable;
  BTYPE                bType;
  struct Expr          *expr;
  struct ExprList      *exprList;
};

%token <idName>      ID
%token <intVal>      INT_CONST
%token <floatVal>    FLOAT_CONST
%token <doubleVal>   SCIENTIFIC
%token <stringVal>   STR_CONST

%type <variable>     array_decl
%type <variableList> identifier_list
%type <initialArray> initial_array literal_list
%type <arrayDimNode> dim dimension
%type <funcAttrNode> parameter_list
%type <attribute>    literal_const
%type <symTableNode> const_list
%type <bType>        scalar_type
%type <expr>         function_invoke_statement variable_reference array_list factor term arithmetic_expression relation_expression logical_factor logical_term logical_expression
%type <exprList>     logical_expression_list

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

program
  : decl_list funct_def decl_and_def_list
  {
    if(Opt_SymTable == 1)
      printSymTable(symbolTableList->global);
    deleteLastSymTable(symbolTableList);
  }
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
  : scalar_type ID L_PAREN R_PAREN
    {
      funcReturnType = createExtType($1,0,NULL);
      struct SymTableNode *node;
      node = findFuncDeclaration(symbolTableList->global,$2);
      if(node == NULL) //no declaration yet
      {
        struct SymTableNode *newNode = createFunctionNode($2,scope,funcReturnType,NULL,false);
        insertTableNode(symbolTableList->global,newNode);
      } else if (node->decl) {
        checkFuncAttr(node,NULL,$1);
      } else {
        printError("redefinition of function '%s'", node->name);
      }
    }
    compound_statement
    {
      if (!lastReturn)
        printError("no return statement at the end of non-void function '%s'", $2);
      lastReturn = false;
      free($2);
    }
  | scalar_type ID L_PAREN parameter_list R_PAREN
    {
      funcReturnType = createExtType($1,0,NULL);
      struct SymTableNode *node;
      node = findFuncDeclaration(symbolTableList->global,$2);
      struct Attribute *attr = createFunctionAttribute($4);
      if(node == NULL)//no declaration yet
      {
        struct SymTableNode *newNode = createFunctionNode($2,scope,funcReturnType,attr,false);
        insertTableNode(symbolTableList->global,newNode);
      } else if (node->decl) {
        checkFuncAttr(node,$4,$1);
      } else {
        printError("redefinition of function '%s'", node->name);
      }
    }
    L_BRACE
    {//enter a new scope
      ++scope;
      AddSymTable(symbolTableList);
      //add parameters
      struct FuncAttrNode *attrNode = $4;
      while(attrNode != NULL)
      {
        struct SymTableNode *newNode = createParameterNode(attrNode->name,scope,attrNode->value);
        insertTableNode(symbolTableList->tail,newNode);
        attrNode = attrNode->next;
      }
    }
    var_const_stmt_list R_BRACE
    {
      if(Opt_SymTable == 1)
        printSymTable(symbolTableList->tail);
      deleteLastSymTable(symbolTableList);
      --scope;
      if (!lastReturn)
        printError("no return statement at the end of non-void function '%s'", $2);
      lastReturn = false;
      free($2);
    }
  | VOID ID L_PAREN R_PAREN
    {
      funcReturnType = createExtType(VOID_t,0,NULL);
      struct SymTableNode *node;
      node = findFuncDeclaration(symbolTableList->global,$2);
      if(node == NULL)//no declaration yet
      {
        struct SymTableNode *newNode = createFunctionNode($2,scope,funcReturnType,NULL,false);
        insertTableNode(symbolTableList->global,newNode);
      } else if (node->decl) {
        checkFuncAttr(node,NULL,VOID_t);
      } else {
        printError("redefinition of function '%s'", node->name);
      }
      free($2);
    }
    compound_statement
    {
      lastReturn = false;
    }
  | VOID ID L_PAREN parameter_list R_PAREN
    {
      funcReturnType = createExtType(VOID_t,0,NULL);
      struct SymTableNode *node;
      node = findFuncDeclaration(symbolTableList->global,$2);
      if(node == NULL)//no declaration yet
      {
        struct Attribute *attr = createFunctionAttribute($4);
        struct SymTableNode *newNode = createFunctionNode($2,scope,funcReturnType,attr,false);
        insertTableNode(symbolTableList->global,newNode);
      } else if (node->decl) {
        checkFuncAttr(node,$4,VOID_t);
      } else {
        printError("redefinition of function '%s'", node->name);
      }
    }
    L_BRACE
    {
      //enter a new scope
      ++scope;
      AddSymTable(symbolTableList);
      //add parameters
      struct FuncAttrNode *attrNode = $4;
      while(attrNode != NULL)
      {
        struct SymTableNode *newNode = createParameterNode(attrNode->name,scope,attrNode->value);
        insertTableNode(symbolTableList->tail,newNode);
        attrNode = attrNode->next;
      }
    }
    var_const_stmt_list R_BRACE
      {
        if(Opt_SymTable == 1)
          printSymTable(symbolTableList->tail);
        deleteLastSymTable(symbolTableList);
        --scope;
        free($2);
        lastReturn = false;
      }
;

funct_decl
  : scalar_type ID L_PAREN R_PAREN SEMICOLON
    {
      funcReturnType = createExtType($1,0,NULL);
      struct SymTableNode *newNode = createFunctionNode($2,scope,funcReturnType,NULL,true);
      struct SymTableNode *node = findFuncDeclaration(symbolTableList->global,$2);
      if (node == NULL)
        insertTableNode(symbolTableList->global,newNode);
      else
        printError("redeclaration of function '%s'", node->name);
      free($2);
    }
  | scalar_type ID L_PAREN parameter_list R_PAREN SEMICOLON
    {
      funcReturnType = createExtType($1,0,NULL);
      struct Attribute *attr = createFunctionAttribute($4);
      struct SymTableNode *newNode = createFunctionNode($2,scope,funcReturnType,attr,true);
      struct SymTableNode *node = findFuncDeclaration(symbolTableList->global,$2);
      if (node == NULL)
        insertTableNode(symbolTableList->global,newNode);
      else
        printError("redeclaration of function '%s'", node->name);
      free($2);
    }
  | VOID ID L_PAREN R_PAREN SEMICOLON
    {
      funcReturnType = createExtType(VOID_t,0,NULL);
      struct SymTableNode *newNode = createFunctionNode($2,scope,funcReturnType,NULL,true);
      struct SymTableNode *node = findFuncDeclaration(symbolTableList->global,$2);
      if (node == NULL)
        insertTableNode(symbolTableList->global,newNode);
      else
        printError("redeclaration of function '%s'", node->name);
      free($2);
    }
  | VOID ID L_PAREN parameter_list R_PAREN SEMICOLON
    {
      funcReturnType = createExtType(VOID_t,0,NULL);
      struct Attribute *attr = createFunctionAttribute($4);
      struct SymTableNode *newNode = createFunctionNode($2,scope,funcReturnType,attr,true);
      struct SymTableNode *node = findFuncDeclaration(symbolTableList->global,$2);
      if (node == NULL)
        insertTableNode(symbolTableList->global,newNode);
      else
        printError("redeclaration of function '%s'", node->name);
      free($2);
    }
;

parameter_list
  : parameter_list COMMA scalar_type ID
    {
      struct FuncAttrNode *newNode = (struct FuncAttrNode*)malloc(sizeof(struct FuncAttrNode));
      newNode->value = createExtType($3,0,NULL);
      newNode->name = strdup($4);
      free($4);
      newNode->next = NULL;
      connectFuncAttrNode($1,newNode);
      $$ = $1;
    }
  | parameter_list COMMA scalar_type array_decl
    {
      struct FuncAttrNode *newNode = (struct FuncAttrNode*)malloc(sizeof(struct FuncAttrNode));
      newNode->value = $4->type;//use pre-built ExtType(type is unknown)
      newNode->value->baseType = $3;//set correct type
      newNode->name = strdup($4->name);
      newNode->next = NULL;
      free($4->name);
      free($4);
      connectFuncAttrNode($1,newNode);
      $$ = $1;

    }
  | scalar_type array_decl
    {
      struct FuncAttrNode *newNode = (struct FuncAttrNode*)malloc(sizeof(struct FuncAttrNode));
      newNode->value = $2->type;//use pre-built ExtType(type is unknown)
      newNode->value->baseType = $1;//set correct type
      newNode->name = strdup($2->name);
      newNode->next = NULL;
      free($2->name);
      free($2);
      $$ = newNode;
    }
  | scalar_type ID
    {
      struct FuncAttrNode *newNode = (struct FuncAttrNode*)malloc(sizeof(struct FuncAttrNode));
      newNode->value = createExtType($1,0,NULL);
      newNode->name = strdup($2);
      free($2);
      newNode->next = NULL;
      $$ = newNode;
    }
;

var_decl
  : scalar_type identifier_list SEMICOLON
    {
      struct Variable* listNode = $2->head;
      struct SymTableNode *newNode;
      while(listNode != NULL)
      {
        newNode = createVariableNode(listNode->name,scope,listNode->type);
        insertTableNode(symbolTableList->tail,newNode);
        listNode = listNode->next;
      }
      deleteVariableList($2);
    }
;

identifier_list
  : identifier_list COMMA ID
    {
      struct ExtType *type = createExtType(baseType,false,NULL);//type unknown here
      struct Variable *newVariable = createVariable($3,type);
      free($3);
      connectVariableList($1,newVariable);
      $$ = $1;
    }
  | identifier_list COMMA ID ASSIGN_OP logical_expression
    {
      struct ExtType *type = createExtType(baseType,false,NULL);//type unknown here
      struct Variable *newVariable = createVariable($3,type);
      struct Expr* tempExpr = createVariableExpr(newVariable);
      checkAssignOp(tempExpr,$5);
      deleteExpr(tempExpr);
      free($3);
      connectVariableList($1,newVariable);

      $$ = $1;
    }
  | identifier_list COMMA array_decl ASSIGN_OP initial_array
    {
      connectVariableList($1,$3);
      checkInitArray($3,$5);
      $$ = $1;
    }
  | identifier_list COMMA array_decl
    {
      connectVariableList($1,$3);
      $$ = $1;
    }
  | array_decl ASSIGN_OP initial_array
    {
      $$ = createVariableList($1);
      checkInitArray($1,$3);
    }
  | array_decl
    {
      $$ = createVariableList($1);
    }
  | ID ASSIGN_OP logical_expression
    {

      struct ExtType *type = createExtType(baseType,false,NULL);
      struct Variable *newVariable = createVariable($1,type);
      struct Expr* tempExpr = createVariableExpr(newVariable);
      checkAssignOp(tempExpr,$3);
      deleteExpr(tempExpr);
      $$ = createVariableList(newVariable);
      free($1);
    }
  | ID
    {
      struct ExtType *type = createExtType(baseType,false,NULL);
      struct Variable *newVariable = createVariable($1,type);
      $$ = createVariableList(newVariable);
      free($1);
    }
;

initial_array
  : L_BRACE literal_list R_BRACE
    {
      $$ = $2;
    }
;

literal_list
  : literal_list COMMA logical_expression
    {
      connectInitArray($1);
      $$ = $1;
    }
  | logical_expression
    {
      $$ = createInitArray();
    }
  |
    {
      $$ = NULL;
    }
;

const_decl
  : CONST scalar_type const_list SEMICOLON
  {
    struct SymTableNode *list = $3;//symTableNode base on initailized data type, scalar_type is not used
    while(list != NULL)
    {
      insertTableNode(symbolTableList->tail,list);
      list = list->next;
    }
  }
;

const_list
  : const_list COMMA ID ASSIGN_OP literal_const
    {
      struct ExtType *type = createExtType(baseType,false,NULL);
      struct SymTableNode *temp = $1;
      while(temp->next != NULL)
      {
        temp = temp->next;
      }
      temp->next = createConstNode($3,scope,type,$5);
      free($3);
    }
  | ID ASSIGN_OP literal_const
    {
      struct ExtType *type = createExtType(baseType,false,NULL);
      $$ = createConstNode($1,scope,type,$3);
      free($1);
    }
;

array_decl
  : ID dim
    {
      struct ExtType *type = createExtType(baseType,true,$2);
      struct Variable *newVariable = createVariable($1,type);
      free($1);
      $$ = newVariable;
      checkArrayVariable($$);
    }
;

dim
  : dim ML_BRACE INT_CONST MR_BRACE
    {
      connectArrayDimNode($1,createArrayDimNode($3));
      $$ = $1;
    }
  | ML_BRACE INT_CONST MR_BRACE
    {
      $$ = createArrayDimNode($2);
    }
;

compound_statement
  : L_BRACE
    {//enter a new scope
      ++scope;
      AddSymTable(symbolTableList);
    }
    var_const_stmt_list R_BRACE
    {
      if(Opt_SymTable == 1)
        printSymTable(symbolTableList->tail);
      deleteLastSymTable(symbolTableList);
      --scope;
    }
;

var_const_stmt_list
  : var_const_stmt_list statement
  | var_const_stmt_list var_decl
  | var_const_stmt_list const_decl
  |
;

statement
  : compound_statement { lastReturn = false; }
  | simple_statement { lastReturn = false; }
  | conditional_statement { lastReturn = false; }
  | while_statement { lastReturn = false; }
  | for_statement { lastReturn = false; }
  | function_invoke_statement { lastReturn = false; }
  | jump_statement
;

simple_statement
  : variable_reference ASSIGN_OP logical_expression SEMICOLON
    {
      checkAssignOp($1,$3);
      deleteExpr($1);
      deleteExpr($3);
    }
  | PRINT logical_expression SEMICOLON
    {
      checkPrintRead($2);
      deleteExpr($2);
    }
  | READ variable_reference SEMICOLON
    {
      checkPrintRead($2);
      deleteExpr($2);
    }
;

conditional_statement
  : IF L_PAREN logical_expression R_PAREN
    {
      checkCond($3,"if");
      deleteExpr($3);
    }
    compound_statement

  | IF L_PAREN logical_expression R_PAREN
    {
      checkCond($3,"if");
      deleteExpr($3);
    }
      compound_statement
    ELSE
      compound_statement
;

while_statement
  : WHILE
    {
      //enter a new scope
      ++scope;
      AddSymTable(symbolTableList);
    }
    L_PAREN logical_expression R_PAREN
    {
      insideLoop += 1;
      checkCond($4,"while");
      deleteExpr($4);
    }
    L_BRACE var_const_stmt_list R_BRACE
    {
      insideLoop -= 1;
      if(Opt_SymTable == 1)
        printSymTable(symbolTableList->tail);
      deleteLastSymTable(symbolTableList);
      --scope;
    }
  | DO L_BRACE
    {
      insideLoop += 1;
      //enter a new scope
      ++scope;
      AddSymTable(symbolTableList);
    }
    var_const_stmt_list
    R_BRACE WHILE L_PAREN logical_expression R_PAREN SEMICOLON
    {
      insideLoop -= 1;
      checkCond($8,"while");
      deleteExpr($8);
      if(Opt_SymTable == 1)
        printSymTable(symbolTableList->tail);
      deleteLastSymTable(symbolTableList);
      --scope;
    }
;

for_statement
  : FOR
    {
      //enter a new scope
      ++scope;
      AddSymTable(symbolTableList);
    }
    L_PAREN initial_expression_list SEMICOLON control_expression_list SEMICOLON increment_expression_list R_PAREN
    {
      insideLoop += 1;
    }
    L_BRACE var_const_stmt_list R_BRACE
    {
      insideLoop -= 1;
      if(Opt_SymTable == 1)
        printSymTable(symbolTableList->tail);
      deleteLastSymTable(symbolTableList);
      --scope;
    }
;

initial_expression_list
  : initial_expression
  |
;

initial_expression
  : initial_expression COMMA variable_reference ASSIGN_OP logical_expression
    {
      checkAssignOp($3,$5);
      deleteExpr($3);
      deleteExpr($5);
    }
  | initial_expression COMMA logical_expression
    {
      deleteExpr($3);
    }
  | logical_expression
    {
      deleteExpr($1);
    }
  | variable_reference ASSIGN_OP logical_expression
    {
      checkAssignOp($1,$3);
      deleteExpr($1);
      deleteExpr($3);
    }
;

control_expression_list
  : control_expression
  |
;

control_expression
  : control_expression COMMA variable_reference ASSIGN_OP logical_expression
    {
      printError("control expression must be of type boolean");
      /*
      checkAssignOp($3,$5);
      deleteExpr($3);
      deleteExpr($5);
      */
    }
  | control_expression COMMA logical_expression
    {
      checkCond($3,"for");
      deleteExpr($3);
    }
  | logical_expression
    {
      checkCond($1,"for");
      deleteExpr($1);
    }
  | variable_reference ASSIGN_OP logical_expression
    {
      printError("control expression must be of type boolean");
      /*
      checkAssignOp($1,$3);
      deleteExpr($1);
      deleteExpr($3);
      */
    }
;

increment_expression_list
  : increment_expression
  |
;

increment_expression
  : increment_expression COMMA variable_reference ASSIGN_OP logical_expression
    {
      checkAssignOp($3,$5);
      deleteExpr($3);
      deleteExpr($5);
    }
  | increment_expression COMMA logical_expression
    {
      deleteExpr($3);
    }
  | logical_expression
    {
      deleteExpr($1);
    }
  | variable_reference ASSIGN_OP logical_expression
    {
      checkAssignOp($1,$3);
      deleteExpr($1);
      deleteExpr($3);
    }
;

function_invoke_statement
  : ID L_PAREN logical_expression_list R_PAREN SEMICOLON
    {
      struct SymTableNode* node = findID(symbolTableList,$1);
      checkFunction(node,true);
      checkFuncCall(node, $3);
      deleteExprList($3);
      free($1);
    }
  | ID L_PAREN R_PAREN SEMICOLON
    {
      struct SymTableNode* node = findID(symbolTableList,$1);
      checkFunction(node,true);
      checkFuncCall(node, NULL);
      free($1);
    }
;

jump_statement
  : CONTINUE SEMICOLON
    {
      if (!insideLoop)
        printError("'continue' statement not in loop statement");
      lastReturn = false;
    }
  | BREAK SEMICOLON
    {
      if (!insideLoop)
        printError("'break' statement not in loop statement");
      lastReturn = false;
    }
  | RETURN logical_expression SEMICOLON
    {
      lastReturn = true;
      checkReturn($2,funcReturnType);
      deleteExpr($2);
    }
;

variable_reference
  : array_list
    {
      $$ = $1;
    }
  | ID
    {
      struct SymTableNode* node = findID(symbolTableList, $1);
      if (checkFunction(node,false))
        $$ = createExpr(node);
      else
        $$ = NULL;
      free($1);
    }
;


logical_expression
  : logical_expression OR_OP logical_term
    {
      $$ = checkLogicalOp($1,$3,"||");
      deleteExpr($1);
      deleteExpr($3);
    }
  | logical_term
    {
      $$ = $1;
    }
;

logical_term
  : logical_term AND_OP logical_factor
    {
      $$ = checkLogicalOp($1,$3,"&&");
      deleteExpr($1);
      deleteExpr($3);
    }
  | logical_factor
    {
      $$ = $1;
    }
;

logical_factor
  : NOT_OP logical_factor
    {
      $$ = checkNotOp($2);
      deleteExpr($2);
    }
  | relation_expression
    {
      $$ = $1;
    }
;

relation_expression
  : arithmetic_expression LT_OP arithmetic_expression
    {
      $$ = checkRelOp($1,$3,"<");
      deleteExpr($1);
      deleteExpr($3);
    }
  | arithmetic_expression LE_OP arithmetic_expression
    {
      $$ = checkRelOp($1,$3,"<=");
      deleteExpr($1);
      deleteExpr($3);
    }
  | arithmetic_expression EQ_OP arithmetic_expression
    {
      $$ = checkCompOp($1,$3,"==");
      deleteExpr($1);
      deleteExpr($3);
    }
  | arithmetic_expression GE_OP arithmetic_expression
    {
      $$ = checkRelOp($1,$3,">=");
      deleteExpr($1);
      deleteExpr($3);
    }
  | arithmetic_expression GT_OP arithmetic_expression
    {
      $$ = checkRelOp($1,$3,">");
      deleteExpr($1);
      deleteExpr($3);
    }
  | arithmetic_expression NE_OP arithmetic_expression
    {
      $$ = checkCompOp($1,$3,"!=");
      deleteExpr($1);
      deleteExpr($3);
    }
  | arithmetic_expression
    {
      $$ = $1;
    }
;

arithmetic_expression
  : arithmetic_expression ADD_OP term
    {
      $$ = checkArithOp($1,$3,"+");
      deleteExpr($1);
      deleteExpr($3);
    }
  | arithmetic_expression SUB_OP term
    {
      $$ = checkArithOp($1,$3,"-");
      deleteExpr($1);
      deleteExpr($3);
    }
  | relation_expression
    {
      $$ = $1;
    }
  | term
    {
      $$ = $1;
    }
;

term
  : term MUL_OP factor
    {
      $$ = checkArithOp($1,$3,"*");
      deleteExpr($1);
      deleteExpr($3);
    }
  | term DIV_OP factor
    {
      $$ = checkArithOp($1,$3,"/");
      deleteExpr($1);
      deleteExpr($3);
    }
  | term MOD_OP factor
    {
      $$ = checkModOp($1,$3);
      deleteExpr($1);
      deleteExpr($3);
    }
  | factor
    {
      $$ = $1;
    }
;

factor
  : variable_reference
    {
      $$ = $1;
    }
  | SUB_OP factor
    {
      $$ = checkSubOp($2);
    }
  | L_PAREN logical_expression R_PAREN
    {
      $$ = $2;
    }
  | ID L_PAREN logical_expression_list R_PAREN
    {
      struct SymTableNode* node = findID(symbolTableList, $1);
      if (checkFunction(node,true) && checkFuncCall(node, $3))
        $$ = createExpr(node);
      else
        $$ = NULL;
      deleteExprList($3);
      free($1);
    }
  | ID L_PAREN R_PAREN
    {
      struct SymTableNode* node = findID(symbolTableList, $1);
      if (checkFunction(node,true))
        $$ = createExpr(node);
      else
        $$ = NULL;
      free($1);
    }
  | literal_const
    {
      $$ = createConstExpr($1);
      killAttribute($1);
    }
;

logical_expression_list
  : logical_expression_list COMMA logical_expression
    {
      connectExprList($1,$3);
      $$ = $1;
    }
  | logical_expression
    {
      $$ = createExprList($1);
    }
;

array_list
  : ID dimension
    {
      struct SymTableNode* node = findID(symbolTableList,$1);
      if (checkFunction(node, false))
        $$ = createArrayRefExpr(node, $2);
      else
        $$ = NULL;
      free($1);
    }
;

dimension
  : dimension ML_BRACE logical_expression MR_BRACE
    {
      connectArrayDimNode($1,createArrayDimNode(0));
      $$ = $1;
    }
  | ML_BRACE logical_expression MR_BRACE
    {
      $$ = createArrayDimNode(0);
    }
;

scalar_type
  : INT
    {
      $$ = INT_t;
      baseType = $$;
    }
  | DOUBLE
    {
      $$ = DOUBLE_t;
      baseType = $$;
    }
  | STRING
    {
      $$ = STRING_t;
      baseType = $$;
    }
  | BOOL
    {
      $$ = BOOL_t;
      baseType = $$;
    }
  | FLOAT
    {
      $$ = FLOAT_t;
      baseType = $$;
    }
;

literal_const
  : INT_CONST
    {
      int val = $1;
      $$ = createConstantAttribute(INT_t,&val);
    }
  | SUB_OP INT_CONST
    {
      int val = -$2;
      $$ = createConstantAttribute(INT_t,&val);
    }
  | FLOAT_CONST
    {
      float val = $1;
      $$ = createConstantAttribute(FLOAT_t,&val);
    }
  | SUB_OP FLOAT_CONST
    {
      float val = -$2;
      $$ = createConstantAttribute(FLOAT_t,&val);
    }
  | SCIENTIFIC
    {
      double val = $1;
      $$ = createConstantAttribute(DOUBLE_t,&val);
    }
  | SUB_OP SCIENTIFIC
    {
      double val = -$2;
      $$ = createConstantAttribute(DOUBLE_t,&val);
    }
  | STR_CONST
    {
      $$ = createConstantAttribute(STRING_t,$1);
      free($1);
    }
  | TRUE
    {
      bool val = true;
      $$ = createConstantAttribute(BOOL_t,&val);
    }
  | FALSE
    {
      bool val = false;
      $$ = createConstantAttribute(BOOL_t,&val);
    }
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
  //  fprintf( stderr, "%s\t%d\t%s\t%s\n", "Error found in Line ", linenum, "next token: ", yytext );
}
