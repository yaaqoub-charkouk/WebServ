#include "HttpResponse.hpp"


HttpResponse::HttpResponse() :
status(-1), status_msg(""), body(""), headers(){}

void    HttpResponse::setStatus(int code, const std::string &msg)
{
    status = code;
    status_msg = msg;
}

void    HttpResponse::setBody(const std::string &body)
{
    this->body = body;
}

void    HttpResponse::setHeaders(const std::string &key, const std::string &val)
{
    headers.insert(std::make_pair(key, val));
}

std::string HttpResponse::getFileType(const std::string &extension)
{
    static std::map<std::string, std::string> mimes;
    if (mimes.empty())
    {
        mimes[".html"] = "text/html";
        mimes[".css"] = "text/css";
        mimes[".js"] = "application/javascript";
        mimes[".png"] = "image/png";
        mimes[".jpg"] = "image/jpeg";
        mimes[".ico"] = "image/x-icon";
        mimes[".txt"] = "text/plain";
    }

    if (mimes.find(extension) != mimes.end())
        return mimes[extension];

    return "application/octet-stream";
}

std::string HttpResponse::getFileExtension(const std::string &filename)
{
    std::string extension;
    size_t pos = filename.rfind('.');

    if (pos == std::string::npos || pos == 0) //Pos == 0 if a hidden file
        return ("");

    extension = filename.substr(pos);
    return extension;
}

// std::string CRLN = "\r\n";

// Response example
// HTTP/1.1 200 OK\r\n
// Content-Type: text/html\r\n
// Content-Length: 150\r\n
// \r\n
// <html>...</html>

std::string HttpResponse::getResponse() const
{
    std::stringstream code;
    code << status;
    
}
