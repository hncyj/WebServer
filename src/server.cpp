#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080
#define SIZE 1024

int main() {
    std::cout << "\n------------Sever start------------\n";
    int server_socket_fd, new_socket_fd;
    struct sockaddr_in server_socket_addr;
    char buffer[SIZE];
    int opt = 1;
    socklen_t server_sock_len = sizeof(server_socket_addr);

    // create server welcome socket file descriptor
    if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("create server's welcome socket failed.");
        exit(EXIT_FAILURE);
    }
    std::cout << "create server's welcome socket success." << std::endl;

    // set server welcome socket's opt
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("set server welcome socket's opt failed.");
        exit(EXIT_FAILURE);
    }
    std::cout << "set server's welcome socket opt success." << std::endl;

    // set welcome socket, bind it to fixed port
    server_socket_addr.sin_family = AF_INET;
    server_socket_addr.sin_port = htons(PORT);
    server_socket_addr.sin_addr.s_addr = INADDR_ANY;
    std::cout << "set welcome socket info success." << std::endl;

    if (bind(server_socket_fd, (struct sockaddr*) &server_socket_addr, server_sock_len) < 0) {
        perror("bind server sock info to server_sock_struct failed.");
        exit(EXIT_FAILURE);
    }
    std::cout << "bind server's welcome socket to fixed port success." << std::endl;

    std::cout << "start listening" << std::endl;
    // listen to connection
    if (listen(server_socket_fd, 3) < 0) {
        perror("listen failed.");
        exit(EXIT_FAILURE);
    }

    std::cout << "Start accept:" << std::endl;
    while (true) {
        if ((new_socket_fd = accept(server_socket_fd, (struct sockaddr*)& server_socket_addr, &server_sock_len)) < 0) {
            perror("Accept failed.\n");
            exit(EXIT_FAILURE);
        }
        std::cout << "Connection accepted.\n";
        while (true) {
            memset(buffer, 0, SIZE);
            int valread = read(new_socket_fd, buffer, SIZE - 1);
            if (valread <= 0) {
                std::cerr << "Client disconnected or read error" << std::endl;
                break;
            } else {
                buffer[valread] = '\0';
                std::cout << "Message from client: " << buffer << std::endl;
            }
            std::string response = "Message recieved.    --server";
            send(new_socket_fd, response.c_str(), response.size(), 0);
        }
        close(new_socket_fd);
        std::cout << "Connection close." << std::endl;
    }
    close(server_socket_fd);

    return 0;
}