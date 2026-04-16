#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <string>
#include <vector>
#include <map>
#include "../config/ServerConfig.hpp"
#include "../config/LocationConfig.hpp"
#include "../response/HttpResponse.hpp"

class RequestHandler
{
public:
    static HttpResponse handleRequest(
        const std::string& method,
        const std::string& uri,
        const std::string& body,
        const ServerConfig& server,
        size_t contentLength);

private:
    static const LocationConfig* findLocation(const std::string& uri,const ServerConfig& server);
    static HttpResponse handleGet(const std::string& uri,const ServerConfig& server,const LocationConfig* location);
    static HttpResponse handlePost(const std::string& uri,const std::string& body,const ServerConfig& server,const LocationConfig* location);
    static HttpResponse handleDelete(const std::string& uri,const ServerConfig& server,const LocationConfig* location);

    static std::string buildFilePath(const std::string& uri,const ServerConfig& server,const LocationConfig* location);

    static HttpResponse makeErrorResponse(int code,const ServerConfig& server);

    static bool fileExists(const std::string& path);
    static bool directoryExists(const std::string& path);
    static std::string joinPath(const std::string& a, const std::string& b);
};

#endif
