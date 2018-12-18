#ifndef __SEMCHECK__
#define __SEMCHECK__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "symtable.h"
#include "error.h"

extern int linenum;

typedef enum { JUMP_t } STYPE;

struct Expr {
  int            reference;
  KIND           kind;
  struct ExtType *type;
  struct Expr    *next;
};

struct ExprList {
  int         reference;
  struct Expr *head;
  struct Expr *tail;
};

struct StmtAttr {
  bool lastReturn;
};

// Expr/ExprList operations
struct Expr* createExpr(struct SymTableNode* node);
struct Expr* createConstExpr(struct Attribute* attr);
struct Expr* createVariableExpr(struct Variable* node);
struct Expr* createArrayRefExpr(struct SymTableNode* node, struct ArrayDimNode* dim);
struct Expr* deleteExpr(struct Expr* target);
struct ExprList* createExprList(struct Expr* head);
int deleteExprList(struct ExprList* list);
int connectExprList(struct ExprList* list, struct Expr* node);

// assisting functions
struct SymTableNode* findID(struct SymTableList* list, const char* name);

// array checking functions
void checkArrayVariable(struct Variable* node);
void checkInitArray(struct Variable* variable, struct InitArray* list);

// avoid the misuse of ID (function/other)
bool checkFunction(struct SymTableNode* node, bool func);

// type checking
bool checkArrayOp(struct Expr* lhs, struct Expr *rhs);
void checkFuncAttr(struct SymTableNode* node, struct FuncAttrNode* list, BTYPE returnType);
bool checkFuncCall(struct SymTableNode* node, struct ExprList* list);
void checkAssignOp(struct Expr* lhs, struct Expr* rhs);
struct Expr* checkModOp(struct Expr* lhs, struct Expr* rhs);
struct Expr* checkLogicalOp(struct Expr* lhs, struct Expr* rhs, const char* op);
struct Expr* checkNotOp(struct Expr* expr);
struct Expr* checkSubOp(struct Expr* expr);
struct Expr* checkArithOp(struct Expr* lhs, struct Expr* rhs, const char* op);
struct Expr* checkRelOp(struct Expr* lhs, struct Expr* rhs, const char* op);
struct Expr* checkCompOp(struct Expr* lhs, struct Expr* rhs, const char* op);

void checkPrintRead(struct Expr* expr);
void checkCond(struct Expr* expr, const char* stmt);
void checkReturn(struct Expr* expr, struct ExtType* type);

#endif
