# pragma once

# include <string>
# include <sstream>
# include <iostream>
# include "../config/Parser.hpp"
# include "../request/HttpRequest.hpp"
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
    const ServerConfig& serverConfig;
    int                 clientPort;
    std::string         clientAddress;
    
    // request parsing
    std::string request_str;
    HttpRequest request;
    ParseState  state;
    size_t      header_end_pos;
    std::string header_str;

    std::string         response_str;
    // needs response buffering , send()
    
    Client(const ServerConfig& serverConfig, int clientPort, std::string clientAddress);
    // Client& operator=(const Client& newClient);
    void    parseRequest();
    void    parseRequestHeaders();
    void    parseRequestBody();
};

std::string trim(const std::string& str);