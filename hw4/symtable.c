#include "symtable.h"

int initSymTableList(struct SymTableList *list)
{
  list->head = NULL;
  list->tail = NULL;
  list->global = NULL;
  list->reference = 1;
  return 0;
}

int destroySymTableList(struct SymTableList *list)
{
  list->reference -= 1;//derefence
  if (list->reference > 0) return -1;
  while (list->head != NULL)
  {
    //kill head node
    list->head = deleteSymTable(list->head);//return new head
  }
  return 0;
}

struct SymTable* deleteSymTable(struct SymTable* target)
{
  struct SymTable *next;
  if (target == NULL)
    next = NULL;
  else
  {
    target->reference -= 1;//dereference
    if (target->reference > 0)
      return NULL;
    next = target->next;
    while (target->head != NULL)
    {
      target->head = deleteTableNode(target->head);
    }
  }
  if (next != NULL) next->prev = NULL;
  return next;
}

int AddSymTable(struct SymTableList* list)//enter a new scope
{
  if (list->head == NULL)
  {
    struct SymTable *newTable = (struct SymTable*)malloc(sizeof(struct SymTable));
    newTable->head = NULL;
    newTable->tail = NULL;
    newTable->next = NULL;
    newTable->prev = NULL;
    list->head = newTable;
    list->tail = list->head;
    list->global = list->head;
    newTable->reference = 1;
  }
  else
  {
    struct SymTable *newTable = (struct SymTable*)malloc(sizeof(struct SymTable));
    newTable->head = NULL;
    newTable->tail = NULL;
    newTable->next = NULL;
    newTable->prev = list->tail;
    list->tail->next = newTable;
    list->tail = newTable;
    newTable->reference = 1;
  }
  return 0;
}

int deleteLastSymTable(struct SymTableList* list)//leave scope
{
  struct SymTable *temp = list->tail;
  if (temp == NULL)
    return -1;
  temp->reference -= 1;//derefence
  if (temp->reference > 0)
    return -1;
  if (list->head != list->tail)
  {
    temp->prev->next = NULL;
    list->tail = temp->prev;
  }
  else
  {
    list->tail = NULL;
    list->head = NULL;
  }
  deleteSymTable(temp);
  return 0;
}

int insertTableNode(struct SymTable *table,struct SymTableNode* newNode)
{
  if (table == NULL || newNode == NULL) return -1;
  // Check if the symbol has been defined before
  struct SymTableNode* node = findTableNode(table, newNode->name);
  if (node != NULL)
  {
    printError("symbol '%s' is redeclared.", newNode->name);
    deleteTableNode(newNode);
    return -1;
  }
  if (newNode->kind != FUNCTION_t) {
    node = findTableNode(symbolTableList->global, newNode->name);
    if (node != NULL && node->kind == FUNCTION_t) {
      printError("symbol '%s' is declared as a function.", newNode->name);
      deleteTableNode(newNode);
      return -1;
    }
  }
  if (table->tail == NULL)
  {
    table->head = newNode;
    table->tail = newNode;
  }
  else
  {
    table->tail->next = newNode;
    table->tail = newNode;
  }
  newNode->reference += 1;
  return 0;
}

struct SymTableNode* findTableNode(struct SymTable* table,const char* name)
{
  struct SymTableNode *node = table->head;
  while (node != NULL)
  {
    if (strcmp(node->name, name) == 0) return node;
    node = node->next;
  }
  return NULL;
}

struct SymTableNode* deleteTableNode(struct SymTableNode* target)//return next node
{
  struct SymTableNode *next;
  if (target == NULL)
    next = NULL;
  else
  {
    target->reference -= 1; //defreference
    if (target->reference > 0)
      return NULL;
    next = target->next;
    killExtType(target->type);
    if (target->attr != NULL)
      killAttribute(target->attr);
    free(target);
  }
  return next;
}

