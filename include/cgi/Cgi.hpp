#pragma once
#include "../request/HttpRequest.hpp"
#include "../response/HttpResponse.hpp"

#include <ctime>
#include <sys/types.h>
#include <vector>
#include <string>
#include <map>
#include <sys/wait.h>
#include <sstream>
#include <iostream>
#include <cstring>
#include <signal.h> // Added for kill() function

enum CgiStatus
{
    CGI_IDLE,
    CGI_ENV_ERROR,
    CGI_SUCCESS,
    CGI_EXEC_ERROR,
    CGI_PIPE_ERROR,
    CGI_TIMEOUT
};

class Cgi
{
private:
    pid_t pid;
    int status;
    std::vector<std::string> env_vect;
    char **envp;
    int script_in[2];
    int script_out[2];
    time_t start_time;
    time_t timeout;
    std::string request_uri;
    std::string query_string;
    std::string script_path;
    std::string script_name;
    std::string script_interpreter;
    CgiStatus cgi_status;
    std::map<std::string, std::string> headers;
    std::string body;
    HttpResponse res;
    
    void closePipes();
    std::string ultostr(size_t num);
    void free_envp();
    void buildEnvp(const HttpRequest &req);
    void buildHeaders(const HttpRequest &req);
    void parseOutputHeaders(const std::string &output);
public:
    Cgi();
    Cgi(const HttpRequest &req, const std::string &scriptPath, time_t timeout = 5);
    ~Cgi();
    std::string execute(const std::string&, const std::string& input);
    void parseOutput(const std::string &output);
    bool checkTimeout();
    void makeResponse();
    HttpResponse getResponse() const;

};