#pragma source on

// define a variable for each type
int i; float f; double d; bool b; string s;

// (one declaration) -> one definition
// redeclaration
void redeclaration();
void redeclaration();
int redeclarationScalar();
int redeclarationScalar();
// redefinition
void redefinition() {}
void redefinition() {}
int redefinitionScalar() { return i; }
int redefinitionScalar() { return i; }
// defined before declaration
void definedBeforeDeclaration() {}
void definedBeforeDeclaration();
int definedBeforeDeclarationScalar() { return i; }
int definedBeforeDeclarationScalar();

// definition/declaration type mismatch
// number of arguments
void numberOfArguments(int a, float b);
void numberOfArguments(int a) {}
int numberOfArgumentsScalar(int a, float b);
int numberOfArgumentsScalar(int a) { return i; }
// type of arguments
void typeOfArguments(int a);
void typeOfArguments(float a) {}
int typeOfArgumentsScalar(int a);
int typeOfArgumentsScalar(float a) { return i; }
// array size
void arraySize(int a[1]);
void arraySize(int a[2]) {}
int arraySizeScalar(int a[1]);
int arraySizeScalar(int a[2]) { return i; }

// return type
// legal
void returnTypeLegalVoid() { }
int returnTypeLegalInt() { return i; }
float returnTypeLegalFloat() { return f; }
double returnTypeLegalDouble() { return d; }
bool returnTypeLegalBool() { return b; }
string returnTypeLegalString() { return s; }
// illegal
void returnTypeIllegalVoid() { return i; }
int returnTypeIllegalInt() { return f; }
float returnTypeIllegalFloat() { return d; }
double returnTypeIllegalDouble() { return b; }
bool returnTypeIllegalBool() { return i; }
string returnTypeIllegalString() { return i; }

// last statement is return
// legal
int returnLegal() { return i; }
// illegal
int noReturn() { for (;;) {} } // no return
int noEndReturn() { for (;;) { return i; } } // return not in the very end

// function symbol misuse
void functionOperand() {
  int functionOperand; // reuse function symbol
  functionOperand = i; // assign value to function
  i = functionOperand; // assign function to variable
}

// function invocation
void func(int a, float b);
void functionInvocation() {
  func(i); // too few arguments
  func(i, f, d); // too many arguments
}