struct SymTableNode* createVariableNode(const char* name,int level,struct ExtType* type)
{
  struct SymTableNode *newNode = (struct SymTableNode*)malloc(sizeof(struct SymTableNode));
  //set node
  strncpy(newNode->name,name,32);
  newNode->kind = VARIABLE_t;
  newNode->level = level;
  /**/
  newNode->type = type;
  newNode->type->reference += 1;
  /**/
  newNode->attr = NULL;
  newNode->next = NULL;
  newNode->reference = 0;
  return newNode;
}

struct SymTableNode* createFunctionNode(const char* name,int level,struct ExtType* type,struct Attribute* attr,bool decl)
{
  struct SymTableNode *newNode = (struct SymTableNode*)malloc(sizeof(struct SymTableNode));
  //set node
  strncpy(newNode->name,name,32);
  newNode->kind = FUNCTION_t;
  newNode->level = level;
  newNode->decl = decl;
  /**/
  newNode->type = type;
  newNode->type->reference += 1;
  /**/
  newNode->attr = attr;
  if (attr != NULL)
    newNode->attr->reference += 1;
  /**/
  newNode->next = NULL;
  newNode->reference = 0;
  return newNode;

}

struct SymTableNode* createConstNode(const char* name,int level,struct ExtType* type,struct Attribute* attr)
{
  struct SymTableNode *newNode = (struct SymTableNode*)malloc(sizeof(struct SymTableNode));
  //set node
  strncpy(newNode->name,name,32);
  newNode->kind = CONSTANT_t;
  newNode->level = level;
  struct ExtType* tempType = createExtType(attr->constVal->type,false,NULL);
  if (!canConvertTypeImplicitly(tempType,type,false)) {
    char* str = typeString(tempType);
    printError("constant %s should be of type '%s'", name, str);
    type->baseType = attr->constVal->type;
    free(str);
  }
  killExtType(tempType);
  /**/
  newNode->type = type;
  newNode->type->reference += 1;
  /**/
  newNode->attr = attr;
  if (attr != NULL)
    newNode->attr->reference += 1;
  /**/
  newNode->next = NULL;
  newNode->reference = 0;
  return newNode;
}
struct SymTableNode* createParameterNode(const char* name,int level,struct ExtType* type)
{
  struct SymTableNode *newNode = (struct SymTableNode*)malloc(sizeof(struct SymTableNode));
  //set node
  strncpy(newNode->name,name,32);
  newNode->kind = PARAMETER_t;
  newNode->level = level;
  /**/
  newNode->type = type;
  newNode->type->reference+=1;
  /**/
  newNode->attr = NULL;
  newNode->next = NULL;
  newNode->reference = 0;
  return newNode;
}

struct Attribute* createFunctionAttribute(struct FuncAttrNode* list)
{
  int num = 0;
  struct Attribute *newAttr = (struct Attribute*)malloc(sizeof(struct Attribute));
  newAttr->constVal = NULL;
  newAttr->funcParam = (struct FuncAttr*)malloc(sizeof(struct FuncAttr));
  newAttr->funcParam->reference = 1;
  /**/
  newAttr->funcParam->head = list;
  newAttr->funcParam->head->reference += 1;
  /**/
  while (list != NULL)
  {
    list = list->next;
    ++num;
  }
  newAttr->funcParam->paramNum = num;
  newAttr->reference = 0;
  return newAttr;
}

