#include <iostream>
#include <chrono>
#include "HttpServer.h"

int main() {
    double x = 0.5;

    auto start = std::chrono::high_resolution_clock::now();

    std::string result = handle_calculate(x);

    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Calculation time: " << duration << " ms" << std::endl;

    if (duration >= 5000 && duration <= 20000) {
        std::cout << "Test Passed!" << std::endl;
    } else {
        std::cout << "Test Failed!" << std::endl;
    }

    return 0;
}
