#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <string>
#include <vector>

class HttpServer {
public:
    void start();

private:
    void handleClient(int client_socket);
    std::vector<double> prepareAndSortData(int n);
    std::string handleComputeRequest(int n);
    int extractParameter(const std::string& request, const std::string& param, int default_value);
};

#endif
