#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cmath>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 8081

class HttpServer {
public:
    void start() {
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
            perror("Bind failed");
            close(serverSocket);
            exit(EXIT_FAILURE);
        }

        if (listen(serverSocket, 10) < 0) {
            perror("Listen failed");
            close(serverSocket);
            exit(EXIT_FAILURE);
        }

        std::cout << "Server started on port " << PORT << std::endl;

        while (true) {
            int clientSocket;
            socklen_t addrLen = sizeof(address);
            if ((clientSocket = accept(serverSocket, (struct sockaddr*)&address, &addrLen)) < 0) {
                perror("Accept failed");
                continue;
            }

            if (fork() == 0) { 
                close(serverSocket);
                handleClient(clientSocket);
                close(clientSocket);
                exit(0);
            }

            close(clientSocket); 
        }

        close(serverSocket);
    }

private:
    void handleClient(int clientSocket) {
        char buffer[1024] = {0};
        read(clientSocket, buffer, 1024);

        std::string request(buffer);
        std::cout << "Received request:\n" << request << std::endl;

        std::string method = parseMethod(request);
        std::string path = parsePath(request);

        if (method == "GET") {
            handleGet(clientSocket, path);
        } else if (method == "PUT") {
            handlePut(clientSocket, path, request);
        } else {
            sendResponse(clientSocket, "400 Bad Request", "Unsupported HTTP method");
        }
    }

    std::string parseMethod(const std::string& request) {
        size_t pos = request.find(' ');
        return request.substr(0, pos);
    }

    std::string parsePath(const std::string& request) {
        size_t start = request.find(' ') + 1;
        size_t end = request.find(' ', start);
        return request.substr(start, end - start);
    }

    void handleGet(int clientSocket, const std::string& path) {
        if (path == "/compute") {
            auto start = std::chrono::high_resolution_clock::now();

            std::vector<double> values;
            for (int i = 1; i <= 5000000; ++i) {
                values.push_back(computeLn(1.0 / i));
            }

            std::sort(values.begin(), values.end());

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            sendResponse(clientSocket, "200 OK", "Computation took: " + std::to_string(duration.count()) + " ms");
        } else {
            std::string filePath = "." + path;
            sendFileResponse(clientSocket, filePath);
        }
    }

    void handlePut(int clientSocket, const std::string& path, const std::string& request) {
        size_t bodyPos = request.find("\r\n\r\n") + 4;
        std::string body = request.substr(bodyPos);

        std::string filePath = "." + path;
        std::ofstream outFile(filePath);
        if (outFile.is_open()) {
            outFile << body;
            outFile.close();
            sendResponse(clientSocket, "201 Created", "File created successfully");
        } else {
            sendResponse(clientSocket, "500 Internal Server Error", "Failed to create file");
        }
    }

    void sendFileResponse(int clientSocket, const std::string& filePath) {
        int file = open(filePath.c_str(), O_RDONLY);
        if (file < 0) {
            sendResponse(clientSocket, "404 Not Found", "File not found");
            return;
        }

        struct stat statBuf;
        fstat(file, &statBuf);

        std::string header = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(statBuf.st_size) + "\r\n\r\n";
        send(clientSocket, header.c_str(), header.size(), 0);

        off_t offset = 0;
        sendfile(clientSocket, file, &offset, statBuf.st_size);
        close(file);
    }

    void sendResponse(int clientSocket, const std::string& status, const std::string& body) {
        std::string response = "HTTP/1.1 " + status + "\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
        send(clientSocket, response.c_str(), response.size(), 0);
    }

    double computeLn(double x) {
        double sum = 0.0;
        for (int n = 1; n <= 100; ++n) {
            sum += std::pow(-1, n - 1) * std::pow(x, n) / n;
        }
        return sum;
    }
};

int main() {
    HttpServer server;
    server.start();
    return 0;
}