struct Attribute* createConstantAttribute(BTYPE type,void* value)
{
  struct Attribute *newAttr = (struct Attribute*)malloc(sizeof(struct Attribute));
  struct ConstAttr *newConstAttr = (struct ConstAttr*)malloc(sizeof(struct ConstAttr));
  newAttr->funcParam = NULL;
  newAttr->constVal = newConstAttr;
  newConstAttr->reference = 1;
  newConstAttr->hasMinus = false;
  newConstAttr->type = type;
  switch (type)
  {
    case INT_t:
      newConstAttr->value.integerVal = *(int*)value;
      if (*(int*)value < 0)
        newConstAttr->hasMinus = true;
      break;
    case FLOAT_t:
      newConstAttr->value.floatVal = *(float*)value;
      if (*(float*)value < 0.0)
        newConstAttr->hasMinus = true;
      break;
    case DOUBLE_t:
      newConstAttr->value.doubleVal = *(double*)value;
      if (*(double*)value < 0.0)
        newConstAttr->hasMinus = true;
      break;
    case BOOL_t:
      newConstAttr->value.boolVal = *(bool*)value;
      break;
    case STRING_t:
      newConstAttr->value.stringVal = strdup((char*)value);
      break;
    default:
      break;
  }
  newAttr->reference = 0;
  return newAttr;
}

struct FuncAttrNode* deleteFuncAttrNode(struct FuncAttrNode* target)
{
  struct FuncAttrNode *next;
  if (target == NULL)
    next=NULL;
  else
  {
    target->reference -= 1;
    if (target->reference>0)
      return NULL;
    next = target->next;
    killExtType(target->value);
    free(target->name);
    free(target);
  }
  return next;
}

int killAttribute(struct Attribute* target)
{
  target->reference -= 1;
  if (target->reference>0)
    return -1;
  if (target->constVal != NULL)
  {
    target->constVal->reference -= 1;
    if (target->constVal->reference>0)
      return -1;
    if (target->constVal->type==STRING_t)
      free(target->constVal->value.stringVal);
    free(target->constVal);
  }
  if (target->funcParam != NULL)
  {
    target->funcParam->reference -= 1;
    if (target->funcParam->reference>0)
      return -1;
    target->funcParam->head = deleteFuncAttrNode(target->funcParam->head);
    free(target->funcParam);
  }
  free(target);
  return 0;
}

struct FuncAttrNode* createFuncAttrNode(struct ExtType* type,const char* name)
{
  struct FuncAttrNode *newNode = (struct FuncAttrNode*)malloc(sizeof(struct FuncAttrNode));
  /*reference*/
  newNode->value = type;
  type->reference += 1;
  /*         */
  newNode->name = strdup(name);
  newNode->next = NULL;
  newNode->reference = 0;
  return newNode;
}

int connectFuncAttrNode(struct FuncAttrNode* head, struct FuncAttrNode* newNode)//connect node to tail of function attribute list
{
  if (head == NULL || newNode == NULL || head==newNode)
    return -1;
  struct FuncAttrNode *temp = head;
  while (temp->next != NULL)
  {
    temp = temp->next;
  }
  temp->next = newNode;
  newNode->reference += 1;
  return 0;
}

struct ExtType* createExtType(BTYPE baseType,bool isArray,struct ArrayDimNode* dimArray)
{
  int dimNum = 0;
  struct ArrayDimNode *temp = dimArray;
  struct ExtType *newExtType = (struct ExtType*)malloc(sizeof(struct ExtType));
  newExtType->baseType = baseType;
  newExtType->isArray = isArray;
  /**/
  newExtType->dimArray = dimArray;
  if (dimArray != NULL)
    dimArray->reference += 1;
  /**/
  newExtType->reference = 0;
  while (temp != NULL)
  {
    temp = temp->next;
    ++dimNum;
  }
  newExtType->dim = dimNum;
  return newExtType;
}

int killExtType(struct ExtType* target)
{
  if (target == NULL)
    return -1;
  target->reference -= 1;
  if (target->reference>0)
    return -1;
  if (target->isArray)
  {
    while (target->dimArray != NULL)
    {
      target->dimArray = deleteArrayDimNode(target->dimArray);
    }
  }
  return 0;
}

