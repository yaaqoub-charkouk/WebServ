# include "../../include/server/server.hpp"



void    Server::processCgiEvent(int cgi_pipe)
{
    CgiClient& cgi_client = cgi_clients.at(cgi_pipe);
    std::cout << "process cgi event : is write " << cgi_client.is_write_end << std::endl;

    if (cgi_client.is_write_end)
        processCgiWriteEvent(cgi_pipe);
    else
        processCgiReadEvent(cgi_pipe);
}


// ====== cgi ========

void    Server::make_cgi_pipes_nonblocking(int& script_in, int& script_out)
{
    std::cout << " making cgi pipe nonblcoking " << script_out << std::endl;
    if (fcntl(script_out, F_SETFL, O_NONBLOCK) == -1) {
        // close(script_out);
        // script_out = -1;
        // RETURN internat server error;
        throw std::runtime_error("Failed to make the client cgi script_out nonblocking");
    }




    // (void)script_in;
    std::cout << " making cgi pipe nonblcoking " << script_in << std::endl;
    if (script_in != -1 && fcntl(script_in, F_SETFL, O_NONBLOCK) == -1) {
        // close(script_in);
        // script_in = -1;
        // RETURN internal server error;
        throw std::runtime_error("Failed to make the client cgi script_in nonblocking");
    }
}

void    Server::add_cgi_pipes_to_pollFds(int script_in, int script_out)
{
    // add cgi pipe to pollFds;
    struct pollfd pfd;
    pfd.fd = script_out;
    pfd.events = POLLIN;
    pfd.revents = 0;
    pollFds.push_back(pfd);

    // (void)script_in;
    // No need to add it to pollFds since it s a GET request
    if (script_in != -1)
    {
        struct pollfd pfd_write;
        pfd_write.fd = script_in;
        pfd_write.events = POLLOUT;
        pfd_write.revents = 0;
        pollFds.push_back(pfd_write);
    }
}

void    Server::processCgiReadEvent(int cgi_pipe) // DANGER : reference may be invalid after vector reallocate !!!
{
    std::cout << "  processCgiReadEvent :  " << cgi_pipe << std::endl;

    CgiClient& cgi_client = cgi_clients.at(cgi_pipe);

    if (cgi_client.processed)
    {
        std::cout << "CGI ALREADY PROCESSED FOR CGI PIPE : " << cgi_pipe << std::endl;

        changePollEvent(cgi_pipe, POLLIN);
        return ; // set another poll event to cgi_pipe;
    }
    // if (cgi_client.cgi == NULL)
    //     return ;

    if (clients.find(cgi_client.http_client_fd) == clients.end())
    {
        std::cout << "DANGER : http client closed :" << cgi_client.http_client_fd << " for cgi : " << cgi_pipe << std::endl;

        // close_cgi_client(cgi_pipe);
        return ;
    }

    cgi_client.cgi->read_output(); // !!

    if (cgi_client.cgi->cgi_status == CGI_DONE_READING)
    {
        cgi_client.cgi->build_response();
        // cgi_client.response = cgi_client.cgi->getResponse();
        setHttpClientResponse(*cgi_client.cgi, cgi_client.http_client_fd);

        // http_client.pfd.events = POLLOUT
        changePollEvent(cgi_client.http_client_fd, POLLOUT);

        // cgi_client.pfd.events = POLLOUT; // client
        // cgi_client.pfd.revents = 0;

        // changePollEvent(cgi_pipe, POLLIN);
        std::cout << "CGI done , ready to write response to cgi_client.cgi_pipe " << cgi_pipe << std::endl;


        // close_cgi_client(cgi_pipe); // no need because client will close it ;
        // remove from poll
                    for (size_t i = 0; i < pollFds.size(); ++i)
                    {
                        if (pollFds[i].fd == cgi_pipe)
                        {
                            pollFds.erase(pollFds.begin() + i);
                            clientRemoved = true;
                            break ;
                        }
                    }

        // std::cout << "cgi pipe got removed from pollFds : " << cgi_pipe << std::endl;
    cgi_client.processed = true;
    
    }
}




void    Server::processCgiWriteEvent(int cgi_pipe) // DANGER : reference may be invalid after vector reallocate !!!
{
    std::cout << "  processCgiWriteEvent" << std::endl;

    CgiClient& cgi_client = cgi_clients.at(cgi_pipe);

    if (cgi_client.processed)
    {
        std::cout << "CGI ALREADY PROCESSED FOR CGI PIPE : " << cgi_pipe << std::endl;

        // changePollEvent(cgi_pipe, POLLOUT);
        return ; // set another poll event to cgi_pipe;
    }
    // if (cgi_client.cgi == NULL)
    //     return ;

    if (clients.find(cgi_client.http_client_fd) == clients.end())
    {
        std::cout << "http client closed :" << cgi_client.http_client_fd << " for cgi : " << cgi_pipe << std::endl;
        std::cout << "~~~~~~ cgi pipe is write : " << cgi_client.is_write_end << std::endl;

        // close_cgi_client(cgi_pipe);
        return ;
    }

    cgi_client.cgi->write_body(); // the method why i need to add this pipe to cgi_clients
    
    if (cgi_client.cgi->cgi_status == CGI_DONE_WRITING)
    {
        cgi_client.cgi->cgi_status = CGI_READING;
        cgi_client.processed = true;
        // remove from poll
                    for (size_t i = 0; i < pollFds.size(); ++i)
                    {
                        if (pollFds[i].fd == cgi_pipe)
                        {
                            pollFds.erase(pollFds.begin() + i);
                            clientRemoved = true;
                            // break ;
                        }
                    }
        
        close(cgi_client.cgi->script_in[1]); // closing the fd let pipe re use it again while we still have cgi_client entry with the same fd
        cgi_client.cgi->script_in[1] = -1;
        cgi_clients.erase(cgi_pipe);

        std::cout << "cgi done writing : " << cgi_pipe << std::endl;
        // exit(0);
        // std::cout << "cgi pipe got removed from pollFds : " << cgi_pipe << std::endl;
    
    }
}





void    Server::close_cgi_client(int fd)
{
    if (fd == -1) // of leaks this is the cause . check if script_in[1] was seted to -1 after cgi_client created
        return ;
    std::cout << "close cgi client " << fd << std::endl;
    CgiClient& cgi_client = cgi_clients.at(fd);
    std::cout << "close cgi client " << fd << std::endl;

    kill(cgi_client.cgi->pid, SIGKILL);
    waitpid(cgi_client.cgi->pid, NULL, WNOHANG);
    
    // delete cgi_client.cgi; // delete once at client . 
    // cgi_client.cgi = NULL;

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