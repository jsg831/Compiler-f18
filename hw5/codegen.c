#include "codegen.h"

FILE* codeFile;
char* progName;

extern int loadCount;

int jumpLabelSize = 0;
struct JumpLabel* jumpLabel = 0;

int labelCount = 0;
int ifElseLabelCount = 0;
int continueLabel = 0;
int breakLabel = 0;

void initCodeGen(const char* filename)
{
  int len = strlen(filename);
  progName = malloc(len+2);
  int dot = len;
  // Find the last dot
  for (int i = len-1; i >= 0; --i) {
    if (filename[i] == '.') {
      dot = i;
      break;
    }
  }
  // Replace the string by "j" after the dot if one exist, otherwise append ".j"
  for (int i = 0; i <= len; ++i) {
    progName[i] = filename[i];
    if (i == dot) {
      progName[i] = '.';
      progName[i+1] = 'j';
      progName[i+2] = '\0';
      break;
    }
  }
  codeFile = fopen(progName, "w");
  // Add initial code
  // Remove the extension ".j"
  len = strlen(progName);
  int slash = 0;
  for (int i = len-1; i >= 0; --i) {
    if (progName[i] == '/') {
      slash = i+1;
      break;
    }
  }
  for (int i = len-1; i >= 0; --i) {
    if (progName[i] == '.') {
      progName[i] = '\0';
      break;
    }
  }
  for (int i = slash; i < len-1; ++i) {
    progName[i-slash] = progName[i];
    if (progName[i] == '\0') break;
  }
  fprintf(codeFile, ".source %s\n", filename);
  fprintf(codeFile, ".class public %s\n", progName);
  fprintf(codeFile, ".super java/lang/Object\n");
  fprintf(codeFile, ".field public static _sc Ljava/util/Scanner;\n");
  // temporary double variable for type coersion
  fprintf(codeFile, ".field public static _temp_double_0 D\n");
  fprintf(codeFile, ".field public static _temp_double_1 D\n");
}

void terminateCodeGen()
{
  fclose(codeFile);
  free(progName);
}

char javaTypeChar(SEMTYPE type)
{
  switch (type) {
    case VOID_t: return 'V';
    case INTEGER_t: return 'I';
    case BOOLEAN_t: return 'Z';
    case FLOAT_t: return 'F';
    case DOUBLE_t: return 'D';
    default: return '-';
  }
}

char javaFuncChar(SEMTYPE type)
{
  switch (type) {
    case INTEGER_t:
    case BOOLEAN_t:
      return 'i';
    case FLOAT_t:
      return 'f';
    case DOUBLE_t:
      return 'd';
    default: return '-';
  }
}

void printFuncName(struct SymNode* node)
{
  fprintf(codeFile, "%s(", node->name);
  if (node->attribute->formalParam != NULL) {
    struct PTypeList* head = node->attribute->formalParam->params;
    while (head != NULL) {
      fprintf(codeFile, "%c", javaTypeChar(head->value->type));
      head = head->next;
    }
  }
  fprintf(codeFile, ")%c", javaTypeChar(node->type->type));
}

void genMainStart(const char* name)
{
  fprintf(codeFile, ".method public static main([Ljava/lang/String;)V\n");
  fprintf(codeFile, "  new java/util/Scanner\n");
  fprintf(codeFile, "  dup\n");
  fprintf(codeFile, "  getstatic java/lang/System/in Ljava/io/InputStream;\n");
  fprintf(codeFile, "  invokespecial java/util/Scanner/<init>(Ljava/io/InputStream;)V\n");
  fprintf(codeFile, "  putstatic %s/_sc Ljava/util/Scanner;\n", progName);
}

void genFuncStart(struct SymNode* node)
{
  fprintf(codeFile, ".method public static ");
  printFuncName(node);
  fprintf(codeFile, "\n");
  fprintf(codeFile, "  new java/util/Scanner\n");
  fprintf(codeFile, "  dup\n");
  fprintf(codeFile, "  getstatic java/lang/System/in Ljava/io/InputStream;\n");
  fprintf(codeFile, "  invokespecial java/util/Scanner/<init>(Ljava/io/InputStream;)V\n");
  fprintf(codeFile, "  putstatic %s/_sc Ljava/util/Scanner;\n", progName);
}

