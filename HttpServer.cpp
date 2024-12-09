#include "HttpServer.h"
#include "FuncA.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#define PORT 8081

void HttpServer::start() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server started on port " << PORT << std::endl;

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            continue; 
        }

        std::thread(&HttpServer::handleClient, this, new_socket).detach();
    }
}

void HttpServer::handleClient(int client_socket) {
    char buffer[30000] = {0};
    read(client_socket, buffer, 30000);

    std::string request(buffer);
    std::cout << "Received request: \n" << request << std::endl;

    if (request.find("GET /compute") != std::string::npos) {
        int n = extractParameter(request, "n", 100);
        std::string response = handleComputeRequest(n);
        write(client_socket, response.c_str(), response.size());
    } else {
        std::string error_response = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\n\nInvalid Request\n";
        write(client_socket, error_response.c_str(), error_response.size());
    }

    close(client_socket);
}

std::vector<double> HttpServer::prepareAndSortData(int n) {
    FuncA func;
    std::vector<double> data;

    for (int i = 1; i <= n; ++i) {
        data.push_back(func.calculate(0.5, i));
    }

    std::sort(data.begin(), data.end());
    return data;
}

std::string HttpServer::handleComputeRequest(int n) {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<double> sortedData = prepareAndSortData(n);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::ostringstream response;
    response << "HTTP/1.1 200 OK\nContent-Type: text/plain\n\n";
    response << "Elapsed time: " << elapsed.count() << " seconds\n";
    response << "Top 5 values: ";
    for (size_t i = 0; i < 5 && i < sortedData.size(); ++i) {
        response << sortedData[i] << " ";
    }
    response << "\n";

    return response.str();
}

int HttpServer::extractParameter(const std::string& request, const std::string& param, int default_value) {
    size_t param_pos = request.find(param + "=");
    if (param_pos != std::string::npos) {
        size_t value_start = param_pos + param.length() + 1;
        size_t value_end = request.find_first_of(" \r\n", value_start);
        std::string value_str = request.substr(value_start, value_end - value_start);
        try {
            return std::stoi(value_str);
        } catch (...) {
            return default_value;
        }
    }
    return default_value;
}
