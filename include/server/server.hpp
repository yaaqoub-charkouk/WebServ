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

class CgiClient
{
public:
    struct pollfd&  pfd;
    Cgi*            cgi;
    std::string&    response;

    CgiClient(struct pollfd& pfd, Cgi* cgi, std::string& response) : pfd(pfd), cgi(cgi), response(response) {
        std::cout << "CgiClient constructor called for  : " << this << " " <<  pfd.fd << " on cgi pipe : " << cgi->script_out[0] << std::endl;
    }

    ~CgiClient() {
        std::cout << "CgiClient destructor called for client : " << this << std::endl;
        // delete cgi;
    }
};

class Server
{
private:
    std::vector<int>                    listenSockets;
    std::map<int, const ServerConfig>   configs;
    std::map<int, Client>               clients;
    std::map<int, CgiClient>            cgi_clients;
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

    // CGI:
    void    make_cgi_pipes_nonblocking(int script_in, int script_out);
    void    add_cgi_pipes_to_pollFds(int script_in, int script_out);
    bool    isCgiPipe(int fd);
    void    processCgiReadEvent(struct pollfd& pfd);
    void    close_cgi_client(int fd);
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