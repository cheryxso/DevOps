#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <string>
#include <vector>

class HttpServer {
public:
    void start(); // Запускає сервер
private:
    int determineOptimalN(double x, double targetTime); // Визначає оптимальне n для цільового часу
    std::vector<double> prepareAndSortData(double x, int n);
    std::string handleComputeRequest(const std::string& query); // Обробляє /compute запит
};

#endif
