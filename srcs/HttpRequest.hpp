#pragma once

#include <string>
#include <map>
#include <cstddef>

class HttpRequest
{
public:
    std::string                        method;
    std::string                        path;
    std::string                        queryString;
    std::string                        version;
    std::map<std::string, std::string> headers;
    std::string                        body;
    bool                               complete;
    int                                errorCode;

    HttpRequest();

    bool        parse(const std::string& raw);
    std::string getHeader(const std::string& name) const;
    size_t      getContentLength() const;

    static bool isRequestComplete(const std::string& raw);
};
