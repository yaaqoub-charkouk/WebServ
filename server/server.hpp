#pragma once



# include <vector>
# include <map>
# include <algorithm>
# include <string>
# include <poll.h>
# include <cstring>

# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <fstream>
# include <sstream>
# include <fcntl.h>
# include <iostream>
# include <cerrno>

# include "../tahalla/include/Parser.hpp"


class Client
{
public:
    std::string request;
    std::string response;
};



class Server
{
private:
    std::vector<int>                listenSockets;
    std::map<int, Client>           clients;
    std::vector<struct pollfd>      pollFds;

private:
    void    acceptClient(int serverFd); // session creation
    void    readFromClient(struct pollfd& pfd);
    void    writeToClient(struct pollfd& pfd);
    void    closeClient(int clientFd);
    bool    isListeningSocket(int fd);

public:
    Server(const std::vector<ServerConfig>& servers);
    void    addListeningSocket(int port); // setup
    void    run();

    // APIs
    std::string getRequest(int clientFd);
    void        sendResponse(int clientFd, const std::string& data);


// public:
//     Server();
//     Server(const Server& other);
//     Server& operator=(const Server& other);
//     ~Server();
};


// this class is the server machine 
// the server socket is the NIC 