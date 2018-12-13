#include "semcheck.h"

struct SymTableNode* findID(struct SymTableList* list, const char* name)
{
  // search the list from the local scope to the global scope (tail to head)
  struct SymTable *table = list->tail;
  while (table != NULL)
  {
    struct SymTableNode *matched = findTableNode(table, name);
    if (matched != NULL) return matched;
    table = table->prev;
  }
  printError("symbol '%s' is not defined.", name);
  return NULL;
}

void checkArrayVariable(struct Variable* node)
{
  struct ArrayDimNode* a = node->type->dimArray;
  while (a != NULL)
  {
    if (a->size <= 0) {
      printError("the size of '%s' is not positive.", node->name);
      return;
    }
    a = a->next;
  }
}

void checkInitArray(struct Variable* variable, struct InitArray* list)
{
  if (variable->type->dim != 1) {
    printError("assign 1-d list to %d-d array'%s'.", variable->type->dim, variable->name);
    return;
  }
  if (variable->type->dimArray->size < list->size) {
    printError("the length of the initialization array(%d) "
               "exceeds the size of '%s'(%d)",
               list->size, variable->name, variable->type->dimArray->size);
  }
}

bool checkArrayDim(struct ArrayDimNode* a1, struct ArrayDimNode* a2)
{
  while (a1 != NULL && a2 != NULL)
  {
    if (a1->size != a2->size) break;
    a1 = a1->next;
    a2 = a2->next;
  }
  return (a1 == NULL && a2 == NULL);
}
