# include "../../include/server/server.hpp"



Server::Server(const std::vector<ServerConfig>& servers)
{
    for (size_t i = 0; i < servers.size(); ++i)
    {
        int newServerSocket = addListeningSocket(servers[i].getPort());
        // configs[newServerSocket] = servers[i];
        configs.insert(std::make_pair(newServerSocket, servers[i])); // may need to check if exists

        std::cout << "listening on : " <<  servers[i].getPort() << "  fd :" << newServerSocket << std::endl; // for debugging
    }
    clientRemoved = false;
}




static std::string intToString(int value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

int    Server::addListeningSocket(int port)
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

    return (newServSocket);
}


void    Server::run()
{
    while (1337)
    {
        int ret = poll(pollFds.data(), pollFds.size(), -1);

        if (ret < 0)
            throw std::runtime_error("poll failed can't listen on servers sockets");
        std::cout << "------------new poll cycle -----------------" << std::endl;
        std::cout << "pollFds size : " << pollFds.size() << std::endl;
        for (size_t i = 0; i < pollFds.size();)
        {
            
            if (pollFds[i].revents == 0) {
                ++i;
                continue ; // just to optimise ignore sockets with no events .
            }
            clientRemoved = false;

            if (pollFds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                if (isCgiPipe(pollFds[i].fd)) {

                    // pollFds[i].revents = 0; // why 
                    // pollFds[i].events = POLLIN; DANGER
                    if (pollFds[i].revents & POLLIN) // Added the POLLHUP so we read even if the child closed the pipe
                        processCgiReadEvent(pollFds[i].fd); // DANGER : invalid reference , allocating new client while processing another cgi_client
                    else if (pollFds[i].revents & POLLOUT)
                        processCgiWriteEvent(pollFds[i].fd);

                    std::cout << "cgi pipe got POLLHUP" << std::endl;
                    // continue;
                }
                else {
                    closeSocket(pollFds[i].fd); // exclude cgi pipe
                    continue ;
                }
            }
            if (!clientRemoved && pollFds[i].revents & POLLIN)
            {
                if (isListeningSocket(pollFds[i].fd))
                    acceptClient(pollFds[i].fd);
                else if (isCgiPipe(pollFds[i].fd))
                {
                    processCgiReadEvent(pollFds[i].fd); // DANGER : invalid reference , allocate new client while processing another cgi_client
                }
                else
                {
                    readFromClient(pollFds[i].fd); // DANGER : invalid reference , new cgi pipes may be added to pollFds , vector may reallocate !
                    std::cout << "------ after reading request from a new client ---- " << std::endl;
                }
            }
            // std::cout << "client Removed " << clientRemoved << std::endl;
            // std::cout << "write condition : " << (!clientRemoved && pollFds[i].revents & POLLOUT) << std::endl;
            if (!clientRemoved && (pollFds[i].revents & POLLOUT))
            {
                if (isCgiPipe(pollFds[i].fd))
                    processCgiWriteEvent(pollFds[i].fd);
                else
                    writeToClient(pollFds[i].fd);

                //  std::cout << "write to client  poll size :" << pollFds.size() << std::endl
                //     << "    i : " << i << std::endl;
            }


            if (!clientRemoved)
                ++i;
        }
    }
}


bool Server::isListeningSocket(int fd)
{
    return std::find(listenSockets.begin(),
                     listenSockets.end(),
                     fd) != listenSockets.end();
}

void    Server::closeSocket(int fd)
{
    std::cout << "closeSocket called on " << fd << std::endl;
    if (isListeningSocket(fd)) // if serverSocket remove it from listenSocket && configs.
    {
        configs.erase(fd);

        for (size_t i = 0; i < listenSockets.size(); ++i)
        {
            if (listenSockets[i] == fd)
            {
                listenSockets.erase(listenSockets.begin() + i);
                break;
            }
        }
    }
    // else if (isCgiPipe(fd)) {
    //     cgi_clients.erase(fd);
        
    // }
    else // add if for cgi.
    {
        Client& client = clients.at(fd);
        if (client.isCgi)
        {
            close_cgi_client(client.cgi->script_out[0]); // DANGER : the method closes cgi fd
            // close the other cgi pipe . script_in[1]
            close_cgi_client(client.cgi->script_in[1]);
        }
        clients.erase(fd);
    }

    close(fd); // cgi pipe may be closed at cgi.

    // remove from pollFds
    for (size_t i = 0; i < pollFds.size(); ++i)
    {
        if (pollFds[i].fd == fd)
        {
            pollFds.erase(pollFds.begin() + i);
            break;
        }
    }

}


// pollFds lookup to change event.
void    Server::changePollEvent(int fd, int event)
{
    for (size_t i = 0; i < pollFds.size(); ++i)
    {
        if (pollFds[i].fd == fd)
        {
            pollFds[i].revents = 0;
            pollFds[i].events = event;
            break ;
        }
    }
}

void    Server::setHttpClientResponse(Cgi& cgi, int http_client_fd)
{
    std::cout << "setting response to : " << http_client_fd << std::endl;
    // Cookies checking
    // cookies.checkRequest(clients.at(http_client_fd).request);
    // if (cookies.shouldSetCookie)
    //     cgi.res.setHeaders(cookies.key, cookies.value);

    clients.at(http_client_fd).response_str = cgi.getResponse();
}