void genFuncEnd(bool isVoid)
{
  if (isVoid)
    fprintf(codeFile, "  return\n");
  fprintf(codeFile, "  .limit stack 100\n");
  fprintf(codeFile, "  .limit locals 100\n");
  fprintf(codeFile, ".end method\n");
}

void genFieldVar(struct SymNode* node)
{
  fprintf(codeFile, ".field public static %s %c\n", node->name, javaTypeChar(node->type->type));
}

void genLitLoad(struct ConstAttr* attr)
{
  switch (attr->category) {
    case INTEGER_t:
      fprintf(codeFile, "  ldc %d\n", attr->value.integerVal);
      break;
    case BOOLEAN_t:
      if (attr->value.booleanVal) fprintf(codeFile, "  iconst_1\n");
      else fprintf(codeFile, "  iconst_0\n");
      break;
    case STRING_t:
      fprintf(codeFile, "  ldc \"%s\"\n", attr->value.stringVal);
      break;
    case FLOAT_t:
      fprintf(codeFile, "  ldc %f\n", attr->value.floatVal);
      break;
    case DOUBLE_t:
      fprintf(codeFile, "  ldc %f\n", attr->value.doubleVal);
      break;
  }
}

void genVarLoad(struct SymNode* node)
{
  if (node->scope == 0) {
    fprintf(codeFile, "  getstatic %s/%s %c\n", progName, node->name, javaTypeChar(node->type->type));
  } else {
    switch (node->type->type) {
      case INTEGER_t:
      case BOOLEAN_t:
      case FLOAT_t:
      case DOUBLE_t:
        fprintf(codeFile, "  %cload ", javaFuncChar(node->type->type));
        break;
    }
    fprintf(codeFile, "%d\n", node->localNumber);
  }
}

void genConstLoad(struct SymNode* node)
{
  struct ConstAttr* attr = node->attribute->constVal;
  switch (node->type->type) {
    case INTEGER_t:
      fprintf(codeFile, "  ldc %d\n", attr->value.integerVal);
      break;
    case BOOLEAN_t:
      if (attr->value.booleanVal) fprintf(codeFile, "  iconst_1\n");
      else fprintf(codeFile, "  iconst_0\n");
      break;
    case STRING_t:
      fprintf(codeFile, "  ldc \"%s\"\n", attr->value.stringVal);
      break;
    case FLOAT_t:
      fprintf(codeFile, "  ldc %f\n", attr->value.floatVal);
      break;
    case DOUBLE_t:
      fprintf(codeFile, "  ldc2_w %f\n", attr->value.floatVal);
      break;
  }
}

void genTypeCoersion(SEMTYPE source_type, SEMTYPE target_type)
{
  if (source_type != target_type)
    fprintf(codeFile, "  %c2%c\n", javaFuncChar(source_type), javaFuncChar(target_type));
}

void genVarDeclStore(SEMTYPE type, const char* id, int scope, int localNumber)
{
  if (scope == 0) {
    fprintf(codeFile, "  putstatic %s/%s %c\n", progName, id, javaTypeChar(type));
  } else {
    switch (type) {
      case INTEGER_t:
      case BOOLEAN_t:
      case FLOAT_t:
      case DOUBLE_t:
        fprintf(codeFile, "  %cstore ", javaFuncChar(type));
        break;
    }
    fprintf(codeFile, "%d\n", localNumber);
  }
}

void genVarStore(struct SymNode* node)
{
  if (node->scope == 0) {
    fprintf(codeFile, "  putstatic %s/%s %c\n", progName, node->name, javaTypeChar(node->type->type));
  } else {
    switch (node->type->type) {
      case INTEGER_t:
      case BOOLEAN_t:
      case FLOAT_t:
      case DOUBLE_t:
        fprintf(codeFile, "  %cstore ", javaFuncChar(node->type->type));
        break;
    }
    fprintf(codeFile, "%d\n", node->localNumber);
  }
}

void genFuncCall(struct SymNode* node)
{
  fprintf(codeFile, "  invokestatic %s/", progName);
  printFuncName(node);
  fprintf(codeFile, "\n");
}

void dumpReturn(SEMTYPE type)
{
  if (type == DOUBLE_t)
    fprintf(codeFile, "  pop2\n");
  else
    fprintf(codeFile, "  pop\n");
}

void genUnaryOp(SEMTYPE type)
{
  switch (type) {
    case BOOLEAN_t:
      fprintf(codeFile, "  iconst_1\n");
      fprintf(codeFile, "  ixor\n");
      break;
    case INTEGER_t:
    case FLOAT_t:
    case DOUBLE_t:
      fprintf(codeFile, "  %cneg\n", javaFuncChar(type));
      break;
  }
}

