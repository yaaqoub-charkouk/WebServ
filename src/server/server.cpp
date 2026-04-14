# include "../../include/server/server.hpp"



Server::Server(const std::vector<ServerConfig>& servers)
{
    for (size_t i = 0; i < servers.size(); ++i)
    {
        addListeningSocket(servers[i].getPort());
        std::cout << "listening on : " <<  servers[i].getPort() << std::endl;
    }
}




static std::string intToString(int value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

void    Server::addListeningSocket(int port)
{
    int newServSocket;

    newServSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (newServSocket < 0)
        throw std::runtime_error("Failed to create listening socket port : " + intToString(port));
    
    int opt = 1;
    if (setsockopt(newServSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        close(newServSocket);
        throw std::runtime_error("Failed to set socket options " + intToString(port));
    }

    struct sockaddr_in nic;
    memset(&nic, 0, sizeof(nic));
    nic.sin_family = AF_INET;
    nic.sin_port = htons(port);
    if (inet_pton(nic.sin_family, "0.0.0.0", &nic.sin_addr) <= 0) {
        close(newServSocket);
        throw std::runtime_error("Failed to set listening ip " + intToString(port));
    }

    if (bind(newServSocket, reinterpret_cast<sockaddr*>(&nic), sizeof(nic)) == -1) {
        close(newServSocket);
        throw std::runtime_error("Failed to bind socket with port " + intToString(port));
    }

    if (listen(newServSocket, SOMAXCONN) == -1) {
        close(newServSocket);
        throw std::runtime_error("Failed to listen on socket binded to port : " + intToString(port));
    }



    if (fcntl(newServSocket, F_SETFL, O_NONBLOCK) == -1) {
        close(newServSocket);
        throw std::runtime_error("Failed to make the socket non Blocking" + intToString(port));
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
    
        for (size_t i = 0; i < pollFds.size(); ++i)
        {
            if (pollFds[i].revents == 0)
                continue ; // just to optimise ignore sockets with no events . 
            if (pollFds[i].revents & POLLIN)
            {                                
                if (isListeningSocket(pollFds[i].fd))
                    acceptClient(pollFds[i].fd);
                else
                {
                    readFromClient(pollFds[i]);
                    
                    // call the http handler TAHALLA
                }
            }
            if (pollFds[i].revents & POLLOUT)
            {
                writeToClient(pollFds[i]);
            }
        }
    }
}


bool Server::isListeningSocket(int fd)
{
    return std::find(listenSockets.begin(),
                     listenSockets.end(),
                     fd) != listenSockets.end();
}

// int main(void)
// {
//     Server server;

//     server.addListeningSocket(8080);
//     // server.addListeningSocket(1337);
//     server.run();
// }