# include "../../include/server/server.hpp"


void    Server::acceptClient(int serverFd)
{
    // add a while (true) to accept multiple clients in one poll cycle
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

        // we're adding a new client . so make it non blocking & add it to pollFds;
        if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
        {
            close(client_fd);
            throw std::runtime_error("Failed to make client_fd non block");
        }

        struct pollfd pfd;
        pfd.fd = client_fd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        pollFds.push_back(pfd);

        // clients[client_fd] = Client(configs[serverFd]);
        clients.insert(std::make_pair(client_fd, Client(configs[serverFd], pfd,
                     ntohs(client.sin_port), inet_ntoa(client.sin_addr))));


        // just for debugging :
        // printf("Client IP: %s\n", inet_ntoa(client.sin_addr));
        std::cout << "============ NEW CLIENT ACCEPTED : CLIENT LOGS (fd = " << pfd.fd <<  ")============" << std::endl;
        printf("Client port: %d\n", ntohs(client.sin_port));

    }
}

// if (pollFds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
// {
//     closeClient(pollFds[i].fd);
//     continue;
// }
void    Server::readFromClient(int  client_fd)
{
    std::cout << "read From client fd : " << client_fd << std::endl;

    char buffer[4096];

    if (clients.find(client_fd) == clients.end()) {
        std::cout << "LAHWAAA !!!!! reading from client ,, fd : " << client_fd << " does not exist" << std::endl;
        return ;
    }

    Client&  client = clients.at(client_fd); // !!!! there is no client when POLLIN on CGI pipe
    if (client.isCgi)
    {
        std::cout << "client reading request and he is already cgi " << client_fd << std::endl;
        // set pfd.event to not checked event;
        // exit(1);
        // return ;
    }
    std::cout << "++++++++++++++++++++++++++++" << std::endl;
    while (true)
    {
        int n = recv(client_fd, buffer, sizeof(buffer), 0);
        // is cgi done reading
        // go to client waiting for cgi response
        if (n > 0)
        {
            client.request_str.append(buffer, n);
            // std::cout << client.request_str << std::endl;
            client.parseRequest();

            // check client.state
            if (client.state == COMPLETE) { // call request handler

                const LocationConfig* location = RequestHandler::findLocation(client.request.uri, client.serverConfig);

                if ((client.request.method == "GET" || client.request.method == "POST") &&
                                RequestHandler::isCgiRequest(client.request.uri, location))
                {
                    client.isCgi = true;
                    std::cout << "  ===== CGI request ====" << std::endl;
                    // call the handleCgi method ;
                    RequestHandler::handleCgi(client, location);
                    
                    // if cgi_response_error { }
                    if (!client.isCgiResponseError) // cgi code started correctly , there is a child process running there
                    {
                        // init cgi 
                        client.cgi->execute();
                        if (client.request.method == "POST")
                            client.cgi->cgi_status = CGI_WRITING;
                        else //if (client.request.method == "GET")
                            client.cgi->cgi_status = CGI_READING;


                        // make cgi pipes non blocking
                        // add cgi pipes to pollFds
                        make_cgi_pipes_nonblocking(client.cgi->script_in[1], client.cgi->script_out[0]);
                        add_cgi_pipes_to_pollFds(client.cgi->script_in[1], client.cgi->script_out[0]);
                        
                        // associate cgi pipes to client & .
                        // cgi_clients[client.cgi->script_out[0]] = CgiClient(pfd, client.cgi, client.response_str); // pfd is for the client who received the request
                        std::cout << "new cgi client fd : " << client_fd << std::endl;
                        cgi_clients.insert(std::make_pair(client.cgi->script_out[0], CgiClient(client_fd, client.cgi)));
                        std::cout << "+++++++++++++++++++++++++++++++++" << std::endl;
                        return ;

                    }
                }
                        // return handleCgi(client.request.method, client.request.uri, client.request.body, server, location);


                client.response  = RequestHandler::handleRequest(client);

                client.response_str = client.response.getResponse();

                // call changePollEvent instead ;
                changePollEvent(client_fd, POLLOUT);

                // pfd.events = POLLOUT; // DANGER ! invalid reference
                // pfd.revents = 0;
                std::cout << "completed request and pollout ready" << std::endl;
            }
            else if (client.state == ERROR) {
                // send erorr page
                std::cerr << "request parse error " << std::endl;
                





                // handle the error case
                }
            //}
        }
        else if (n == 0) // client closed connection
        {
            if (client.isCgi)
            {
                std::cout << "-- should stop cgi execution " << std::endl;
                std::cout << "-- closing a cgi client -- " << std::endl;
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                close_cgi_client(client.cgi->script_out[0]);


            }
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

void Server::writeToClient(int  client_fd) // DANGER : best practice to take 
{
    std::cout << "====> WRITING TO CLIENT : " << client_fd << std::endl;
    Client& client = clients.at(client_fd);

    if (client.bytes_sent >= client.response_str.size()) {
        closeClient(client_fd);
        return ;
    }

    // std::cout << "====> RESPONSE: " << client.response_str << std::endl;

    size_t remaining = client.response_str.size() - client.bytes_sent;

    // FORCE partial send (for testing)
    // size_t chunk_size = std::min(remaining, (size_t)10); // send only 10 bytes max

    ssize_t bytes_sent = send(client_fd,
                            client.response_str.c_str() + client.bytes_sent,
                            remaining,
                            0);

    if (bytes_sent > 0)
    {
        std::cout << "server sent " << bytes_sent << " bytes to client : " << client.clientPort << std::endl;

        client.bytes_sent += bytes_sent;

        std::cout << "still " << client.response_str.size() - client.bytes_sent << " bytes to send" << std::endl;
        std::cout << std::endl;

        if (client.bytes_sent == client.response_str.size())
            closeClient(client_fd);
    }
    else if (bytes_sent == 0) // connection closed
        closeClient(client_fd);
    else if (bytes_sent < 0) // error case
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return ;
        closeClient(client_fd);
    }








    // fine for now . but i still need to implement partial send logic !!!

    






    // hardcoded write to client for now . wait until adnane build response 

    // std::ifstream file("index.html");

    // if (!file.is_open())
    // {
    //     std::cerr << "Failed to open file\n";
    //     throw std::runtime_error("Failed to open index.html");
    // }
    // std::stringstream buffer_stream;
    // buffer_stream << file.rdbuf();
    // std::string body = buffer_stream.str();

    
    
    // std::stringstream response;
    // response << "HTTP/1.1 200 OK\r\n";
    // response << "Content-Type: text/html\r\n";
    // response << "Content-Length: " << body.size() << "\r\n";
    // response << "Connection: close\r\n";
    // response << "\r\n";
    // response << body;


    // std::string response_str = response.str();
    
    // send(pfd.fd, response_str.c_str(), response_str.size(), 0);

    // maybe i'll keep the client alive since the browser can use only one tcp three way handshake 

    // closeClient(pfd.fd);
}

void Server::closeClient(int clientFd)
{
    std::cout << "closeClient called" << std::endl;
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

//                         The Invisible Copies (Before the Semicolon)
// CgiClient(...)
// You explicitly create the first object.
// (Prints Constructor: 0x...0060)
// std::make_pair(...)
// make_pair takes your object and copies it into a temporary std::pair<int, CgiClient>.
// (Silent copy construction: 0x...00a8)
// The sneaky const conversion
// std::map::insert strictly requires a std::pair<const int, CgiClient>. Notice the const! Because make_pair didn't have const int, C++ is forced to create a third temporary pair to convert it. It copies the object again!
// (Silent copy construction: 0x...00e8)
// Inserting into the Map
// The map takes that converted pair and copies it one final time into the permanent map node.
// (Silent copy construction: 0x...0888)