#include "FuncA.h"
#include <cmath>

//Ф-ія обчислює значення суми перших n елементів ряду для ln(1 + x)
//Параметри:
// x - значення, для якого обчислюється логарифм
// n - кл-ть елементів ряду для обчислення

double FuncA::calculate(double x, int n) {
  double sum = 0.0;
  for (int i = 1; i <= n; ++i) {
    sum += pow(-1, i - 1) * pow(x, i) / i;
  }
  return sum;
}
