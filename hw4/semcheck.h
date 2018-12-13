#ifndef __SEMCHECK__
#define __SEMCHECK__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "symtable.h"
#include "error.h"

// assisting functions
struct SymTableNode* findID(struct SymTableList* list, const char* name);

// checking functions
void checkArrayVariable(struct Variable* node);
void checkInitArray(struct Variable* variable, struct InitArray* list);
bool checkArrayDim(struct ArrayDimNode* a1, struct ArrayDimNode* a2);

#endif
