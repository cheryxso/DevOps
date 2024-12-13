#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>
#include <chrono>
#include <algorithm>
#include <random>

#define PORT 8081

void handleClient(int clientSocket);
void handleGetFile(int clientSocket, const std::string &filePath);
void handleCompute(int clientSocket);

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Attach socket to port
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to the address
    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(serverSocket, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        std::cout << "Waiting for a connection..." << std::endl;

        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();
        if (pid == 0) { // Child process
            close(serverSocket);
            handleClient(clientSocket);
            close(clientSocket);
            exit(0);
        } else if (pid > 0) {
            close(clientSocket);
        } else {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

void handleClient(int clientSocket) {
    char buffer[4096] = {0};
    read(clientSocket, buffer, sizeof(buffer));

    std::string request(buffer);
    std::cout << "Received request:\n" << request << std::endl;

    std::string method = request.substr(0, request.find(' '));
    std::string path = request.substr(request.find(' ') + 1);
    path = path.substr(0, path.find(' '));

    if (method == "GET") {
        if (path == "/compute") {
            handleCompute(clientSocket);
        } else {
            std::string filePath = "." + path;
            handleGetFile(clientSocket, filePath);
        }
    } else {
        std::string response = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
    }
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

    std::string header = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(statBuf.st_size) + "\r\n\r\n";
    send(clientSocket, header.c_str(), header.size(), 0);

    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = read(file, buffer, sizeof(buffer))) > 0) {
        send(clientSocket, buffer, bytesRead, 0);
    }

    close(file);
}

void handleCompute(int clientSocket) {
    auto start = std::chrono::high_resolution_clock::now();

    const int n = 1000000; // Number of values to compute
    std::vector<double> values(n);

    // Generate random values
    std::mt19937 gen(12345);
    std::uniform_real_distribution<> dis(0.0, 1.0);
    for (int i = 0; i < n; ++i) {
        values[i] = dis(gen);
    }

    // Compute series
    for (double &x : values) {
        x = 0.0;
        for (int k = 1; k <= 100; ++k) {
            x += (k % 2 == 0 ? -1 : 1) * std::pow(x, k) / k;
        }
    }

    std::sort(values.begin(), values.end());

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::string body = "Computation time: " + std::to_string(duration) + " ms\n";
    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
    send(clientSocket, response.c_str(), response.size(), 0);
}
