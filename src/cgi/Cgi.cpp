#include "../../include/cgi/Cgi.hpp"
#include <cstddef>
#include <cstring>

Cgi::Cgi() : pid(-1), cgi_status(CGI_SUCCESS), envp(NULL) {};

Cgi::Cgi(const HttpRequest &req, const std::string &scriptPath, time_t timeout)
    : timeout(timeout), script_path(scriptPath), envp(NULL)
{
    buildEnvp(req);
    if (!envp)
    {
        cgi_status = CGI_EXEC_ERROR;
        return;
    }
    if (pipe(script_in) == -1)
    {
        cgi_status = CGI_PIPE_ERROR;
        return;
    }
    if (pipe(script_out) == -1)
    {
        cgi_status = CGI_PIPE_ERROR;
        close(script_in[0]);
        close(script_in[1]);
        return;
    }
    pid = fork();
    if (pid == -1)
    {
        cgi_status = CGI_EXEC_ERROR;
        closePipes();
        return;
    }
    if (pid == 0)
    {
        dup2(script_in[0], STDIN_FILENO);
        dup2(script_out[1], STDOUT_FILENO);
        closePipes();
        char *argv[] = {
            const_cast<char *>(script_interpreter.c_str()),
            const_cast<char *>(script_path.c_str()),
            NULL
        };
        execve(script_interpreter.c_str(), argv, envp);
        free_envp();
        cgi_status = CGI_EXEC_ERROR;
        _exit(1);
    }
    close(script_in[0]);
    close(script_out[1]);
    start_time = time(NULL);
}


Cgi::~Cgi()
{
    if (cgi_status == CGI_SUCCESS)
    {
        close(script_in[1]);
        close(script_out[0]);
        waitpid(pid, &status, 0);
    }
    free_envp();
}


void Cgi::closePipes()
{
    close(script_in[0]);
    close(script_in[1]);
    
    close(script_out[1]);
    close(script_out[0]);
}

void Cgi::free_envp()
{
    if (!envp)
        return;
    for (size_t i = 0; envp[i] != NULL; i++)
        delete [] envp[i];
    delete [] envp;
    envp = NULL;
}

std::string Cgi::ultostr(size_t num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

void    Cgi::buildEnvp(const HttpRequest &req)
{
    size_t pos = script_path.find_last_of('/');
    script_name = (pos != std::string::npos) ? script_path.substr(pos + 1) : script_path;
    pos = req.uri.find('?');
    request_uri = (pos != std::string::npos) ? req.uri.substr(0, pos) : req.uri;
    query_string = (pos != std::string::npos) ? req.uri.substr(pos + 1) : "";
    
    
    env_vect.push_back("REQUEST_METHOD=" + req.method);
    env_vect.push_back("REQUEST_URI=" + request_uri);
    env_vect.push_back("CONTENT_TYPE=" + req.headers.at("Content-Type"));
    env_vect.push_back("CONTENT_LENGTH=" + ultostr(req.contentLength));
    env_vect.push_back("QUERY_STRING=" + query_string);
    env_vect.push_back("SCRIPT_FILENAME=" + script_path);
    env_vect.push_back("SCRIPT_NAME=" + script_name);
    env_vect.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env_vect.push_back("SERVER_PROTOCOL=HTTP/1.0");
    env_vect.push_back("SERVER_SOFTWARE=WebServ/1.0");


    script_interpreter = "/usr/bin/python3";
    envp = new char*[env_vect.size() + 1];
    for (size_t i = 0; i < env_vect.size(); i++)
    {
        envp[i] = new char[env_vect[i].size() + 1];
        strcpy(envp[i], env_vect[i].c_str());
    }
    envp[env_vect.size()] = NULL;
}