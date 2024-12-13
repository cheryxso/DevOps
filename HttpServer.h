#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <string>

class HttpServer {
public:
    void start();

private:
    void handleClient(int client_socket);
    std::string compute_response();
    void send_file(int client_socket, const std::string& file_path);
};

#endif
