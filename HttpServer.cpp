#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <cstdlib>
#include <sys/wait.h>
#include <fstream>

#define PORT 8081
#define BUFFER_SIZE 1024

std::vector<double> compute_ln_series(double x, size_t size) {
    std::vector<double> series(size);
    for (size_t n = 1; n <= size; ++n) {
        series[n - 1] = pow(-1, n - 1) * pow(x, n) / n;
    }
    return series;
}

std::string handle_compute(double x) {
    const size_t array_size = 100000; 
    const int sort_iterations = 100; 
    auto series = compute_ln_series(x, array_size);
    
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < sort_iterations; ++i) {
        std::sort(series.begin(), series.end());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::ostringstream response_body;
    response_body << "Computation and sorting took " << duration_ms << " ms.\n"
                  << "Sorted " << sort_iterations << " times.\n"
                  << "First 10 computed values: ";
    for (size_t i = 0; i < std::min(series.size(), size_t(10)); ++i) {
        response_body << series[i] << " ";
    }
    response_body << "\n";

    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/plain\r\n"
             << "Content-Length: " << response_body.str().length() << "\r\n"
             << "\r\n"
             << response_body.str();

    return response.str();
}

std::string handle_file_request(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (file.is_open()) {
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: application/octet-stream\r\n"
                 << "\r\n";
        response << file.rdbuf(); 
        return response.str();
    } else {
        return "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found.";
    }
}

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE] = {0};
    read(client_fd, buffer, BUFFER_SIZE);

    std::string request(buffer);
    std::istringstream request_stream(request);
    std::string method, resource;
    request_stream >> method >> resource;

    std::string response;

    if (method == "GET") {
        if (resource == "/compute") {
            response = handle_compute(0.5); 
        } else if (resource.find("/compute?x=") == 0) {
            try {
                double x = std::stod(resource.substr(12)); 
                response = handle_compute(x);
            } catch (...) {
                response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid x parameter.";
            }
        } else if (resource.find(".") != std::string::npos) {
            response = handle_file_request("." + resource);
        } else {
            response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nResource not found.";
        }
    } else {
        response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/plain\r\n\r\nOnly GET is allowed.";
    }

    send(client_fd, response.c_str(), response.size(), 0);
    close(client_fd);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is running on port " << PORT << "...\n";

    while (true) {
        if ((client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        if (fork() == 0) {
            close(server_fd);
            handle_client(client_fd);
            exit(0);
        } else {
            close(client_fd);
            waitpid(-1, nullptr, WNOHANG);
        }
    }

    close(server_fd);
    return 0;
}
