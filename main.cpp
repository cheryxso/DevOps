#include <iostream>
#include "FuncA.h"

int main() {
  FuncA func;
  std::cout << "FuncA result: " << func.calculate(1.0, 10) << std::endl;
  return 0;
}