struct ArrayDimNode* createArrayDimNode(int size)
{
  struct ArrayDimNode *newNode = (struct ArrayDimNode*)malloc(sizeof(struct ArrayDimNode));
  newNode->size = size;
  newNode->next = NULL;
  newNode->reference = 0;
  return newNode;
}

int connectArrayDimNode(struct ArrayDimNode* head,struct ArrayDimNode* newNode)//connect dimension node to tail of list
{
  if (head == NULL || newNode == NULL || head==newNode)
    return -1;
  struct ArrayDimNode *temp = head;
  while (temp->next != NULL)
  {
    temp = temp->next;
  }
  /**/
  temp->next = newNode;
  newNode->reference += 1;
  /**/
  return 0;
}

struct ArrayDimNode* deleteArrayDimNode(struct ArrayDimNode* target)
{
  struct ArrayDimNode *next;
  if (target == NULL)
    next = NULL;
  else
  {
    target->reference -= 1;
    if (target->reference>0)
      return NULL;
    next = target->next;
    free(target);
  }
  return next;
}

struct SymTableNode* findFuncDeclaration(struct SymTable* table,const char* name)
{
  struct SymTableNode *temp = table->head;
  while (temp != NULL)
  {
    if (temp->kind == FUNCTION_t)
    {
      if (strncmp(temp->name,name,32) == 0)
        return temp;
    }
    temp = temp->next;
  }
  return NULL;
}

int printSymTable(struct SymTable* table)
{
  struct SymTableNode *entry;
  struct ArrayDimNode *dimNode;
  struct Attribute *attr;
  struct FuncAttrNode *funcAttrNode;
  char strBuffer[32];
  if (table == NULL) return -1;
  if (table->head == NULL) return 1;//no entry to output
  printf("=======================================================================================\n");
  // Name [29 blanks] Kind [7 blanks] Level [7 blank] Type [15 blanks] Attribute [15 blanks]
  printf("Name                             Kind       Level       Type               Attribute               \n");
  printf("---------------------------------------------------------------------------------------\n");
  entry = table->head;
  while (entry != NULL)
  {
    //name
    printf("%-32s ",entry->name);
    //kind
    switch (entry->kind)
    {
      case VARIABLE_t:
        printf("%-11s","variable");
        break;
      case CONSTANT_t:
        printf("%-11s","constant");
        break;
      case FUNCTION_t:
        printf("%-11s","function");
        break;
      case PARAMETER_t:
        printf("%-11s","parameter");
        break;
      default:
        printf("%-11s","unknown");
        break;
    }
    //level
    if (entry->level==0)
      printf("%-12s","0(global) ");
    else
    {
      sprintf(strBuffer,"%d(local)  ",entry->level);
      printf("%-12s",strBuffer);
    }
    //type
    printType(entry->type);
    //attribute
    attr = entry->attr;
    if (attr != NULL)
    {
      if (attr->constVal != NULL)
      {
        printConstAttribute(attr->constVal);
      }
      if (attr->funcParam != NULL)
      {
        printParamAttribute(attr->funcParam);
      }
    }
    entry = entry->next;
    printf("\n");
  }
  printf("=======================================================================================\n");
  return 0;
}

int printType(struct ExtType* extType)
{
  struct ArrayDimNode *dimNode;
  char strBuffer[20];
  char strTemp[20];
  if (extType == NULL)
    return -1;
  memset(strBuffer,0,20*sizeof(char));
  switch (extType->baseType)
  {
    case INT_t:
      strncpy(strBuffer,"int",3);
      break;
    case FLOAT_t:
      strncpy(strBuffer,"float",5);
      break;
    case DOUBLE_t:
      strncpy(strBuffer,"double",6);
      break;
    case BOOL_t:
      strncpy(strBuffer,"bool",4);
      break;
    case STRING_t:
      strncpy(strBuffer,"string",6);
      break;
    case VOID_t:
      strncpy(strBuffer,"void",4);
      break;
    default:
      strncpy(strBuffer,"unknown",7);
      break;
  }
  if (extType->isArray)
  {
    dimNode = extType->dimArray;
    while (dimNode != NULL)
    {
      memset(strTemp,0,20*sizeof(char));
      sprintf(strTemp,"[%d]",dimNode->size);
      if (strlen(strBuffer)+strlen(strTemp) < 20)
        strcat(strBuffer,strTemp);
      else
      {
        strBuffer[16]='.';
        strBuffer[17]='.';
        strBuffer[18]='.';
      }
      dimNode = dimNode->next;
    }
  }
  printf("%-19s",strBuffer);
  return 0;
}


