# include "server.hpp"
#include <cstring>
#include <iterator>
#include <netinet/in.h>
#include <stdexcept>
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

    // now i have all servers sockets & poll in 
}


void    Server::run()
{
    while (1337)
    {
        int ret = poll(pollFds.data(), pollFds.size(), -1);
    
        if (ret < 0)
            throw std::runtime_error("poll failed can't listen on servers sockets");
        
        // std::vector<struct pollfd>::iterator it = pollFds.begin();
        // std::vector<struct pollfd>::iterator end = pollFds.end();
    
        for (size_t i = 0; i < pollFds.size(); ++i)
        {
            if (pollFds[i].revents & POLLIN)
            {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                
                memset(&client, 0, len);
                                
                if (isListeningSocket(pollFds[i].fd))
                {
                    // accept a new client
                    // add the new client to pollFds 
                    int client_fd = accept(pollFds[i].fd, reinterpret_cast<sockaddr*>(&client), &len);
                    if (client_fd == -1)
                    {
                        
                        throw std::runtime_error("Failed to add new client accept failed");
                    }
                    
                    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
                    {
                        close(client_fd);
                        throw std::runtime_error("Failed to make client_fd non block");
                    }

                    struct pollfd pfd;
                    pfd.fd = client_fd;
                    pfd.events = POLLIN;
                    pfd.revents = 0;

                    pollFds.push_back(pfd);

                }
                else
                {
                    // receive from a client
                }


            }
            it++;
        }

    }


}

bool Server::isListeningSocket(int fd)
{
    return std::find(listenSockets.begin(),
                     listenSockets.end(),
                     fd) != listenSockets.end();
}