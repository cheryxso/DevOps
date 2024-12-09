#include <iostream>
#include "FuncA.h"
#include "HttpServer.h"

int main() {
    FuncA func;
    std::cout << "FuncA result (test): " << func.calculate(1.0, 10) << std::endl;

    HttpServer server;
    server.start();

    return 0;
}

