# include "../../include/server/server.hpp"


void    Server::acceptClient(int serverFd)
{
    while (true)
    {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        
        memset(&client, 0, len);

        int client_fd = accept(serverFd, reinterpret_cast<sockaddr*>(&client), &len);
        if (client_fd == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return ;
            throw std::runtime_error("Failed to add new client accept failed");
        }

        if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
        {
            sendErrorResponse(client_fd, serverFd, client, 500);
            continue ;
        }

        struct pollfd pfd;
        pfd.fd = client_fd;
        pfd.events = POLLIN;
        pfd.revents = 0;

        // add client to clients & pollFds
        pollFds.push_back(pfd);
        clients.insert(std::make_pair(client_fd, 
                        Client(configs[serverFd], ntohs(client.sin_port),
                        ipToString(client.sin_addr.s_addr))));
        

        std::cout << "============ NEW CLIENT ACCEPTED : CLIENT LOGS (fd = " << pfd.fd <<  ")============" << std::endl; // debugging
        std::cout << "Client ip: " << ipToString(client.sin_addr.s_addr) << std::endl;
        printf("Client port: %d\n", ntohs(client.sin_port)); // debugging

    }
}

void    Server::readFromClient(int  client_fd)
{
    std::cout << "read From client fd : " << client_fd << std::endl;

    char buffer[4096];

    if (clients.find(client_fd) == clients.end()) {
        std::cout << "LAHWAAA !!!!! reading from client ,, fd : " << client_fd << " does not exist" << std::endl; // debugging
        return ;
    }

    Client&  client = clients.at(client_fd);
    if (client.isCgi)
    {
        std::cout << "client reading request and he is already cgi " << client_fd << std::endl; // debugging
    }
    while (true)
    {
        int n = recv(client_fd, buffer, sizeof(buffer), 0);
        if (n > 0)
        {
            client.request_str.append(buffer, n);
            client.parseRequest();

            // check client.state
            if (client.state == COMPLETE)
            {
                const LocationConfig* location = RequestHandler::findLocation(client.request.uri, client.serverConfig);

                if ((client.request.method == "GET" || client.request.method == "POST") &&
                                RequestHandler::isCgiRequest(client.request.uri, location))
                {
                    std::cout << "  ===== CGI request ====" << std::endl;
                    RequestHandler::handleCgi(client, location);
                    
                    if (!client.isCgiResponseError)
                    {
                        client.cgi->execute();
                        
                        if (client.cgi->cgi_status == CGI_PIPE_ERROR || client.cgi->cgi_status == CGI_EXEC_ERROR || client.cgi->cgi_status == CGI_ENV_ERROR)
                        {
                            client.response = RequestHandler::makeErrorResponse(500, client.serverConfig);
                            client.response_str = client.response.getResponse();
                            changePollEvent(client_fd, POLLOUT);
                            return ;
                        }

                        if (client.request.method == "POST")
                            client.cgi->cgi_status = CGI_WRITING;
                        else //if (client.request.method == "GET")
                            client.cgi->cgi_status = CGI_READING;

                        try{
                            make_cgi_pipes_nonblocking(client.cgi->script_in[1], client.cgi->script_out[0]);
                        }
                        catch (...) {

                            client.cgi->closePipes();
                            kill(client.cgi->pid, SIGKILL);
                            waitpid(client.cgi->pid, NULL, WNOHANG);
                            client.response = RequestHandler::makeErrorResponse(500, client.serverConfig);
                            client.response_str = client.response.getResponse();
                            changePollEvent(client_fd, POLLOUT);
                            return ;
                        }
                        
                        add_cgi_pipes_to_pollFds(client.cgi->script_in[1], client.cgi->script_out[0]);

                        // associate cgi pipes to client & .
                        cgi_clients.insert(std::make_pair(client.cgi->script_out[0], CgiClient(client_fd, client.cgi, 0)));
                        if (client.cgi->script_in[1] != -1 || client.request.method == "POST")
                            cgi_clients.insert(std::make_pair(client.cgi->script_in[1], CgiClient(client_fd, client.cgi, 1)));
                        
                        client.isCgi = true;
                        return ;

                    }
                    else
                    {
                        client.response_str = client.response.getResponse();
                        changePollEvent(client_fd, POLLOUT);
                        return ;
                    }
                }

                client.response  = RequestHandler::handleRequest(client);
                // Cookies checking
                cookies.checkRequest(client.request);
                if (cookies.shouldSetCookie)
                    client.response.setHeaders(cookies.key, cookies.value);

                client.response_str = client.response.getResponse();
                changePollEvent(client_fd, POLLOUT);
                return ;
            }
            else if (client.state == ERROR) {

                client.response = RequestHandler::makeErrorResponse(client.error_code, client.serverConfig);
                client.response_str = client.response.getResponse();
                changePollEvent(client_fd, POLLOUT);
                return ;

            }
        }
        else if (n == 0)
        {
            closeClient(client_fd);
            return ;
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break ;
            closeClient(client_fd);
            return ;
        }
    }
}

void Server::writeToClient(int  client_fd)
{
    Client& client = clients.at(client_fd);

    if (client.bytes_sent >= client.response_str.size()) {
        closeClient(client_fd);
        return ;
    }

    size_t remaining = client.response_str.size() - client.bytes_sent;

    // size_t chunk_size = std::min(remaining, (size_t)10);

    ssize_t bytes_sent = send(client_fd,
                            client.response_str.c_str() + client.bytes_sent,
                            remaining,
                            0);

    if (bytes_sent > 0)
    {
        std::cout << "server sent " << bytes_sent << " bytes to client : " << client.clientPort << std::endl; // debugging

        client.bytes_sent += bytes_sent;

        std::cout << "still " << client.response_str.size() - client.bytes_sent << " bytes to send" << std::endl; // debugging

        if (client.bytes_sent == client.response_str.size())
            closeClient(client_fd);
    }
    else if (bytes_sent == 0)
        closeClient(client_fd);
    else if (bytes_sent < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return ;
        closeClient(client_fd);
    }
}

void    Server::sendErrorResponse(int client_fd, int serverFd, struct sockaddr_in client, int error_code)
{
    clients.insert(std::make_pair(client_fd, 
                        Client(configs[serverFd], ntohs(client.sin_port),
                        ipToString(client.sin_addr.s_addr))));
            
    Client&  http = clients.at(client_fd);

    http.response = RequestHandler::makeErrorResponse(error_code, http.serverConfig);
    http.response_str = http.response.getResponse();

    writeToClient(client_fd);
}

void Server::closeClient(int clientFd)
{
    Client& client = clients.at(clientFd);
    if (client.isCgi && client.cgi != NULL)
    {
        close_cgi_client(client.cgi->script_out[0]);
        close_cgi_client(client.cgi->script_in[1]);
    }

    close(clientFd);

    // remove from pollFds
    for (size_t i = 0; i < pollFds.size(); ++i)
    {
        if (pollFds[i].fd == clientFd)
        {
            pollFds.erase(pollFds.begin() + i);
            break;
        }
    }


    // remove client session
    clients.erase(clientFd);

    clientRemoved = true;
}

std::string ipToString(uint32_t addr)
{
    uint32_t ip = ntohl(addr);
    std::ostringstream oss;
    oss << ((ip >> 24) & 0xFF) << "."
        << ((ip >> 16) & 0xFF) << "."
        << ((ip >> 8) & 0xFF) << "."
        << (ip & 0xFF);
    return oss.str();
}