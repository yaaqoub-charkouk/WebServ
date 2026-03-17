#pragma once

#include <string>
#include <vector>
#include <map>
#include "srcs/HttpRequest.hpp"
#include "tahalla/include/ServerConfig.hpp"

class Cgi
{
private:
    const HttpRequest&  request;
    const ServerConfig& config;
    std::string         scriptPath;
    std::string         docRoot;

    std::vector<std::string> buildEnv() const;
    std::string parseResponse(const std::string& raw) const;

public:
    Cgi(const HttpRequest& req, const ServerConfig& conf,
        const std::string& script, const std::string& root);

    std::string execute();
};
