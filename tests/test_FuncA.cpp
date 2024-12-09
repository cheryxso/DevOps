#include <iostream>
#include "FuncA.h"

int main() {
    FuncA func;
    double result = func.calculate(0.5, 10);
    std::cout << "FuncA result: " << result << std::endl;

    // Очікуваний результат для x = 0.5, n = 10
    if (result >= 0.4 && result <= 0.41) {
        std::cout << "Test Passed!" << std::endl;
    } else {
        std::cout << "Test Failed!" << std::endl;
    }
    return 0;
}