void genBinaryOp(SEMTYPE lhsType, OPERATOR op, SEMTYPE rhsType)
{
  // Type coersion
  SEMTYPE expr_type = (lhsType > rhsType) ? lhsType : rhsType;
  if (lhsType != expr_type) {
    if (expr_type == DOUBLE_t) {
      fprintf(codeFile, "  putstatic %s/_temp_double_0 D\n", progName);
      fprintf(codeFile, "  %c2d\n", javaFuncChar(lhsType));
      fprintf(codeFile, "  putstatic %s/_temp_double_1 D\n", progName);
      fprintf(codeFile, "  getstatic %s/_temp_double_0 D\n", progName);
      fprintf(codeFile, "  getstatic %s/_temp_double_1 D\n", progName);
    } else {
      fprintf(codeFile, "  swap\n");
      fprintf(codeFile, "  %c2%c\n", javaFuncChar(lhsType), javaFuncChar(expr_type));
      fprintf(codeFile, "  swap\n");
    }
  } else if (rhsType != expr_type) {
    fprintf(codeFile, "  %c2%c\n", javaFuncChar(rhsType), javaFuncChar(expr_type));
  }
  // Operator
  switch (op) {
    case ADD_t:
      fprintf(codeFile, "  %cadd\n", javaFuncChar(expr_type));
      break;
    case SUB_t:
      fprintf(codeFile, "  %csub\n", javaFuncChar(expr_type));
      break;
    case MUL_t:
      fprintf(codeFile, "  %cmul\n", javaFuncChar(expr_type));
      break;
    case DIV_t:
      fprintf(codeFile, "  %cdiv\n", javaFuncChar(expr_type));
      break;
    case MOD_t:
      fprintf(codeFile, "  irem\n");
      break;
    case AND_t:
      fprintf(codeFile, "  iand\n");
      break;
    case OR_t:
      fprintf(codeFile, "  ior\n");
      break;
    case LT_t:
    case LE_t:
    case EQ_t:
    case GE_t:
    case GT_t:
    case NE_t:
      if (expr_type == FLOAT_t || expr_type == DOUBLE_t) {
        fprintf(codeFile, "  %ccmpl\n", javaFuncChar(expr_type));
      } else {
        fprintf(codeFile, "  isub\n");
      }
      switch (op) {
        case LT_t:
          fprintf(codeFile, "  iflt L%d\n", labelCount++);
          break;
        case LE_t:
          fprintf(codeFile, "  ifle L%d\n", labelCount++);
          break;
        case EQ_t:
          fprintf(codeFile, "  ifeq L%d\n", labelCount++);
          break;
        case GE_t:
          fprintf(codeFile, "  ifge L%d\n", labelCount++);
          break;
        case GT_t:
          fprintf(codeFile, "  ifgt L%d\n", labelCount++);
          break;
        case NE_t:
          fprintf(codeFile, "  ifne L%d\n", labelCount++);
          break;
      }
      fprintf(codeFile, "  iconst_0\n");
      fprintf(codeFile, "  goto L%d\n", labelCount++);
      fprintf(codeFile, "L%d:\n", labelCount-2);
      fprintf(codeFile, "  iconst_1\n");
      fprintf(codeFile, "L%d:\n", labelCount-1);
      break;
  }
}

void genPrintStart()
{
  fprintf(codeFile, "  getstatic java/lang/System/out Ljava/io/PrintStream;\n");
}

void genPrintEnd(SEMTYPE type)
{
  if (type == STRING_t)
    fprintf(codeFile, "  invokevirtual java/io/PrintStream/print(Ljava/lang/String;)V\n");
  else
    fprintf(codeFile, "  invokevirtual java/io/PrintStream/print(%c)V\n", javaTypeChar(type));
}

