#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

class HttpRequest
{
public:
    std::string method;
    std::string uri;
    std::string body;
    std::string clientAddr;
    std::string serverName;
    int serverPort;
    std::map<std::string, std::string> headers;
    size_t contentLength;

    HttpRequest(): method("GET"), uri("/"), body(""), clientAddr(""), serverName(""), serverPort(0), contentLength(0) {}

    static HttpRequest parse(const std::string& rawRequest); // must do by yaaqoub
};

#endif
