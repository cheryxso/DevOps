#include "HttpServer.h"
#include "FuncA.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#define PORT 8080

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

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    while (true) {
        std::cout << "Waiting for connections..." << std::endl;

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        char buffer[30000] = {0};
        read(new_socket, buffer, 30000);

        if (strstr(buffer, "GET /compute")) {
            std::string query(buffer);
            std::string response = handleComputeRequest(query);
            write(new_socket, response.c_str(), response.size());
        }

        close(new_socket);
    }
}

int HttpServer::determineOptimalN(double x, double targetTime) {
    FuncA func;
    int n = 1000; // Початкове значення
    double elapsed = 0.0;

    while (elapsed < targetTime) {
        auto start = std::chrono::high_resolution_clock::now();
        func.calculate(x, n);
        auto end = std::chrono::high_resolution_clock::now();
        elapsed = std::chrono::duration<double>(end - start).count();
        n *= 2; // Подвоюємо n для наступного тесту
    }

    return n / 2; // Повертаємо попереднє значення n, що відповідало б часу
}

std::vector<double> HttpServer::prepareAndSortData(double x, int n) {
    FuncA func;
    std::vector<double> data;

    for (int i = 1; i <= n; ++i) {
        data.push_back(func.calculate(x, i));
    }

    std::sort(data.begin(), data.end());
    return data;
}

std::string HttpServer::handleComputeRequest(const std::string& query) {
    double x = 0.5; // Значення за замовчуванням
    double targetTime = 10.0; // Цільовий час виконання 10 секунд
    size_t x_pos = query.find("x=");
    if (x_pos != std::string::npos) {
        x = std::stod(query.substr(x_pos + 2));
    }

    int n = determineOptimalN(x, targetTime);

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<double> sortedData = prepareAndSortData(x, n);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::string response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\n";
    response += "Elapsed time: " + std::to_string(elapsed.count()) + " seconds\n";
    response += "Number of elements: " + std::to_string(sortedData.size()) + "\n";

    return response;
}
