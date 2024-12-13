#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PORT 8081

void handleGetCompute(int clientSocket);
void handleGetFile(int clientSocket, const std::string &filePath);
void handlePutRequest(int clientSocket, const std::string &filePath, const std::string &body);

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 10) < 0) {
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    while (true) {
        std::cout << "Waiting for connections on port " << PORT << "..." << std::endl;
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        int pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(clientSocket);
            continue;
        }

        if (pid == 0) { // Child process
            close(serverSocket);

            char buffer[30000] = {0};
            int bytesRead = read(clientSocket, buffer, sizeof(buffer));
            if (bytesRead <= 0) {
                close(clientSocket);
                exit(0);
            }

            std::string request(buffer);
            size_t methodEnd = request.find(' ');
            size_t pathEnd = request.find(' ', methodEnd + 1);

            std::string method = request.substr(0, methodEnd);
            std::string path = request.substr(methodEnd + 1, pathEnd - methodEnd - 1);

            if (method == "GET") {
                if (path == "/compute") {
                    handleGetCompute(clientSocket);
                } else {
                    handleGetFile(clientSocket, "." + path);
                }
            } else if (method == "PUT") {
                size_t bodyPos = request.find("\r\n\r\n") + 4;
                std::string body = request.substr(bodyPos);
                handlePutRequest(clientSocket, "." + path, body);
            } else {
                std::string response = "HTTP/1.1 400 Bad Request\r\n\r\n";
                send(clientSocket, response.c_str(), response.size(), 0);
            }

            close(clientSocket);
            exit(0);
        } else { // Parent process
            close(clientSocket);
        }
    }

    close(serverSocket);
    return 0;
}

void handleGetCompute(int clientSocket) {
    auto start = std::chrono::high_resolution_clock::now();

    const int n = 1000000;
    const double x = 0.5;
    std::vector<double> values(n);

    for (int i = 1; i <= n; ++i) {
        values[i - 1] = pow(-1, i - 1) * pow(x, i) / i;
    }

    std::sort(values.begin(), values.end());

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nTime elapsed: " + std::to_string(elapsed.count()) + " seconds\n";
    send(clientSocket, response.c_str(), response.size(), 0);
}

void handleGetFile(int clientSocket, const std::string &filePath) {
    int file = open(filePath.c_str(), O_RDONLY);
    if (file < 0) {
        std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        return;
    }

    struct stat statBuf;
    fstat(file, &statBuf);

    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(statBuf.st_size) + "\r\n\r\n";
    send(clientSocket, response.c_str(), response.size(), 0);

    sendfile(clientSocket, file, nullptr, statBuf.st_size);
    close(file);
}

void handlePutRequest(int clientSocket, const std::string &filePath, const std::string &body) {
    int file = open(filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (file < 0) {
        std::string response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
        return;
    }

    write(file, body.c_str(), body.size());
    close(file);

    std::string response = "HTTP/1.1 201 Created\r\n\r\n";
    send(clientSocket, response.c_str(), response.size(), 0);
}
