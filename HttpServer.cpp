#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <chrono>
#include <vector>
#include <cmath>
#include <algorithm>
#include <sstream>

const int PORT = 8081;

void send_response(int client_socket, int status_code, const std::string& content_type, const std::string& content);
void handle_request(int client_socket);
void handle_get_compute(int client_socket);
void handle_get_file(int client_socket, const std::string& path);
void handle_put(int client_socket, const std::string& path, const std::string& body);
std::string get_mime_type(const std::string& filename);

int main() {
    int server_socket;
    struct sockaddr_in server_addr;
    socklen_t addr_size = sizeof(server_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, addr_size) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening on port " << PORT << std::endl;

    while (true) {
        int client_socket = accept(server_socket, (struct sockaddr*)&server_addr, &addr_size);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(client_socket);
            continue;
        } else if (pid == 0) {
            // Child process
            close(server_socket); // Close the listening socket in child
            handle_request(client_socket);
            close(client_socket);
            exit(EXIT_SUCCESS);
        } else {
            // Parent process
            close(client_socket); // Close the client socket in parent
        }
    }

    close(server_socket);
    return 0;
}

void handle_request(int client_socket) {
    char buffer[4096];
    std::string request;
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        request.append(buffer, bytes_received);
        if (request.find("\r\n\r\n") != std::string::npos)
            break;
    }

    if (bytes_received < 0) {
        perror("Error receiving data");
        return;
    }

    if (request.empty()) {
        send_response(client_socket, 400, "text/plain", "Bad Request");
        return;
    }

    size_t pos = request.find(' ');
    if (pos == std::string::npos) {
        send_response(client_socket, 400, "text/plain", "Bad Request");
        return;
    }
    std::string method = request.substr(0, pos);
    size_t pos2 = request.find(' ', pos + 1);
    if (pos2 == std::string::npos) {
        send_response(client_socket, 400, "text/plain", "Bad Request");
        return;
    }
    std::string path = request.substr(pos + 1, pos2 - pos - 1);

    if (method == "GET") {
        if (path == "/compute") {
            handle_get_compute(client_socket);
        } else {
            handle_get_file(client_socket, path);
        }
    } else if (method == "PUT") {
        size_t header_end = request.find("\r\n\r\n");
        if (header_end == std::string::npos) {
            send_response(client_socket, 400, "text/plain", "Bad Request");
            return;
        }
        size_t content_start = header_end + 4;
        std::string body = request.substr(content_start);
        handle_put(client_socket, path, body);
    } else {
        send_response(client_socket, 405, "text/plain", "Method Not Allowed");
    }
}

void send_response(int client_socket, int status_code, const std::string& content_type, const std::string& content) {
    std::string response;
    std::string status_line;

    switch (status_code) {
        case 200:
            status_line = "HTTP/1.1 200 OK\r\n";
            break;
        case 201:
            status_line = "HTTP/1.1 201 Created\r\n";
            break;
        case 404:
            status_line = "HTTP/1.1 404 Not Found\r\n";
            break;
        case 400:
            status_line = "HTTP/1.1 400 Bad Request\r\n";
            break;
        case 500:
            status_line = "HTTP/1.1 500 Internal Server Error\r\n";
            break;
        case 405:
            status_line = "HTTP/1.1 405 Method Not Allowed\r\n";
            break;
        default:
            status_line = "HTTP/1.1 500 Internal Server Error\r\n";
            break;
    }

    response = status_line +
               "Content-Type: " + content_type + "\r\n" +
               "Content-Length: " + std::to_string(content.size()) + "\r\n" +
               "Connection: close\r\n\r\n" +
               content;

    send(client_socket, response.c_str(), response.size(), 0);
}

void handle_get_compute(int client_socket) {
    auto start = std::chrono::high_resolution_clock::now();

    const int num_x_values = 9;
    const int num_terms = 1000;
    std::vector<double> results;

    for (double x = 0.1; x < 1.0; x += 0.1) {
        double sum = 0.0;
        for (int n = 0; n < num_terms; ++n) {
            sum += pow(-1, n) * pow(x, n + 1) / (n + 1);
        }
        results.push_back(sum);
    }

    std::sort(results.begin(), results.end());

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::ostringstream oss;
    oss << duration << " milliseconds";

    send_response(client_socket, 200, "text/plain", oss.str());
}

void handle_get_file(int client_socket, const std::string& path) {
    if (path == "/") {
        path = "/index.html";
    }

    std::string file_path = "." + path;
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        send_response(client_socket, 404, "text/plain", "File not found");
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::string mime_type = get_mime_type(file_path);

    send_response(client_socket, 200, mime_type, content);
}

void handle_put(int client_socket, const std::string& path, const std::string& body) {
    if (path.find("..") != std::string::npos) {
        send_response(client_socket, 400, "text/plain", "Bad Request");
        return;
    }

    std::string file_path = "." + path;
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        send_response(client_socket, 500, "text/plain", "Internal Server Error");
        return;
    }

    file << body;
    file.close();

    send_response(client_socket, 201, "text/plain", "File created/updated");
}

std::string get_mime_type(const std::string& filename) {
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    if (extension == "html" || extension == "htm")
        return "text/html";
    else if (extension == "css")
        return "text/css";
    else if (extension == "js")
        return "application/javascript";
    else if (extension == "jpg" || extension == "jpeg")
        return "image/jpeg";
    else if (extension == "png")
        return "image/png";
    else if (extension == "gif")
        return "image/gif";
    else if (extension == "ico")
        return "image/x-icon";
    else
        return "application/octet-stream";
}
