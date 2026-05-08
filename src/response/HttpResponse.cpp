#include "../../include/response/HttpResponse.hpp"


HttpResponse::HttpResponse() :
status(-1), status_msg(""), headers(), body(""){}

void    HttpResponse::setStatus(int code, const std::string &msg)
{
    status = code;
    status_msg = msg;
}

int     HttpResponse::getStatusCode()
{
    return status;
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
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
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


std::string HttpResponse::getResponse() const
{
    std::string CRLF = "\r\n";
    std::stringstream rs;
    std::map<std::string, std::string>::const_iterator it;

    rs << "HTTP/1.1 " << status << " " << status_msg << CRLF;

    for (it = headers.begin(); it != headers.end(); it++)
        rs << it->first << ": " << it->second << CRLF;

    rs << CRLF;
    rs << body;

    return (rs.str());
}


HttpResponse HttpResponse::makeAutoindexRes(const std::string &path, const std::string &uri)
{
    DIR *dir;
    struct dirent *entry;
    std::stringstream body;
    std::stringstream size;
    HttpResponse res;
    
    dir = opendir(path.c_str());
    if (!dir)
        return (makeErrorRes(403, "403.html"));
    
    std::string uriPath = (uri.empty() || uri[uri.size() - 1] != '/') ? uri + "/" : uri;
    
    body << "<!DOCTYPE html>\n<html>\n<head><meta charset=\"utf-8\"><title>Index of " << uri << "</title></head>"
        << "<body><h1>Index of " << uri << "</h1><hr><ul>\n";
        while ((entry = readdir(dir)) != NULL)
        {
            std::string name = entry->d_name;
            if (name == ".")
                continue;
            if ( entry->d_type == DT_DIR)
                name += "/";
            body << "<li><a href=\"" << uriPath + name << "\">" << name << "</a></li>\n";

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
        def << "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>" << code << " - " << getStatusMsg(code) << "</title>"
            << "<style>:root{--bg-1:#0f172a;--bg-2:#1e293b;--text:#e5e7eb;--muted:#94a3b8;--accent:#f59e0b;--accent-2:#ef4444;}"
            << "*{box-sizing:border-box;margin:0;padding:0;} "
            << "body{min-height:100vh;font-family:sans-serif;color:var(--text);"
            << "background:radial-gradient(circle at 10% 20%, rgba(245,158,11,0.18), transparent 35%),radial-gradient(circle at 90% 80%, rgba(239,68,68,0.15), transparent 40%),linear-gradient(130deg, var(--bg-1), var(--bg-2));"
            << "display:grid;place-items:center;padding:24px;text-align:center;} "
            << ".card{width:min(720px,100%);background:linear-gradient(180deg,rgba(17,24,39,0.92),rgba(2,6,23,0.92));border:1px solid rgba(148,163,184,0.25);border-radius:18px;padding:36px;box-shadow:0 24px 60px rgba(0,0,0,0.45);} "
            << ".code{font-size:112px;font-weight:800;color:var(--accent);text-shadow:0 0 28px rgba(245,158,11,0.35);line-height:1;margin-bottom:10px;} "
            << "h1{font-size:36px;margin-bottom:14px;} "
            << "p{color:var(--muted);font-size:18px;margin-bottom:26px;} "
            << ".btn{text-decoration:none;border-radius:10px;padding:12px 18px;font-weight:700;color:#0b1020;background:linear-gradient(90deg,var(--accent),var(--accent-2));display:inline-block;} "
            << "</style></head><body>"
            << "<main class=\"card\"><div class=\"code\">" << code << "</div>"
            << "<h1>" << getStatusMsg(code) << "</h1>"
            << "<p>We could not process this request.</p>"
            << "<a class=\"btn\" href=\"/\">Go to Home</a>"
            << "</main></body></html>";
        content = def.str();
        size << content.size();
    }
    
    res.setStatus(code, getStatusMsg(code));
    res.setHeaders("Content-Type", "text/html");
    res.setHeaders("Content-Length", size.str());
    res.setBody(content);
    
    return res;
}


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
