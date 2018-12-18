#pragma source on

int i; float f; double d; bool b; string s; int array[1];

// read/print
void readPrint() {
  read i; // legal
  read array; // read statement does not take array
  print array; // print statement does not take array
  if (i) {} // conditional statement of 'if' must be of type boolean
  while (f) {} // conditional statement of 'while' must be of type boolean
  do {} while(d); // conditional statement of 'while' must be of type boolean
  for (;i == 0;) {} // legal
  for (;i = 0;) {} // control expression of 'for' must be of type boolean
}

// jump
void testJump() {
  for (;;) {
    continue; // legal
    while (true) { break; } // legal
  }
  continue; // continue statement not in loop
  do { break; } while (false); // legal
  break; // break statement not in loop
}
