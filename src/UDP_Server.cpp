#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080
#define SIZE 1024

int main() {
    std::cout << "\n------------- Server Start ----------------" << std::endl;
    int server_socket_fd;
    struct sockaddr_in server_sock_addr, client_sock_addr;
    char buffer[SIZE];
    socklen_t client_sock_len = sizeof(client_sock_addr);

    if ((server_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Create Server's UDP Socket Failed.");
        exit(EXIT_FAILURE);
    }
    std::cout << "Create Server's UDP Socket sucess." << std::endl;

    server_sock_addr.sin_family = AF_INET;
    server_sock_addr.sin_port = htons(PORT);
    server_sock_addr.sin_addr.s_addr = INADDR_ANY;
    std::cout << "Set Sever sock's address success." << std::endl;

    if (bind(server_socket_fd, (struct sockaddr*) &server_sock_addr, sizeof(server_sock_addr)) < 0) {
        perror("bind server socket info to sever_sock_addr failed.");
        exit(EXIT_FAILURE);
    }
    std::cout << "Bind success." << std::endl;

    std::cout << "start listening: ...." << std::endl;
    while (true) {
        memset(buffer, 0, SIZE);
        int len = recvfrom(server_socket_fd, buffer, SIZE, 0, (struct sockaddr*) &client_sock_addr, &client_sock_len);
        if (len < 0) {
            perror("receive failed.");
            exit(EXIT_FAILURE);
        }
        std::cout << buffer << std::endl;
        std::string response = "Message Received.   ---Sever\n";
        sendto(server_socket_fd, response.c_str(), response.size(), 0, (struct sockaddr*) & client_sock_addr, client_sock_len);
    } 
    close(server_socket_fd);

    return 0;
}