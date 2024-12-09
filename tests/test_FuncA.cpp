#include <gtest/gtest.h>
#include "FuncA.h"
#include <chrono>

TEST(FuncATest, CalculateLn) {
    FuncA func;
    EXPECT_NEAR(func.calculate(0.5, 10), 0.405465, 0.0001);
}

TEST(FuncATest, AdaptivePerformanceTest) {
    FuncA func;
    double targetTime = 10.0; 
    int n = 1000000;
    auto start = std::chrono::high_resolution_clock::now();
    func.calculate(0.5, n);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    EXPECT_GE(elapsed.count(), 5.0);
    EXPECT_LE(elapsed.count(), 20.0);
}
