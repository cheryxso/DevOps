#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <string>

class HttpServer {
public:
    void start();

private:
    void handleClient(int client_fd);
    std::string handleCalculate(double x);
};

#endif
