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
    CGI_WRITING,
    CGI_READING,
    CGI_DONE_READING,
    CGI_DONE_WRITING,
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
public:
    int     script_in[2];
    int     script_out[2];
    CgiStatus cgi_status;
private:
    time_t start_time;
    std::string method;
    time_t timeout;
    std::string request_uri;
    std::string query_string;
    std::string script_path;
    std::string script_name;
    std::string script_interpreter;
    std::map<std::string, std::string> headers;
    std::string body;
    HttpResponse res;

    int  read_bytes;
    size_t  written;
    std::string output;
    std::string req_body;
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
    void execute();
    void parseOutput(const std::string &output);
    bool checkTimeout();
    void    read_output();
    void    write_body();
    void makeResponse();
    void    build_response();
    HttpResponse getResponse() const;

};