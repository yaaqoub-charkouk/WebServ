#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

class HttpRequest
{
public:
    std::string method;
    std::string uri;
    std::string version; // if needed after !!
    size_t contentLength;
    std::string body;
    std::string clientAddr;
    std::string serverName;
    int serverPort;
    std::map<std::string, std::string> headers;

    // HttpRequest(): method("GET"), uri("/"), body(""), clientAddr(""), serverName(""), serverPort(0), contentLength(0) {}

};

#endif
// GET /index.html HTTP/1.1\r\n
// Host: localhost:8080\r\n
// User-Agent: Mozilla/5.0\r\n
// Accept: text/html\r\n
// Connection: close\r\n
// \r\n