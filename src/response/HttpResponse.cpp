#include "response/HttpResponse.hpp"


HttpResponse::HttpResponse() :
status(-1), status_msg(""), headers(), body(""){}

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

std::string HttpResponse::getStatusMsg(int code)
{
    switch (code)
    {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Content Too Large";
        case 500: return "Internal Server Error";
        default:  return "Unknown";
    }
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


// Response example
// HTTP/1.1 200 OK\r\n
// Content-Type: text/html\r\n
// Content-Length: 150\r\n
// \r\n
// <html>...</html>

std::string HttpResponse::getResponse() const
{
    std::string CRLF = "\r\n";
    std::stringstream rs;
    std::map<std::string, std::string>::const_iterator it;

    rs << "HTTP/1.0 " << status << " " << status_msg << CRLF;

    for (it = headers.begin(); it != headers.end(); it++)
        rs << it->first << ": " << it->second << CRLF;

    rs << CRLF;
    rs << body;

    return (rs.str());
}


HttpResponse HttpResponse::makeErrorRes(int code, const std::string &path)
{
    HttpResponse res;
    FileInfo info;
    std::string content;
    std::stringstream size;
    std::stringstream def;

    
    info = FileUtils::getInfo(path);
    
    if (info.exist && info.isReadable && info.isFile)
    {
        size << info.size;
        content = FileUtils::getContent(path);
    }
    else
    {
        def << "<html><body><h1>" << code << " " 
        << getStatusMsg(code) <<"</h1></body></html>";
        content = def.str();
        size << content.size();
    }
    
    res.setStatus(code, getStatusMsg(code));
    res.setHeaders("Content-Type", "text/html");
    res.setHeaders("Content-Length", size.str());
    res.setBody(content);
    
    return res;
}

// Redirection example
// HTTP/1.0 301 Moved Permanently\r\n
// Location: /new-url\r\n
// Content-Length: 0\r\n
// \r\n

HttpResponse HttpResponse::makeRedireRes(int code, const std::string &location)
{
    HttpResponse res;
    
    res.setStatus(code, getStatusMsg(code));
    res.setHeaders("Location", location);
    res.setHeaders("Content-Length", "0");

    return res;
}

HttpResponse HttpResponse::makeFileRes(const std::string &path)
{
    HttpResponse res;
    FileInfo info;
    std::string content;
    std::string mime;
    std::stringstream size;

    info = FileUtils::getInfo(path);

    if (!info.exist)
        return (makeErrorRes(404, "404.html"));
    if (!info.isReadable)
        return (makeErrorRes(403, "403.html"));
    if (!info.isFile)
        return (makeErrorRes(403, "403.html"));

    size << info.size;
    content = FileUtils::getContent(path);
    mime = getFileType(getFileExtension(path));
   
    res.setStatus(200, "OK");
    res.setHeaders("Content-Type", mime);
    res.setHeaders("Content-Length", size.str());
    res.setBody(content);
    return res;
}
