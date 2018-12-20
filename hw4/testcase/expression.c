#pragma source on

int i; float f; double d; bool b; string s;

// type checking
void typeChecking() {
  // + - * /
  d = i / (d - f); // legal
  d = b + d; // illegal ('bool' + 'double')
  d = i * s; // illegal ('int' * 'string')
  i = i * d; // illegal ('int' = 'double')
  f = i + d; // illegal ('float' = 'double')
  // %
  d = i % d; // illegal ('int' % 'double')
  // - (unary)
  d = -b; // illegal (-'bool')
  d = -s; // illegal (-'string')
  // && || !
  b = !b && b || b; // legal
  b = i && b; // illegal ('int' && 'bool')
  b = b || i; // illegal ('bool' || 'int')
  b = !i; // illegal (!'int')
  d = b || b; // illegal ('double' = 'bool')
  // < <= >= >
  b = (i < f) || (i <= d) && (f > d) || (d >= d); // legal
  b = b < f; // illegal ('bool' < 'float')
  b = d > s; // illegal ('double' > 'string')
  d = i > i; // illegal ('double' = 'bool')
  // == !=
  b = (b != b) == (i < f); // legal
  b = i == s; // illegal ('int' == 'string')
  b = s != s; // illegal ('string' != 'string')
  d = d == i; // illegal ('double' = 'bool')
}

// function in expression
void funcVoid();
int funcInt();
void functionExpression() {
  d = funcInt(); // legal
  d = funcVoid(); // illegal ('double' = 'void')
}

// array in expression
void array() {
  int a[2][2];
  i = a[2] + i; // array cannot be an operand
}

// function calling
double func(int i, float f, double d, bool b, string s);
void functionCalling() {
  d = func(i,f,d,b,s); // legal
  i = func(i,f,d,b,s); // illegal ('int' = 'double')
  func(i,i,i,b,s); // legal
  func(f,f,f,b,s); // type of argument 1 is 'float'. expected 'int'
}
