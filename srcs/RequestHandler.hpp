#pragma once

#include "tahalla/include/ServerConfig.hpp"
#include "tahalla/include/LocationConfig.hpp"
#include "srcs/HttpRequest.hpp"
#include "adnane/HttpResponse/HttpResponse.hpp"

class RequestHandler
{
private:
    const ServerConfig& config;

    HttpResponse handleGet(const HttpRequest& req,
                            const LocationConfig* loc) const;
    HttpResponse handlePost(const HttpRequest& req,
                             const LocationConfig* loc) const;
    HttpResponse handleDelete(const HttpRequest& req,
                               const LocationConfig* loc) const;
    HttpResponse handleCgi(const HttpRequest& req,
                            const std::string& filePath,
                            const LocationConfig* loc) const;

    std::string  resolveRoot(const LocationConfig* loc) const;
    std::string  resolveIndex(const LocationConfig* loc) const;
    std::string  resolveFilePath(const std::string& urlPath,
                                  const LocationConfig* loc) const;
    HttpResponse getErrorResponse(int code) const;
    bool         isCgiRequest(const std::string& filePath,
                               const LocationConfig* loc) const;

public:
    explicit RequestHandler(const ServerConfig& cfg);
    HttpResponse handle(const HttpRequest& req) const;
};
