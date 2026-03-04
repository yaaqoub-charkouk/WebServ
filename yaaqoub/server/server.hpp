#pragma once



# include <vector>
# include <map>
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



class Client;



class Server
{
private:
    std::vector<int>                listenSockets;
    std::map<int, Client>           clients;
    std::vector<struct pollfd>      pollFds;

private:
    void    acceptClient(int listenFd); // session creation
    void    readFromClient(int clientFd);
    void    writeToClient(int clientFd);
    void    closeClient(int clientFd);
    bool    Server::isListeningSocket(int fd)

public:
    void    addListeningSocket(int port); // setup
    void    run();

    // APIs
    std::string getRequest(int clientFd);
    void        sendResponse(int clientFd, const std::string& data);


public:
    Server();
    Server(const Server& other);
    Server& operator=(const Server& other);
    ~Server();
};


// this class is the server machine 
// the server socket is the NIC 

// what is : event-driven networking ?