# include "../../include/server/server.hpp"
#include <sys/poll.h>



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
        clients.insert(std::make_pair(client_fd, Client(configs[serverFd],
                     ntohs(client.sin_port), inet_ntoa(client.sin_addr))));


        // just for debugging : 
        printf("Client IP: %s\n", inet_ntoa(client.sin_addr));
        printf("Client port: %d\n", ntohs(client.sin_port));

    }     
}

// if (pollFds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
// {
//     closeClient(pollFds[i].fd);
//     continue;
// }
void    Server::readFromClient(struct pollfd& pfd)
{
    char buffer[4096];

    while (true)
    {
        int n = recv(pfd.fd, buffer, sizeof(buffer), 0);

        if (n > 0)
        {
            Client& client = clients.at(pfd.fd);

            client.request_str.append(buffer, n);
            // std::cout << client.request_str << std::endl;
            client.parseRequest();

            // check client.state
            if (client.state == COMPLETE) { // call request handler
                client.response  = RequestHandler::handleRequest(client.request.method,
                                            client.request.uri,
                                            client.request.body,
                                            client.serverConfig,
                                            client.request.contentLength);
                
                pfd.events = POLLOUT;
                pfd.revents = 0;
                std::cout << "completed request and pollout ready" << std::endl;
            }



            // calling Request.parse() instead of server api ;
            // HttpRequest req = HttpRequest::parse(clients[pfd.fd].request_str);

            // need to check for end of request "\r\n\r\n" : Moved to Httprequest.parse()
            // if (client.request_str.find("\r\n\r\n", 0) != std::string::npos) // TAHALLA
            // {
            //     // pfd.events = POLLOUT; // !!! set it only when done with response . how? i don't know .
            //     // pfd.revents = 0;
            // } // keep it until http handler start getting requests
        }
        else if (n == 0) // client closed connection
        {
            closeClient(pfd.fd);
            return ;
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break ;
            closeClient(pfd.fd);
            return ;
        }
    }
}

void Server::writeToClient(struct pollfd& pfd)
{
    Client& client = clients.at(pfd.fd);
    client.response_str = client.response.getResponse();

    // fine for now . but i still need to implement partial send logic !!!
    send(pfd.fd, client.response_str.c_str(), client.response_str.size(), 0);

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
    closeClient(pfd.fd);
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
