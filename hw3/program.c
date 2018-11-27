#pragma symbol on
bool func_a();
bool func_b(int a[2][3], string b);
void func_c();
void func_d(int a, float b, double c, string d, bool e);

int main(string a, int b[2][3])
{
  int array[3] = { 1, 2, 3 };
  const double d = 1.23;
  const int i = 10;
  const float f = 3.14;
  double d = 3.14159;
  const bool flag_1 = true;
  const bool flag_2 = false;
  const string s = "this is a string";
  return 0;
}

bool func_a();
bool func_b(int a[2][3], string b)
{
  int i;
  int j = 10;
  for (i = 0; i < j; i = i + 1) {
    int i = 10, j = 10;
  }
  int j = 20;
}
