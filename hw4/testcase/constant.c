#pragma source on

// constant type and literal type mismatch
const int i = "string";
const float f = 0;      // legal after type coersion
const double d = 0.0;   // legal after type coersion
const bool b = 0;
const string s = 0;

// cannot assign to constant
void assignConstant() { i = 0; }
