#include "semcheck.h"

struct Expr* createExpr(struct SymTableNode* node)
{
  if (node == NULL) return NULL;
  struct Expr* expr = (struct Expr*)malloc(sizeof(struct Expr));
  expr->kind = node->kind;
  expr->type = node->type;
  expr->type->reference += 1;
  expr->next = NULL;
  return expr;
}

struct Expr* createConstExpr(struct Attribute* attr)
{
  if (attr == NULL) return NULL;
  struct Expr* expr = (struct Expr*)malloc(sizeof(struct Expr));
  expr->type = createExtType(attr->constVal->type,false,NULL);
  expr->kind = CONSTANT_t;
  expr->next = NULL;
  return expr;
}

struct Expr* createVariableExpr(struct Variable* node)
{
  if (node == NULL) return NULL;
  struct Expr* expr = (struct Expr*)malloc(sizeof(struct Expr));
  expr->type = node->type;
  expr->type->reference += 1;
  expr->kind = VARIABLE_t;
  expr->next = NULL;
  return expr;
}

struct Expr* createArrayRefExpr(struct SymTableNode* node, struct ArrayDimNode* dim)
{
  if (node == NULL) return NULL;
  struct Expr* expr = (struct Expr*)malloc(sizeof(struct Expr));
  expr->kind = node->kind;
  expr->next = NULL;
  if (node->kind == FUNCTION_t)
  {
    expr->type = NULL;
    // printError("function '%s' cannot be an operand", node->name);
  }
  else
  {
    expr->type = createExtType(node->type->baseType,false,NULL);
    if (!node->type->isArray)
    {
      printError("'%s' not array", node->name);
    }
    else if (expr->type != NULL)
    {
      int arrayCheck = checkArrayDim(dim, node->type->dimArray);
      switch (arrayCheck)
      {
        case -1:
          printError("too many subscripts on array '%s'", node->name);
          deleteExpr(expr);
          expr = NULL;
          break;
        case 0:
          expr->type->isArray = false;
          break;
        default:
          expr->type->isArray = true;
          expr->type->dimArray = node->type->dimArray;
          while (arrayCheck != 0) {
            expr->type->dimArray = expr->type->dimArray->next;
            arrayCheck -= 1;
          }
          expr->type->dimArray->reference += 1;
      }
    }
  }
  return expr;
}

struct Expr* deleteExpr(struct Expr* target)
{
  struct Expr* next;
  if (target == NULL) return NULL;
  target->reference -= 1;
  if (target->reference > 0) return NULL;
  next = target->next;
  killExtType(target->type);
  free(target);
  return next;
}

struct ExprList* createExprList(struct Expr* head)
{
  struct ExprList *list;
  if (head == NULL)
  {
    head = (struct Expr*)malloc(sizeof(struct Expr));
    head->type = createExtType(VOID_t,false,NULL);
    head->type->isArray = false;
    head->type->dimArray = NULL;
    head->reference = 0;
    head->next = NULL;
  }
  list = (struct ExprList*)malloc(sizeof(struct ExprList));
  struct Expr* temp = head;
  while (temp->next != NULL)
  {
    temp = temp->next;
  }
  /**/
  list->head = head;
  head->reference += 1;
  /**/
  list->tail = temp;
  if (head!=temp)
    temp->reference += 1;
  /**/
  list->reference = 0;
  return list;
}

int deleteExprList(struct ExprList* list)
{
  if (list == NULL) return -1;
  list->reference -= 1;
  if (list->reference>0)
    return -1;
  if (list->head != NULL)
  {
    /**/
    //list->head = NULL
    list->head->reference -= 1;
    /**/
    if (list->head!=list->tail)
    {
      //list->tail = NULL
      list->tail->reference -= 1;
    }
    /**/
    while (list->head != NULL)
    {
      list->head=deleteExpr(list->head);
    }
  }
  return 0;
}

int connectExprList(struct ExprList* list, struct Expr* node)
{
  if (list == NULL)
    return -1;
  if (node == NULL)
  {
    node = (struct Expr*)malloc(sizeof(struct Expr));
    node->type = createExtType(VOID_t,false,NULL);
    node->type->isArray = false;
    node->type->dimArray = NULL;
    node->reference = 0;
    node->next = NULL;
  }
  if (node->next != NULL)
    return -2;
  if (list->tail != list->head)
    list->tail->reference -= 1;
  /**/
  list->tail->next = node;
  list->tail->next->reference += 1;
  list->tail = node;
  list->tail->reference += 1;
  /**/
  return 0;
}