char* typeString(struct ExtType* extType)
{
  struct ArrayDimNode *dimNode;
  char *strBuffer;
  char strTemp[20];
  if (extType == NULL)
    return "";
  strBuffer = (char*)malloc(20*sizeof(char));
  switch (extType->baseType)
  {
    case INT_t:
      strncpy(strBuffer,"int",3);
      strBuffer[3] = '\0'; // to fix a weird bug: 'int?' is displayed
      break;
    case FLOAT_t:
      strncpy(strBuffer,"float",5);
      break;
    case DOUBLE_t:
      strncpy(strBuffer,"double",6);
      break;
    case BOOL_t:
      strncpy(strBuffer,"bool",4);
      break;
    case STRING_t:
      strncpy(strBuffer,"string",6);
      break;
    case VOID_t:
      strncpy(strBuffer,"void",4);
      break;
    default:
      strncpy(strBuffer,"unknown",7);
      break;
  }
  if (extType->isArray)
  {
    dimNode = extType->dimArray;
    while (dimNode != NULL)
    {
      memset(strTemp,0,20*sizeof(char));
      sprintf(strTemp,"[%d]",dimNode->size);
      if (strlen(strBuffer)+strlen(strTemp) < 20)
        strcat(strBuffer,strTemp);
      else
      {
        strBuffer[16]='.';
        strBuffer[17]='.';
        strBuffer[18]='.';
      }
      dimNode = dimNode->next;
    }
  }
  return strBuffer;
}

int printConstAttribute(struct ConstAttr* constAttr)
{
  switch (constAttr->type)
  {
    case INT_t:
      printf("%d",constAttr->value.integerVal);
      break;
    case FLOAT_t:
      printf("%f",constAttr->value.floatVal);
      break;
    case DOUBLE_t:
      printf("%lf",constAttr->value.doubleVal);
      break;
    case BOOL_t:
      if (constAttr->value.boolVal)
        printf("true");
      else
        printf("false");
      break;
    case STRING_t:
      printf("%s",constAttr->value.stringVal);
      break;
    default:
      printf("__ERROR__");
      break;
  }
  return 0;
}

int printParamAttribute(struct FuncAttr* funcAttr)
{
  struct FuncAttrNode* attrNode = funcAttr->head;
  struct ArrayDimNode* dimNode;
  if (attrNode != NULL)
  {
    switch (attrNode->value->baseType)
    {
      case INT_t:
        printf("int");
        break;
      case FLOAT_t:
        printf("float");
        break;
      case DOUBLE_t:
        printf("double");
        break;
      case BOOL_t:
        printf("bool");
        break;
      case STRING_t:
        printf("string");
        break;
      case VOID_t:
        printf("void");
        break;
      default:
        printf("unknown");
        break;
    }
    if (attrNode->value->isArray)
    {
      dimNode = attrNode->value->dimArray;
      while (dimNode != NULL)
      {
        printf("[%d]",dimNode->size);
        dimNode = dimNode->next;
      }
    }
    attrNode = attrNode->next;
    while (attrNode != NULL)
    {
      switch (attrNode->value->baseType)
      {
        case INT_t:
          printf(",int");
          break;
        case FLOAT_t:
          printf(",float");
          break;
        case DOUBLE_t:
          printf(",double");
          break;
        case BOOL_t:
          printf(",bool");
          break;
        case STRING_t:
          printf(",string");
          break;
        case VOID_t:
          printf(",void");
          break;
        default:
          printf(",unknown");
          break;
      }
      if (attrNode->value->isArray)
      {
        dimNode = attrNode->value->dimArray;
        while (dimNode != NULL)
        {
          printf("[%d]",dimNode->size);
          dimNode = dimNode->next;
        }
      }
      attrNode = attrNode->next;
    }
  }
  return 0;
}

