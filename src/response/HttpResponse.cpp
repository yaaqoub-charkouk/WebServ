#include "../../include/response/HttpResponse.hpp"


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

// <!doctype html>
// <html>
// <head><meta charset="utf-8"><title>Index of /images/</title></head>
// <body>
//   <h1>Index of /images/</h1>
//   <hr>
//   <ul>
//     <li><a href="../">../</a></li>
//     <li><a href="/images/cat.jpg">cat.jpg</a></li>
//     <li><a href="/images/icons/">icons/</a></li>
//     <li><a href="/images/notes.txt">notes.txt</a></li>
//   </ul>
//   <hr>
// </body>
// </html>
HttpResponse HttpResponse::makeAutoindexRes(const std::string &path)
{
    DIR *dir;
    struct dirent *entry;
    std::stringstream body;
    std::stringstream size;
    HttpResponse res;
    
    dir = opendir(path.c_str());
    if (!dir)
        return (makeErrorRes(403, "403.html"));
    
    body << "<!DOCTYPE html>\n<html>\n<head><meta charset=\"utf-8\"><title>Index of " << path << "</title></head>"
        << "<body><h1>Index of " << path << "</h1><hr><ul>\n";
        while ((entry = readdir(dir)) != NULL)
        {
            std::string name = entry->d_name;
            if (name == ".")
                continue;
            if ( entry->d_type == DT_DIR)
                name += "/";
            body << "<li><a href=\"/uploads/" << name << "\">" << name << "</a></li>\n"; // hna dima ki3tini index of uploads

        }
    body << "</ul><hr></body>\n</html>";
    closedir(dir);
    res.setStatus(200, "OK");
    res.setHeaders("Content-Type", "text/html");
    size << body.str().size();
    res.setHeaders("Content-Length", size.str());
    res.setBody(body.str());
    return res;
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
        def << "<html><body><style>body{display:flex;justify-content:center;height:100vh;}</style><h1>" << code << " " 
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

HttpResponse HttpResponse::makeCgiRes(const std::string &body, std::map<std::string, std::string> &headers)
{
    HttpResponse res;
    std::map<std::string, std::string>::const_iterator it;
    std::stringstream ss;

    if (headers.find("Status") != headers.end())
    {
        std::string status_line = headers.at("Status");
        size_t space_pos = status_line.find(' ');
        if (space_pos != std::string::npos)
        {
            ss << status_line.substr(0, space_pos);
            int code;
            ss >> code;
            std::string msg = status_line.substr(space_pos + 1);
            res.setStatus(code, msg);
        }
        else
            res.setStatus(200, "OK");
        headers.erase("Status");
    }
    else
        res.setStatus(200, "OK");
    if (headers.find("Content-Length") == headers.end())
    {
        ss.str("");
        ss << body.size();
        res.setHeaders("Content-Length", ss.str());
    }
    
    for (it = headers.begin(); it != headers.end(); it++)
        res.setHeaders(it->first, it->second);
    res.setBody(body);

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
