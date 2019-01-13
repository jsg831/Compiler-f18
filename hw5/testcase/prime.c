#pragma symbol on

int sqrt(int n)
{
  int s = 0;
  while (s*s < n) { s = s + 1; }
  return s;
}

bool is_prime(int n)
{
  int i, l = sqrt(n);
  bool prime = true;
  for (i = 2; i <= l; i = i + 1) {
    if (n%i == 0) {
      prime = false;
      break;
    }
  }
  return prime;
}

int main()
{
  int n, count = 0;
  for (n = 2; n < 1000000; n = n + 1) {
    if (is_prime(n)) {
      count = count + 1;
    }
  }
  print "The number of primes under ";
  print n;
  print " is ";
  print count;
  print "\n";
}
