#pragma source on

int i; float f; double d; bool b; string s;

// check if the operands of assignment is legal after type coersion
void variable() {
  i = i; i = f; i = d; i = b; i = s; // 1 legal / 2,3,4,5 illegal
  f = i; f = f; f = d; f = b; f = s; // 1,2 legal / 3,4,5 illegal
  d = i; d = f; d = d; d = b; d = s; // 1,2,3 legal / 4,5 illegal
  b = i; b = f; b = d; b = b; b = s; // 4 legal / 1,2,3,5 illegal
  s = i; s = f; s = d; s = b; s = s; // 5 legal / 1,2,3,4 illegal
}

// check array operations
void array() {
  int a1[1][0]; // array size is not positive
  int a2[2] = {1,2,3}; // initial list is larger than the array size
  int a3[6] = { i, f, d, b, s, array }; // type mismatch of element in initial list
  int a4[1][2] = {0,1,2}; // initializing 2-d array with 1-d list
  i = a1[1][1]; // legal
  i = a1[1]; // assign array to variable
  a1[1] = i; // assign value to array
  i = a1[1][2][3]; // too many array subscripts
}
