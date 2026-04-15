# pragma once

# include <string>
# include <sstream>
# include <iostream>
# include "../config/Parser.hpp"
# include "../request/HttpRequest.hpp"

enum ParseState {
    READING_HEADERS,
    READING_BODY,
    INCOMPLETE,
    COMPLETE,
    ERROR
};


class Client // needs config file .
{
public:
    

    std::string response_str;
    
    // request parsing
    std::string request_str;
    HttpRequest request;
    ParseState  state;
    size_t      header_end_pos;
    std::string header_str;

    Client();
    void    parseRequest();
};

std::string trim(const std::string& str);