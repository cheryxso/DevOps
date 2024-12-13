#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "FuncA.h"

#define PORT 8081
#define BUFFER_SIZE 1024

void handle_client(int client_socket);
std::string compute_response();
void send_file(int client_socket, const std::string& file_path);

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Socket bind failed");
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 5) == -1) {
        perror("Socket listen failed");
        close(server_socket);
        return 1;
    }

    while (true) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket == -1) {
            perror("Client accept failed");
            continue;
        }

        if (fork() == 0) {
            close(server_socket);
            handle_client(client_socket);
            close(client_socket);
            exit(0);
        } else {
            close(client_socket);
            waitpid(-1, NULL, WNOHANG);
        }
    }

    close(server_socket);
    return 0;
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    std::string request;

    int bytes_received = read(client_socket, buffer, BUFFER_SIZE);
    if (bytes_received < 0) {
        perror("Failed to read from socket");
        return;
    }

    request = std::string(buffer, bytes_received);

    if (request.find("GET /compute") != std::string::npos) {
        auto start = std::chrono::high_resolution_clock::now();
        std::string response_body = compute_response();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
        response += "Elapsed time: " + std::to_string(elapsed.count()) + " seconds\n";
        response += response_body;

        write(client_socket, response.c_str(), response.length());
    } else if (request.find("GET ") != std::string::npos) {
        std::string file_path = "." + request.substr(4, request.find(" ", 4) - 4);
        send_file(client_socket, file_path);
    } else if (request.find("PUT ") != std::string::npos) {
        std::string file_path = "." + request.substr(4, request.find(" ", 4) - 4);
        std::ofstream file(file_path);
        file << request.substr(request.find("\r\n\r\n") + 4);
        file.close();

        std::string response = "HTTP/1.1 201 Created\r\n\r\n";
        write(client_socket, response.c_str(), response.length());
    } else {
        std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
        write(client_socket, response.c_str(), response.length());
    }
}

std::string compute_response() {
    FuncA func;
    int n = 1000; 
    std::vector<double> values(n);

    for (int i = 0; i < n; ++i) {
        double x = static_cast<double>(i) / n;
        values[i] = func.calculate(x, n);
    }

    std::sort(values.begin(), values.end());

    std::string result;
    for (double value : values) {
        result += std::to_string(value) + "\n";
    }

    return result;
}

void send_file(int client_socket, const std::string& file_path) {
    int file = open(file_path.c_str(), O_RDONLY);
    if (file == -1) {
        std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
        write(client_socket, response.c_str(), response.length());
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = read(file, buffer, BUFFER_SIZE)) > 0) {
        write(client_socket, buffer, bytes_read);
    }

    close(file);
}