struct VariableList* createVariableList(struct Variable* head)
{
  struct VariableList *list;
  if (head == NULL)
    list = NULL;
  else
  {
    list = (struct VariableList*)malloc(sizeof(struct VariableList));
    struct Variable* temp = head;
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
  }
  return list;
}

int deleteVariableList(struct VariableList* list)
{
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
      list->head=deleteVariable(list->head);
    }
  }
  return 0;
}

int connectVariableList(struct VariableList* list,struct Variable* node)
{
  if (list == NULL || node == NULL)
    return -1;
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

struct Variable* createVariable(const char* name,struct ExtType* type)
{
  struct Variable* variable = (struct Variable*)malloc(sizeof(struct Variable));
  variable->name = strdup(name);
  /**/
  variable->type = type;
  type->reference += 1;
  /**/
  variable->next = NULL;
  variable->reference = 0;
  return variable;
}

struct Variable* deleteVariable(struct Variable* target)
{
  struct Variable* next;
  if (target == NULL)
    next = NULL;
  else
  {
    target->reference -= 1;
    if (target->reference > 0)
      return NULL;
    free(target->name);
    killExtType(target->type);
    next = target->next;
    free(target);
  }
  return next;
}

struct InitArray* createInitArray()
{
  struct InitArray* list;
  list = (struct InitArray*)malloc(sizeof(struct InitArray));
  list->reference = 0;
  list->size = 1;
  return list;
}

int deleteInitArray(struct InitArray* target)
{
  if (target == NULL)
    return -1;
  target->reference -= 1;
  if (target->reference > 0) return -1;
  free(target);
  return 0;
}

int connectInitArray(struct InitArray* list)
{
  if (list == NULL) return -1;
  list->size += 1;
  return 0;
}

bool canConvertTypeImplicitly(struct ExtType* source, struct ExtType* target, bool arg)
{
  if (source == NULL || target == NULL) return false;
  if (source->isArray != target->isArray) return false;
  if (source->baseType != target->baseType)
  {
    switch (target->baseType)
    {
      case INT_t:
        return false;
      case FLOAT_t:
        if (source->baseType == INT_t) break;
        return false;
      case DOUBLE_t:
        if (source->baseType == INT_t || source->baseType == FLOAT_t) break;
        return false;
      case BOOL_t:
      case STRING_t:
      case VOID_t:
        return false;
    }
  }
  if (source->isArray)
  {
    if (arg && !checkArraySize(source->dimArray, target->dimArray))
      return false;
    if (!arg && checkArrayDim(source->dimArray, target->dimArray) != 0)
      return false;
  }
  return true;
}

int checkArrayDim(struct ArrayDimNode* a1, struct ArrayDimNode* a2)
{
  int dim = 0;
  while (a1 != NULL && a2 != NULL)
  {
    dim += 1;
    a1 = a1->next;
    a2 = a2->next;
  }
  if (a1 == NULL && a2 == NULL) return 0;
  if (a1 != NULL) return -1; // too many subscripts
  if (a2 != NULL) return dim;
  return 0;
}

bool checkArraySize(struct ArrayDimNode* a1, struct ArrayDimNode* a2)
{
  while (a1 != NULL && a2 != NULL)
  {
    if (a1->size != a2->size) return false;
    a1 = a1->next;
    a2 = a2->next;
  }
  return (a1 == NULL && a2 == NULL);
}
