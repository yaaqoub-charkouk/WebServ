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
        std::cout << "new poll cycle " << std::endl;
        for (size_t i = 0; i < pollFds.size();)
        {
            
            if (pollFds[i].revents == 0) {
                ++i;
                continue ; // just to optimise ignore sockets with no events .
            }
            clientRemoved = false;

            // if (pollFds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
            //     closeSocket(pollFds[i].fd); // exclude cgi pipe
            //     continue ;
            // }
            if (pollFds[i].revents & POLLIN)
            {
                if (isListeningSocket(pollFds[i].fd))
                    acceptClient(pollFds[i].fd);
                else if (isCgiPipe(pollFds[i].fd))
                {
                    processCgiReadEvent(pollFds[i]);
                }
                else
                {
                    readFromClient(pollFds[i]);
                }
            }
            // std::cout << "client Removed " << clientRemoved << std::endl;
            // std::cout << "write condition : " << (!clientRemoved && pollFds[i].revents & POLLOUT) << std::endl;
            if (!clientRemoved && (pollFds[i].revents & POLLOUT))
            {
                //  std::cout << "write to client  poll size :" << pollFds.size() << std::endl
                //     << "    i : " << i << std::endl;
                writeToClient(pollFds[i]);
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
    else if (isCgiPipe(fd))
        cgi_clients.erase(fd);
    else // add if for cgi.
        clients.erase(fd);

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

// ====== cgi ========

void    Server::make_cgi_pipes_nonblocking(int script_in, int script_out)
{
    std::cout << "making cgi pipe nonblcoking " << script_out << std::endl;
    if (fcntl(script_out, F_SETFL, O_NONBLOCK) == -1) {
        close(script_out);
        throw std::runtime_error("Failed to make the client cgi script_out nonblocking");
    }




    (void)script_in;
    // if (script_in != -1 && fcntl(script_in, F_SETFL, O_NONBLOCK) == -1) {
    //     close(script_in);
    //     throw std::runtime_error("Failed to make the client cgi script_in nonblocking");
    // }
}

void    Server::add_cgi_pipes_to_pollFds(int script_in, int script_out)
{
    // add cgi pipe to pollFds;
    struct pollfd pfd;
    pfd.fd = script_out;
    pfd.events = POLLIN;
    pfd.revents = 0;
    pollFds.push_back(pfd);

    (void)script_in;
}

void    Server::processCgiReadEvent(struct pollfd& pfd)
{
    std::cout << "processCgiReadEvent" << std::endl;

    CgiClient& cgi_client = cgi_clients.at(pfd.fd);
    cgi_client.cgi->read_output(); // !!
    if (cgi_client.cgi->cgi_status == CGI_DONE_READING)
    {
        cgi_client.cgi->build_response();
        cgi_client.response = cgi_client.cgi->getResponse();

        cgi_client.pfd.events = POLLOUT; // client
        cgi_client.pfd.revents = 0;
        std::cout << "file descriptor : " << cgi_client.pfd.fd << "setten to POLLOUT" << std::endl;


        // remove cgi pipe from poll;
        for (size_t i = 0; i < pollFds.size(); ++i)
        {
            if (pollFds[i].fd == pfd.fd)
            {
                pollFds.erase(pollFds.begin() + i);
                break ;
            }
        }
        cgi_clients.erase(pfd.fd);
    }
}

// int main(void)
// {
//     Server server;

//     server.addListeningSocket(8080);
//     // server.addListeningSocket(1337);
//     server.run();
// }

bool    Server::isCgiPipe(int fd)
{
    return (cgi_clients.find(fd) != cgi_clients.end());
}