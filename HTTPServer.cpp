#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "FuncA.h"

#define PORT 8081
#define HTTP_200HEADER "HTTP/1.1 200 OK\r\n"

void handleGETrequest(int clientSocket);
void sendResponse(int clientSocket, const std::string& response);

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Створення сокету
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Налаштування сокету
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Socket configuration failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Прив'язка сокету
    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Прослуховування порту
    if (listen(serverSocket, 10) < 0) {
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    std::cout << "HTTP server running on port " << PORT << std::endl;

    while (true) {
        std::cout << "Waiting for a new connection..." << std::endl;
        if ((clientSocket = accept(serverSocket, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        char buffer[1024] = {0};
        read(clientSocket, buffer, 1024);
        std::string request(buffer);
        std::cout << "Received request:\n" << request << std::endl;

        if (request.find("GET") == 0) {
            handleGETrequest(clientSocket);
        } else {
            std::string response = "HTTP/1.1 400 Bad Request\r\n\r\n";
            sendResponse(clientSocket, response);
        }

        close(clientSocket);
    }

    close(serverSocket);
    return 0;
}

void handleGETrequest(int clientSocket) {
    FuncA func;
    const int nElements = 2000000; // Кількість елементів масиву
    const int nIterations = 500;  // Кількість циклів сортування

    std::vector<double> values;
    values.reserve(nElements);

    // Обчислення значень функції ln(1 + x)
    for (int i = 1; i <= nElements; ++i) {
        double x = static_cast<double>(i) / nElements; // Значення x в діапазоні [0, 1]
        values.push_back(func.calculate(x, 100));      // 100 елементів ряду
    }

    // Початок вимірювання часу
    auto startTime = std::chrono::high_resolution_clock::now();

    // Сортування масиву кілька разів
    for (int i = 0; i < nIterations; ++i) {
        std::sort(values.begin(), values.end());
    }

    // Кінець вимірювання часу
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    // Підготовка відповіді
    std::string response = HTTP_200HEADER;
    response += "Content-Type: text/plain\r\n";
    response += "Content-Length: " + std::to_string(std::to_string(duration).size()) + "\r\n\r\n";
    response += std::to_string(duration);

    sendResponse(clientSocket, response);
}

void sendResponse(int clientSocket, const std::string& response) {
    send(clientSocket, response.c_str(), response.size(), 0);
    std::cout << "Response sent:\n" << response << std::endl;
}