void genRead(struct SymNode* node)
{
  fprintf(codeFile, "  getstatic %s/_sc Ljava/util/Scanner;\n", progName);
  switch (node->type->type) {
    case BOOLEAN_t:
      fprintf(codeFile, "  invokevirtual java/util/Scanner/nextBoolean()Z\n");
      break;
    case INTEGER_t:
      fprintf(codeFile, "  invokevirtual java/util/Scanner/nextInt()I\n");
      break;
    case FLOAT_t:
      fprintf(codeFile, "  invokevirtual java/util/Scanner/nextFloat()F\n");
      break;
    case DOUBLE_t:
      fprintf(codeFile, "  invokevirtual java/util/Scanner/nextDouble()D\n");
      break;
  }
  if (node->scope == 0) {
    fprintf(codeFile, "  putstatic %s/%s %c\n", progName, node->name, javaTypeChar(node->type->type));
  } else {
    fprintf(codeFile, "  %cstore %d\n", javaFuncChar(node->type->type), node->localNumber);
  }
}

void genReturn(SEMTYPE returnType, SEMTYPE exprType)
{
  char returnTypeChar = javaFuncChar(returnType);
  char exprTypeChar = javaFuncChar(exprType);
  if (exprTypeChar != returnTypeChar)
    fprintf(codeFile, "  %c2%c\n", exprTypeChar, returnTypeChar);
  if (returnTypeChar != '-')
    fprintf(codeFile, "  %creturn\n", returnTypeChar);
}

int genIf()
{
  fprintf(codeFile, "  ifeq L%d\n", labelCount++);
  return labelCount-1;
}

int genElse(int label)
{
  fprintf(codeFile, "  goto L%d\n", labelCount++);
  fprintf(codeFile, "L%d:\n", label);
  return labelCount-1;
}

void genIfExit(int label)
{
  fprintf(codeFile, "L%d:\n", label);
}

int genWhile()
{
  fprintf(codeFile, "L%d:\n", labelCount++);
  labelCount++; // Reserved for "exit" label
  pushJumpLabel(labelCount-2, labelCount-1);
  return labelCount-1;
}

void genWhileBody(int label)
{
  fprintf(codeFile, "  ifeq L%d\n", label);
}

void genWhileExit(int label)
{
  fprintf(codeFile, "  goto L%d\n", label-1);
  fprintf(codeFile, "L%d:\n", label);
  popJumpLabel();
}

int genDo()
{
  fprintf(codeFile, "L%d:\n", labelCount++);
  labelCount += 2; // Reserved for "cond" and "exit" label
  pushJumpLabel(labelCount-2, labelCount-1);
  return labelCount-1;
}

void genDoCond(int label)
{
  fprintf(codeFile, "L%d:\n", label-1);
}

void genDoExit(int label)
{
  fprintf(codeFile, "  ifeq L%d\n", label);
  fprintf(codeFile, "  goto L%d\n", label-2);
  fprintf(codeFile, "L%d:\n", label);
  popJumpLabel();
}

int genFor()
{
  fprintf(codeFile, "L%d:\n", labelCount++);
  labelCount += 3; // Reserved for "exit", "body", and "incr" labels
  pushJumpLabel(labelCount-1, labelCount-3);
  return labelCount-1;
}

void genForIncr(int label)
{
  fprintf(codeFile, "  ifeq L%d\n", label-2);
  fprintf(codeFile, "  goto L%d\n", label-1);
  fprintf(codeFile, "L%d:\n", label);
}

void genForBody(int label)
{
  fprintf(codeFile, "  goto L%d\n", label-3);
  fprintf(codeFile, "L%d:\n", label-1);
}

void genForExit(int label)
{
  fprintf(codeFile, "  goto L%d\n", label);
  fprintf(codeFile, "L%d:\n", label-2);
  popJumpLabel();
}

void pushJumpLabel(int continueLabel, int breakLabel)
{
  jumpLabelSize++;
  if (jumpLabelSize == 1)
    jumpLabel = malloc(sizeof(struct JumpLabel));
  else
    jumpLabel = realloc(jumpLabel, jumpLabelSize*sizeof(struct JumpLabel));
  jumpLabel[jumpLabelSize-1].continueLabel = continueLabel;
  jumpLabel[jumpLabelSize-1].breakLabel = breakLabel;
}

void popJumpLabel()
{
  jumpLabelSize--;
  jumpLabel = realloc(jumpLabel, jumpLabelSize*sizeof(struct JumpLabel));
}

void genContinue()
{
  fprintf(codeFile, "  ; loop %d ;\n", jumpLabelSize-1);
  fprintf(codeFile, "  goto L%d\n", jumpLabel[jumpLabelSize-1].continueLabel);
}

void genBreak()
{
  fprintf(codeFile, "  goto L%d\n", jumpLabel[jumpLabelSize-1].breakLabel);
}
