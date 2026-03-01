# include "server.hpp"
#include <stdexcept>
#include <string>
#include <sys/socket.h>


void    Server::addListeningSocket(int port)
{
    int newServSocket;

    newServSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (newServSocket < 0)
        throw std::runtime_error("Failed to create listening socket port : " + std::to_string(port));
    
    int opt = 1;
    if (setsockopt(newServSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        close(newServSocket);
        throw std::runtime_error("Failed to set socket options " + std::to_string(port));
    }

    struct sockaddr_in nic;
    memset(&nic, 0, sizeof(nic));
    nic.sin_family = AF_INET;
    nic.sin_port = htons(port);
    if (inet_pton(nic.sin_family, "0.0.0.0", &nic.sin_addr) <= 0) {
        close(newServSocket);
        throw std::runtime_error("Failed to set listening ip " + std::to_string(port));
    }

    if (bind(newServSocket, reinterpret_cast<sockaddr*>(&nic), sizeof(nic)) == -1) {
        close(newServSocket);
        throw std::runtime_error("Failed to bind socket with port " + std::to_string(port));
    }

    if (listen(newServSocket, 10) == -1) {
        close(newServSocket);
        throw std::runtime_error("Failed to listen on socket binded to port : " + std::to_string(port));
    }

    if (fcntl(newServSocket, F_SETFL, O_NONBLOCK) == -1) {
        close(newServSocket);
        throw std::runtime_error("Failed to make the socket non Blocking" + std::to_string(port));
    }

    struct pollfd pfd;
    pfd.fd = newServSocket;
    pfd.events = POLLIN; // what are all possible events
    pfd.revents = 0;
    
    listenSockets.push_back(newServSocket);
    pollFds.push_back(pfd);
}

// what is TX in kernel when send ?