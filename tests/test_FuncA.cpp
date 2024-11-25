#include <gtest/gtest.h>
#include "FuncA.h"

// Тест на обчислення ln(1 + x) для x = 1.0 і n = 10
TEST(FuncATest, PositiveX) {
    FuncA func;
    double result = func.calculate(1.0, 10);
    // Очікуваний результат (перші 10 елементів ряду для ln(1+x) при x = 1)
    double expected = 0.693147; // Тобто це наближене значення ln(2)
    EXPECT_NEAR(result, expected, 0.0001);  // Перевірка з точністю до 0.0001
}

// Тест на обчислення для негативного x
TEST(FuncATest, NegativeX) {
    FuncA func;
    double result = func.calculate(-0.5, 10);
    double expected = -0.405465;  // Наближене значення ln(0.5)
    EXPECT_NEAR(result, expected, 0.0001);
}

// Тест на мале значення x
TEST(FuncATest, SmallX) {
    FuncA func;
    double result = func.calculate(0.1, 10);
    double expected = 0.095310;  // Наближене значення ln(1.1)
    EXPECT_NEAR(result, expected, 0.0001);
}
