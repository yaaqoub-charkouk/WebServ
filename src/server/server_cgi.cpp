# include "../../include/server/server.hpp"



// ====== cgi ========

void    Server::make_cgi_pipes_nonblocking(int script_in, int script_out)
{
    std::cout << " making cgi pipe nonblcoking " << script_out << std::endl;
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

void    Server::processCgiReadEvent(struct pollfd& pfd) // cgi pipe 
{
    std::cout << "  processCgiReadEvent" << std::endl;

    CgiClient& cgi_client = cgi_clients.at(pfd.fd);

    cgi_client.cgi->read_output(); // !!

    if (cgi_client.cgi->cgi_status == CGI_DONE_READING)
    {
        cgi_client.cgi->build_response();
        cgi_client.response = cgi_client.cgi->getResponse();
        
        cgi_client.pfd.events = POLLOUT; // client
        cgi_client.pfd.revents = 0;
        
        std::cout << "CGI done , ready to write response to cgi_client.pfd.fd " << cgi_client.pfd.fd << std::endl;
        
        
        // remove cgi pipe from poll;
        close_cgi_client(pfd.fd); // code another one that takes cgi_client;
        std::cout << "cgi pipe got removed from pollFds : " << pfd.fd << std::endl;
    }
}

void    Server::close_cgi_client(int fd)
{
    CgiClient& cgi_client = cgi_clients.at(fd);
    delete cgi_client.cgi;

    std::cout << "closing cgi_client ---> " << fd << std::endl;

    close(fd);

    // remove from poll
    for (size_t i = 0; i < pollFds.size(); ++i)
    {
        if (pollFds[i].fd == fd)
        {
            pollFds.erase(pollFds.begin() + i);
            break ;
        }
    }

    // remove from cgi_clients map
    cgi_clients.erase(fd);
    
    clientRemoved = true;
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