struct SymTableNode* findID(struct SymTableList* list, const char* name)
{
  if (list == NULL) return NULL;
  // search the list from the local scope to the global scope (tail to head)
  struct SymTable *table = list->tail;
  while (table != NULL)
  {
    struct SymTableNode *matched = findTableNode(table, name);
    if (matched != NULL) return matched;
    table = table->prev;
  }
  printError("undefined symbol '%s'", name);
  return NULL;
}

void checkArrayVariable(struct Variable* node)
{
  if (node == NULL) return;
  struct ArrayDimNode* a = node->type->dimArray;
  while (a != NULL)
  {
    if (a->size <= 0) {
      printError("non-positive size of array '%s'", node->name);
      return;
    }
    a = a->next;
  }
}

void checkInitArray(struct Variable* variable, struct InitArray* list)
{
  if (variable == NULL || list == NULL) return;
  if (variable->type->dim != 1) {
    printError("1-d list assigned to %d-d array '%s'", variable->type->dim,
               variable->name);
    return;
  }
  if (variable->type->dimArray->size < list->size) {
    printError("length of initial list(%d) exceeds size of '%s'(%d)",
               list->size, variable->name, variable->type->dimArray->size);
  }
}

bool checkFunction(struct SymTableNode* node, bool func)
{
  if (node == NULL) return false;
  if (func && node->kind == FUNCTION_t) return true;
  if (!func && node->kind != FUNCTION_t) return true;
  if (func)
    printError("'%s' is not a function", node->name);
  else
    printError("function '%s' cannot be an operand", node->name);
  return false;
}

void checkInitArrayType(struct Expr* expr, int n)
{
  if (expr == NULL || expr->type == NULL) return;
  struct ExtType* tempType = createExtType(baseType,false,NULL);
  if (!canConvertTypeImplicitly(expr->type,tempType,false) && expr->type->baseType != VOID_t) {
    char* sourceString = typeString(expr->type);
    char* targetString = typeString(tempType);
    printError("element %d of initial list is of type '%s', expected '%s'", n, sourceString, targetString);
    free(sourceString);
    free(targetString);
  }
  killExtType(tempType);
}

bool checkArrayOp(struct Expr* lhs, struct Expr *rhs)
{
  if (lhs == NULL || rhs == NULL || lhs->type == NULL || rhs->type == NULL)
    return false;
  if (lhs->type->isArray || rhs->type->isArray) return false;
  return true;
}

void checkFuncAttr(struct SymTableNode* node, struct FuncAttrNode* list, BTYPE returnType)
{
  struct FuncAttrNode* func = NULL;
  if (node == NULL) return;
  if (node->attr != NULL && node->attr->funcParam)
    func = node->attr->funcParam->head;
  while (func != NULL && list != NULL)
  {
    if (func->value->baseType != list->value->baseType) break;
    if (func->value->isArray != list->value->isArray) break;
    if (func->value->isArray && !checkArraySize(func->value->dimArray,list->value->dimArray)) break;
    func = func->next;
    list = list->next;
  }
  if (func != NULL || list != NULL || node->type->baseType != returnType)
    printError("definition of function '%s' does not match its declaration", node->name);
}

bool checkFuncCall(struct SymTableNode* node, struct ExprList* list)
{
  int argc = 0;
  char* sourceString;
  char* targetString;
  struct FuncAttrNode* func = NULL;
  struct Expr* expr = NULL;
  if (node == NULL) return false;
  if (node->attr != NULL && node->attr->funcParam)
    func = node->attr->funcParam->head;
  if (list != NULL && list->head != NULL)
    expr = list->head;
  while (func != NULL && expr != NULL)
  {
    argc += 1;
    sourceString = typeString(expr->type);
    targetString = typeString(func->value);
    if (!canConvertTypeImplicitly(expr->type, func->value, true) && expr->type->baseType != VOID_t)
      printError("type of argument %d of '%s' is '%s', expected '%s'", argc, node->name, sourceString, targetString);
    func = func->next;
    expr = expr->next;
    free(sourceString);
    free(targetString);
  }
  if (func != NULL)
    printError("too few arguments to function '%s'", node->name);
  if (expr != NULL)
    printError("too many arguments to function '%s'", node->name);
  return (func == NULL && expr == NULL);
}

