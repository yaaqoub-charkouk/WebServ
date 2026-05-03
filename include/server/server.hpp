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
# include "../cookie/Cookie.hpp"




class Server
{
private:
    std::vector<int>                    listenSockets;
    std::map<int, const ServerConfig>   configs;
    std::map<int, Client>               clients;
    std::map<int, CgiClient>            cgi_clients;
    std::vector<struct pollfd>          pollFds;

    bool    clientRemoved;

    Cookie  cookies;

private:
    void    acceptClient(int serverFd); // session creation
    void    readFromClient(int client_fd);
    void    writeToClient(int client_fd);
    void    closeClient(int clientFd);
    bool    isListeningSocket(int fd);
    void    closeSocket(int fd);

    void    changePollEvent(int fd, int event);
    
    int     addListeningSocket(int port); // setup

    // CGI:
    void    make_cgi_pipes_nonblocking(int& script_in, int& script_out);
    void    add_cgi_pipes_to_pollFds(int script_in, int script_out);
    bool    isCgiPipe(int fd);
    void    processCgiEvent(int cgi_pipe);
    void    processCgiReadEvent(int   cgi_pipe);
    void    processCgiWriteEvent(int cgi_pipe);
    void    close_cgi_client(int cgi_pipe_fd);
    void    setHttpClientResponse(Cgi& cgi, int http_client_fd);

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