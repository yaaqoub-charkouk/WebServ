# pragma once



# include "CgiClient.hpp"

#include <algorithm>


enum ParseState {
    READING_HEADERS,
    READING_BODY,
    COMPLETE,
    ERROR
};


class Client // needs config file .
{
public:
    // server config
    int                 http_fd; // UPDATE

    // struct pollfd&      pfd; // client now has pfd. // DANGER 
    
    const ServerConfig& serverConfig;
    int                 clientPort;
    std::string         clientAddress;
    
    // request parsing
    std::string request_str;
    HttpRequest request;
    ParseState  state;
    size_t      header_end_pos;
    std::string header_str;
    
    // cgi 
    bool                isCgi;
    bool                isCgiResponseError;
    Cgi*                cgi;

    
    // response
    std::string         response_str;
    HttpResponse        response;
    size_t              bytes_sent;
    // needs response buffering , send()
    
    Client(const ServerConfig& serverConfig, int clientPort, std::string clientAddress);
    ~Client();
    // Client& operator=(const Client& newClient);
    void    parseRequest();
    void    parseRequestHeaders();
    void    parseRequestBody();
};

std::string trim(const std::string& str);