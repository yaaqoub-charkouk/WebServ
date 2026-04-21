#pragma once



# include <vector>
# include <map>
# include <algorithm>
# include <string>
# include <cstring>
# include <unistd.h>
# include <fstream>
# include <sstream>
# include <fcntl.h>
# include <iostream>
# include <cerrno>

# include <poll.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>

# include "../config/Parser.hpp"
# include "../request/HttpRequest.hpp"
# include "../request/RequestHandler.hpp"
# include "../client/Client.hpp"


class Server
{
private:
    std::vector<int>                    listenSockets;
    std::map<int, const ServerConfig>   configs;
    std::map<int, Client>               clients;
    std::vector<struct pollfd>          pollFds;

    bool    clientRemoved;

private:
    void    acceptClient(int serverFd); // session creation
    void    readFromClient(struct pollfd& pfd);
    void    writeToClient(struct pollfd& pfd);
    void    closeClient(int clientFd);
    bool    isListeningSocket(int fd);
    void    closeSocket(int fd);

    int     addListeningSocket(int port); // setup
    
    void    make_cgi_pipes_nonblocking(int script_in, int script_out);
    void    add_cgi_pipes_to_pollFds(int script_in, int script_out);
    
public:
    void    run();

    // APIs
    std::string getRequest(int clientFd);
    void        sendResponse(int clientFd, const std::string& data);

    

public:
    Server(const std::vector<ServerConfig>& servers);
//     Server();
//     Server(const Server& other);
//     Server& operator=(const Server& other);
//     ~Server();
};


// this class is the server machine 
// the server socket is the NIC 