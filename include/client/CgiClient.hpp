# pragma once

# include <string>
# include <sstream>
# include <iostream>
# include "../config/Parser.hpp"
# include "../request/HttpRequest.hpp"
# include "../response/HttpResponse.hpp"
# include "../cgi/Cgi.hpp"
# include <poll.h>

// new CgiClient update : client fd lookup each time instead of references to it's attributes .
class CgiClient
{
public:
    bool            is_write_end;
    int             http_client_fd;
    Cgi*            cgi;
    bool            processed;

    // http_client lookup ;
    // pollFds lookup

    // struct pollfd&  pfd; // DANGER : client pfd ref invalid, pollFds vector may reallocate for new clients . 
    

    // std::string&    response; // DANGER : client response , Clients map may reallocate for new Clients .

    CgiClient(int client_fd, Cgi* cgi, bool is_write) : is_write_end(is_write), http_client_fd(client_fd), cgi(cgi), processed(false) {
        std::cout << "CgiClient constructor called for  : " << this << " " <<  client_fd << " on cgi pipe : " << cgi->script_out[0] << std::endl;
    }

    ~CgiClient()
    {
        std::cout << "CgiClient destructor called for client : " << this << std::endl;
        // delete cgi;
    }

    // 
    
};