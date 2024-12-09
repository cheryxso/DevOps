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

#define PORT 8081
#define BUFFER_SIZE 1024

std::vector<double> generate_ln_array(double x, size_t size) {
    std::vector<double> series(size);
    for (size_t n = 1; n <= size; ++n) {
        series[n - 1] = pow(-1, n - 1) * pow(x, n) / n;
    }
    return series;
}

std::string handle_calculate(double x) {
    const size_t array_size = 1000000;  
    const int sort_iterations = 300;    
    auto series = generate_ln_array(x, array_size);

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < sort_iterations; ++i) {
        std::sort(series.begin(), series.end());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::ostringstream response_body;
    response_body << "Array sorted " << sort_iterations << " times in " << duration_ms << " ms.";

    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/plain\r\n"
             << "Content-Length: " << response_body.str().length() << "\r\n"
             << "\r\n"
             << response_body.str();

    return response.str();
}

std::string handle_comput() {
    const std::string body = "Computer!";
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/plain\r\n"
             << "Content-Length: " << body.length() << "\r\n"
             << "\r\n"
             << body;
    return response.str();
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
        if (resource == "/comput") {
            response = handle_comput();
        } else if (resource.find("/calculate?x=") == 0) {
            try {
                double x = std::stod(resource.substr(14));
                response = handle_calculate(x);
            } catch (...) {
                response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid x parameter.";
            }
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
}
