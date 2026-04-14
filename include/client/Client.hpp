# pragma once

# include <string>
# include "../config/Parser.hpp"
# include "../request/HttpRequest.hpp"


enum ParseState {
    READING_HEADERS,
    READING_BODY,
    COMPLETE,
    ERROR
};


class Client
{
public:
    std::string response_str;
    

    HttpRequest request;
    std::string request_str;
    ParseState  state;


    Client();
    void    parseRequest();
};
