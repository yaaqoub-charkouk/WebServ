#pragma once

#include <string>
#include <map>
#include <sstream>
#include "response/FileUtils.hpp"



class HttpResponse
{
private:
    int status;
    std::string status_msg;
    std::map<std::string,std::string> headers;
    std::string body;


public:
    HttpResponse();
    // Response building
    void    setStatus(int code, const std::string &msg);
    void    setHeaders(const std::string &key, const std::string &val);
    void    setBody(const std::string &body);
    static std::string getStatusMsg(int code);
    
    std::string getResponse() const;

    //Responses type
    static HttpResponse makeFileRes(const std::string &path);
    static HttpResponse makeErrorRes(int code, const std::string &path);
    static HttpResponse makeRedireRes(int code, const std::string &location);
    
    // For files (if the extension is .html the type is text/html)
   static  std::string getFileType(const std::string &extension);
   static  std::string getFileExtension(const std::string &filename);
};