void checkAssignOp(struct Expr* lhs, struct Expr* rhs)
{
  if (lhs == NULL || rhs == NULL) return;
  if (lhs->type == NULL || rhs->type == NULL) return;
  if (lhs->kind == CONSTANT_t)
  {
    printError("cannot assign to constant");
    return;
  }
  char* lhsString = typeString(lhs->type);
  char* rhsString = typeString(rhs->type);
  if (!checkArrayOp(lhs,rhs) || !canConvertTypeImplicitly(rhs->type, lhs->type, false))
    printError("invalid operands to assignment ('%s' = '%s')", lhsString, rhsString);
  free(lhsString);
  free(rhsString);
}

struct Expr* checkModOp(struct Expr* lhs, struct Expr* rhs)
{
  if (lhs == NULL || rhs == NULL) return NULL;
  if (lhs->type == NULL || rhs->type == NULL) return NULL;
  if (checkArrayOp(lhs, rhs) && lhs->type->baseType == INT_t && rhs->type->baseType == INT_t)
  {
    struct Expr* expr = (struct Expr*)malloc(sizeof(struct Expr));
    expr->type = createExtType(INT_t,false,NULL);
    expr->next = NULL;
    return expr;
  }
  char* lhsString = typeString(lhs->type);
  char* rhsString = typeString(rhs->type);
  printError("invalid operands to binary expression ('%s' %% '%s')", lhsString, rhsString);
  free(lhsString);
  free(rhsString);
  return NULL;
}

struct Expr* checkLogicalOp(struct Expr* lhs, struct Expr* rhs, const char* op)
{
  if (lhs == NULL || rhs == NULL) return NULL;
  if (lhs->type == NULL || rhs->type == NULL) return NULL;
  if (checkArrayOp(lhs, rhs) && lhs->type->baseType == BOOL_t && rhs->type->baseType == BOOL_t)
  {
    struct Expr* expr = (struct Expr*)malloc(sizeof(struct Expr));
    expr->type = createExtType(BOOL_t,false,NULL);
    expr->next = NULL;
    return expr;
  }
  char* lhsString = typeString(lhs->type);
  char* rhsString = typeString(rhs->type);
  printError("invalid operands to binary expression ('%s' %s '%s')", lhsString, op, rhsString);
  free(lhsString);
  free(rhsString);
  return NULL;
}

struct Expr* checkNotOp(struct Expr* expr)
{
  if (expr == NULL && expr->type == NULL) return NULL;
  if (!expr->type->isArray && expr->type->baseType == BOOL_t)
  {
    struct Expr* expr = (struct Expr*)malloc(sizeof(struct Expr));
    expr->type = createExtType(BOOL_t,false,NULL);
    expr->next = NULL;
    return expr;
  }
  char* exprString = typeString(expr->type);
  printError("invalid operand to unary expression (!'%s')", exprString);
  free(exprString);
  return NULL;
}

struct Expr* checkSubOp(struct Expr* expr)
{
  if (expr == NULL) return NULL;
  if (expr->type == NULL) {
    deleteExpr(expr);
    return NULL;
  }
  if (!expr->type->isArray &&
    (expr->type->baseType == INT_t || expr->type->baseType == FLOAT_t || expr->type->baseType == DOUBLE_t))
    return expr;
  char* exprString = typeString(expr->type);
  printError("invalid operand to unary expression (-'%s')", exprString);
  free(exprString);
  deleteExpr(expr);
  return NULL;
}

