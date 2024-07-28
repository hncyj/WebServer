#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080
#define SIZE 1024

int main() {
    int server_socket_fd, new_socket_fd;
    struct sockaddr_in server_socket_info;
    char buffer[SIZE];
    int opt = 1;
    socklen_t server_socket_struct_len = sizeof(server_socket_info);
    // create server welcome socket file descriptor
    if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("create server welcome socket failed.");
        exit(EXIT_FAILURE);
    }
    // set server welcome socket's opt
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("set server welcome socket's opt failed.");
        exit(EXIT_FAILURE);
    }
    // set welcome socket, bind it to fixed port
    server_socket_info.sin_family = AF_INET;
    server_socket_info.sin_port = htons(PORT);
    server_socket_info.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket_fd, (struct sockaddr*) &server_socket_info, server_socket_struct_len) < 0) {
        perror("bind server sock info to server_sock_struct failed.");
        exit(EXIT_FAILURE);
    }
    // create new socket to accept new connection request
    if (listen(server_socket_fd, 3) < 0) {
        perror("listen failed.");
        exit(EXIT_FAILURE);
    }
    if ((new_socket_fd = accept(server_socket_fd, (struct sockaddr*) &server_socket_info, &server_socket_struct_len)) < 0) {
        perror("accept failed.");
        exit(EXIT_FAILURE);
    }
    // read message
    memset(buffer, 0, SIZE);
    if (read(new_socket_fd, buffer, SIZE) < 0) {
        perror("read failed.");
    } else {
        std::cout << "Message from client: " << buffer << std::endl;
    }

    // close connection
    close(new_socket_fd);
    close(server_socket_fd);

    return 0;
}