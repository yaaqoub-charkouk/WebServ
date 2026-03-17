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

# include "tahalla/include/ServerConfig.hpp"
# include "srcs/HttpRequest.hpp"

class Client
{
public:
    std::string request;
    std::string response;
    int         listenPort;
    bool        rawCgiResponse; // response is already a full HTTP response from CGI

    Client() : request(""), response(""), listenPort(0), rawCgiResponse(false) {}
};


class Server
{
private:
    std::vector<struct pollfd>                    pollFds;
    std::vector<int>                              listenSockets;
    std::map<int, Client>                         clients;
    std::map<int, int>                            listenFdToPort;
    std::map<int, std::vector<ServerConfig> >     portToConfigs;

private:
    void acceptClient(int serverFd);
    void readFromClient(struct pollfd& pfd);
    void writeToClient(struct pollfd& pfd);
    void closeClient(int clientFd);
    bool isListeningSocket(int fd);
    void processRequest(struct pollfd& pfd);

    const ServerConfig& findServerConfig(int port, const std::string& host) const;

public:
    void addListeningSocket(int port, const std::vector<ServerConfig>& configs);
    void run();

    // APIs (kept for compatibility)
    std::string getRequest(int clientFd);
    void        sendResponse(int clientFd, const std::string& data);
};
 