struct Expr* checkArithOp(struct Expr* lhs, struct Expr* rhs, const char* op)
{
  struct Expr* expr = NULL;
  if (lhs == NULL || rhs == NULL) return NULL;
  if (lhs->type == NULL || rhs->type == NULL) return NULL;
  if (checkArrayOp(lhs, rhs) &&
      (lhs->type->baseType == INT_t || lhs->type->baseType == FLOAT_t || lhs->type->baseType == DOUBLE_t) &&
      (rhs->type->baseType == INT_t || rhs->type->baseType == FLOAT_t || rhs->type->baseType == DOUBLE_t))
  {
    if (canConvertTypeImplicitly(rhs->type, lhs->type, false)) {
      expr = (struct Expr*)malloc(sizeof(struct Expr));
      expr->type = createExtType(lhs->type->baseType,false,NULL);
      expr->next = NULL;
      return expr;
    } else if (canConvertTypeImplicitly(lhs->type, rhs->type, false)) {
      expr = (struct Expr*)malloc(sizeof(struct Expr));
      expr->type = createExtType(rhs->type->baseType,false,NULL);
      expr->next = NULL;
      return expr;
    }
  }
  char* lhsString = typeString(lhs->type);
  char* rhsString = typeString(rhs->type);
  printError("invalid operands to binary expression ('%s' %s '%s')", lhsString, op, rhsString);
  free(lhsString);
  free(rhsString);
  return expr;
}


struct Expr* checkRelOp(struct Expr* lhs, struct Expr* rhs, const char* op)
{
  struct Expr* expr = NULL;
  if (lhs == NULL || rhs == NULL) return NULL;
  if (lhs->type == NULL || rhs->type == NULL) return NULL;
  if (checkArrayOp(lhs, rhs) &&
      (lhs->type->baseType == INT_t || lhs->type->baseType == FLOAT_t || lhs->type->baseType == DOUBLE_t) &&
      (rhs->type->baseType == INT_t || rhs->type->baseType == FLOAT_t || rhs->type->baseType == DOUBLE_t))
  {
    expr = (struct Expr*)malloc(sizeof(struct Expr));
    expr->type = createExtType(BOOL_t,false,NULL);
    expr->next = NULL;
    return expr;
  }
  char* lhsString = typeString(lhs->type);
  char* rhsString = typeString(rhs->type);
  printError("invalid operands to binary expression ('%s' %s '%s')", lhsString, op, rhsString);
  free(lhsString);
  free(rhsString);
  return expr;
}

struct Expr* checkCompOp(struct Expr* lhs, struct Expr* rhs, const char* op)
{
  struct Expr* expr = NULL;
  if (lhs == NULL || rhs == NULL) return NULL;
  if (lhs->type == NULL || rhs->type == NULL) return NULL;
  if (checkArrayOp(lhs, rhs))
  {
    if ((lhs->type->baseType == INT_t || lhs->type->baseType == FLOAT_t || lhs->type->baseType == DOUBLE_t) &&
        (rhs->type->baseType == INT_t || rhs->type->baseType == FLOAT_t || rhs->type->baseType == DOUBLE_t))
    {
      expr = (struct Expr*)malloc(sizeof(struct Expr));
      expr->type = createExtType(BOOL_t,false,NULL);
      expr->next = NULL;
      return expr;
    }
    if (lhs->type->baseType == BOOL_t && rhs->type->baseType == BOOL_t)
    {
      expr = (struct Expr*)malloc(sizeof(struct Expr));
      expr->type = createExtType(BOOL_t,false,NULL);
      expr->next = NULL;
      return expr;
    }
  }
  char* lhsString = typeString(lhs->type);
  char* rhsString = typeString(rhs->type);
  printError("invalid operands to binary expression ('%s' %s '%s')", lhsString, op, rhsString);
  free(lhsString);
  free(rhsString);
  return expr;
}

void checkPrintRead(struct Expr* expr)
{
  if (expr == NULL || expr->type == NULL) return;
  if (expr->type->isArray || expr->type->baseType == VOID_t)
    printError("non-scalar operand of print/read statement");
}

void checkCond(struct Expr* expr, const char* stmt)
{
  if (expr == NULL || expr->type == NULL) return;
  if (expr->type != NULL && !expr->type->isArray && expr->type->baseType == BOOL_t)
    return;
  printError("conditional expression of '%s' must be of type boolean", stmt);
}

void checkReturn(struct Expr* expr, struct ExtType* type)
{
  if (expr == NULL || expr->type == NULL || type == NULL) return;
  if (type->baseType == VOID_t) {
    printError("return in void function");
    return;
  }
  char* exprString = typeString(expr->type);
  char* expectedString = typeString(type);
  if (!canConvertTypeImplicitly(expr->type,type,false))
    printError("return '%s', expected '%s'", exprString, expectedString);
  free(exprString);
  free(expectedString);
}
