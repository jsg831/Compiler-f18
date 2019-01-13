#ifndef _CODEGEN_H_
#define _CODEGEN_H_
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "semcheck.h"
#include "header.h"
#include "symtab.h"
#include "codebuf.h"

struct JumpLabel {
  int continueLabel;
  int breakLabel;
};

void initCodeGen(const char* filename);
void terminateCodeGen();

char javaFuncChar(SEMTYPE type);
char javaTypeChar(SEMTYPE type);
void printFuncName(struct SymNode* node);

void genProgStart();
void genMainStart();
void genFuncStart(struct SymNode* node);
void genFuncEnd(bool isVoid);

void genFieldVar(struct SymNode* node);

void genLitLoad(struct ConstAttr* attr);
void genVarLoad(struct SymNode* node);
void genConstLoad(struct SymNode* node);

void genTypeCoersion(SEMTYPE source_type, SEMTYPE target_type);
void genVarDeclStore(SEMTYPE type, const char* id, int scope, int localNumber);
void genVarStore(struct SymNode* node);

void genFuncCall(struct SymNode* node);
void dumpReturn(SEMTYPE type);

void genUnaryOp(SEMTYPE type);
void genBinaryOp(SEMTYPE lhsType, OPERATOR op, SEMTYPE rhsType);

void genPrintStart();
void genPrintEnd(SEMTYPE type);
void genRead(struct SymNode* node);

void genReturn(SEMTYPE returnType, SEMTYPE exprType);

int genIf();
int genElse(int label);
void genIfExit(int label);

int genWhile();
void genWhileBody(int label);
void genWhileExit(int label);

int genDo();
void genDoCond(int label);
void genDoExit(int label);

int genFor();
void genForIncr(int label);
void genForBody(int label);
void genForExit(int label);

void pushJumpLabel(int continueLabel, int breakLabel);
void popJumpLabel();
void genContinue();
void genBreak();

#endif
