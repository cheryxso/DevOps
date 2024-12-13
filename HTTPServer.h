#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <string>
#include <vector>
#include "FuncA.h"

class HTTPServer {
public:
    HTTPServer(int port);
    void start();

private:
    int port;
    void handleClientRequest(int clientSocket);
    std::vector<double> calculateAndSortValues(double x, int n, int &elapsedTime);
};

#endif
