# include "../../include/server/server.hpp"




Server::Server(const std::vector<ServerConfig>& servers)
{
    for (size_t i = 0; i < servers.size(); ++i)
    {
        int newServerSocket = addListeningSocket(servers[i].getPort());
        configs.insert(std::make_pair(newServerSocket, servers[i])); // may need to check if exists

        std::cout << "listening on : " <<  servers[i].getPort() << "  fd :" << newServerSocket << std::endl; // debugging
    }
    clientRemoved = false;
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
    pfd.events = POLLIN;
    pfd.revents = 0;

    // adding serverSocket to listenSockets & pollFds
    listenSockets.push_back(newServSocket);
    pollFds.push_back(pfd);

    return (newServSocket);
}

void    Server::run()
{
    while (1337)
    {
        int ret = poll(pollFds.data(), pollFds.size(), 1000);//LEHWAAAAAAA

        if (ret < 0)
            throw std::runtime_error("poll failed can't listen on servers sockets");

        std::cout << "------------new poll cycle -----------------" << std::endl; // debugging
        std::cout << "pollFds size : " << pollFds.size() << std::endl; // debugging

        for (size_t i = 0; i < pollFds.size();)
        {
            if (pollFds[i].revents == 0) {
                ++i;
                std::cout << "ignore sockets with no events " << std::endl; // debugging
                continue ;
            }
            clientRemoved = false;

            if (pollFds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) { // debugging : check if cgi pipe got POLLERR | POLLNVAL
                if (isCgiPipe(pollFds[i].fd)) {
                    std::cout << "=====----++++==== pollFds[i].fd : " << pollFds[i].fd  << std::endl; // debugging
                    if (pollFds[i].revents & POLLHUP)
                        processCgiEvent(pollFds[i].fd);
                    // cgi pipe may get POLLERR 

                    std::cout << "cgi pipe got POLLHUP" << std::endl; // debugging
                    // continue;
                }
                else {
                    closeSocket(pollFds[i].fd);
                    continue ;
                }
            }

            // check for client read event
            if (!clientRemoved && pollFds[i].revents & POLLIN)
            {
                if (isListeningSocket(pollFds[i].fd))
                    acceptClient(pollFds[i].fd);
                
                else if (isCgiPipe(pollFds[i].fd))
                    processCgiReadEvent(pollFds[i].fd);
                else
                    readFromClient(pollFds[i].fd);
            }

            // check for client write event
            if (!clientRemoved && (pollFds[i].revents & POLLOUT))
            {
                if (isCgiPipe(pollFds[i].fd))
                    processCgiWriteEvent(pollFds[i].fd);
                else
                    writeToClient(pollFds[i].fd);
            }

            
            if (!clientRemoved)
                ++i;
        }

        // CGI: timeout check
        std::map<int, Client>::iterator it;
        for (it = clients.begin(); it != clients.end(); ++it)
        {
            if (it->second.isCgi && it->second.cgi != NULL && it->second.cgi->checkTimeout())
            {
                it->second.response = RequestHandler::makeErrorResponse(504, it->second.serverConfig);
                it->second.response_str = it->second.response.getResponse();
                // Should we close the cgi pipes here? LEHWAAA : "no you should not . client close connection after he write response back to client and closes its cgi pipes" .
                changePollEvent(it->first, POLLOUT);

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

void    Server::closeSocket(int fd)
{
    std::cout << "closeSocket called on " << fd << std::endl; // debugging
    if (isListeningSocket(fd))
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
    else
    {
        if (clients.find(fd) == clients.end()) // debugging : may slow down the server tow lookups
            return ;
        Client& client = clients.at(fd);
        if (client.isCgi)
        {
            close_cgi_client(client.cgi->script_out[0]);
            close_cgi_client(client.cgi->script_in[1]);
        }
        clients.erase(fd);
    }

    close(fd);

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

void    Server::changePollEvent(int fd, short int event)
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
    // Cookies checking
    cookies.checkRequest(clients.at(http_client_fd).request);
    if (cookies.shouldSetCookie)
        cgi.res.setHeaders(cookies.key, cookies.value);

    clients.at(http_client_fd).response_str = cgi.getResponse();
}

std::string intToString(int value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}