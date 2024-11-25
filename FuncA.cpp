#include "FuncA.h"
#include <cmath>

double FuncA::calculate(double x, int n) {
  double sum = 0.0;
  int terms = (n < 3) ? n : 3;
  for (int i = 1; i <= terms; ++i) {
    sum += pow(-1, i - 1) * pow(x, i) / i;
  }
  return sum;
}
