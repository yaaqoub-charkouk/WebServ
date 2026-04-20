#include "../../include/cgi/Cgi.hpp"
# include "../../include/server/server.hpp"

Cgi::Cgi() : pid(-1), status(0), envp(NULL), start_time(0), timeout(0), cgi_status(CGI_SUCCESS)
{
    script_in[0] = -1;
    script_in[1] = -1;
    script_out[0] = -1;
    script_out[1] = -1;
};

Cgi::Cgi(const HttpRequest &req, const std::string &scriptPath, time_t timeout)
    : pid(-1), status(0), envp(NULL), start_time(0), timeout(timeout), script_path(scriptPath), cgi_status(CGI_IDLE)
{
    script_in[0] = -1;
    script_in[1] = -1;
    script_out[0] = -1;
    script_out[1] = -1;

    buildEnvp(req);
    if (!envp || cgi_status == CGI_ENV_ERROR)
    {
        res = HttpResponse::makeErrorRes(500, "");
        return;
    }
    std::string output = execute(scriptPath, req.body);
    if (output.empty())
    {
        cgi_status = CGI_EXEC_ERROR;
        std::cerr << "Error : CGI: Failed to execute script: " << scriptPath << std::endl;
        return;
    }
    cgi_status = CGI_SUCCESS;
    parseOutput(output);
    makeResponse();
}


Cgi::~Cgi()
{
    if (cgi_status == CGI_SUCCESS)
    {
        closePipes();
        waitpid(pid, &status, 0);
    }
    else if (pid > 0)
    {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
    }
    free_envp();
}


void Cgi::closePipes()
{
    if (script_in[0] != -1)
    {
        close(script_in[0]);
        script_in[0] = -1;
    }
    if (script_in[1] != -1)
    {
        close(script_in[1]);
        script_in[1] = -1;
    }
    if (script_out[0] != -1)
    {
        close(script_out[0]);
        script_out[0] = -1;
    }
    if (script_out[1] != -1)
    {
        close(script_out[1]);
        script_out[1] = -1;
    }
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

void    Cgi::buildHeaders(const HttpRequest &req)
{
    std::map<std::string , std::string>::const_iterator it = req.headers.begin();
    const std::string http_prefix = "HTTP_";

    while (it != req.headers.end())
    {
        std::string key = it->first;
        for (size_t i = 0; i < key.size(); i++)
        {
            if (key[i] == '-')
                key[i] = '_';
            else
             key[i] = toupper(key[i]);
        }
        env_vect.push_back(http_prefix + key + "=" + it->second);
        it++;
    }
}


void    Cgi::parseOutputHeaders(const std::string &output)
{
    size_t pos = 0;
    while (pos < output.size())
    {
        size_t end = output.find("\r\n", pos);
        if (end != std::string::npos)
        {
            std::string line = output.substr(pos, end - pos);
            if (line.empty())
                break;
            size_t colon = line.find(':');
            if (colon != std::string::npos)
            {
                std::string key = line.substr(0, colon);
                std::string value = line.substr(colon + 1);
                while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
                    value.erase(0, 1);
                headers[key] = value;
            }
            pos = end + 2;
        }
        else
            break;
    }
}

void Cgi::parseOutput(const std::string &output)
{
    size_t pos = output.find("\r\n\r\n");
    if (pos != std::string::npos)
    {
        parseOutputHeaders(output.substr(0, pos));
        body = output.substr(pos + 4);
    }
    else
        body = output;
}

void Cgi::makeResponse()
{
    if (cgi_status != CGI_SUCCESS)
    {
        res = HttpResponse::makeErrorRes(500, "");
        return;
    }
    res = HttpResponse::makeCgiRes(body, headers);
}

std::string Cgi::execute(const std::string&, const std::string& input)
{
    if (pipe(script_in) == -1)
    {
        cgi_status = CGI_PIPE_ERROR;
        return "";
    }
    if (pipe(script_out) == -1)
    {
        cgi_status = CGI_PIPE_ERROR;
        close(script_in[0]);
        close(script_in[1]);
        return "";
    }
    
    Server::make_cgi_pipes_nonblocking(script_in, script_out);

    pid = fork();
    if (pid == -1)
    {
        cgi_status = CGI_EXEC_ERROR;
        closePipes();
        return "";
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
        _exit(1); // is exit safe ? can we use throw an exception instead ? just to free up memory & close Fds .
    }
    close(script_in[0]);
    close(script_out[1]);
    start_time = time(NULL);   
    size_t total_written = 0;
    while (total_written < input.size())
    {
        ssize_t written = write(script_in[1], input.c_str() + total_written, input.size() - total_written);
        if (written <= 0)
        {
            cgi_status = CGI_EXEC_ERROR;
            close(script_in[1]);
            return "";
        }
        total_written += written;
    }
    // close(script_in[1]);
    script_in[1] = -1;
    std::string output;
    char buff[1024];
    ssize_t read_bytes;
    while ((read_bytes = read(script_out[0], buff, sizeof(buff))) > 0)
        output.append(buff, read_bytes);
    // close(script_out[0]);
    script_out[0] = -1;
    return output;
}




void    Cgi::buildEnvp(const HttpRequest &req)
{
    size_t pos = script_path.find_last_of('/');
    script_name = (pos != std::string::npos) ? script_path.substr(pos + 1) : script_path;
    pos = req.uri.find('?');
    request_uri = (pos != std::string::npos) ? req.uri.substr(0, pos) : req.uri;
    query_string = (pos != std::string::npos) ? req.uri.substr(pos + 1) : "";
    std::string content_type = req.headers.count("Content-Type") ? req.headers.at("Content-Type") : "text/plain";
    
    env_vect.push_back("REQUEST_METHOD=" + req.method);
    env_vect.push_back("REQUEST_URI=" + request_uri);
    env_vect.push_back("CONTENT_TYPE=" + content_type);
    env_vect.push_back("CONTENT_LENGTH=" + ultostr(req.contentLength));
    env_vect.push_back("QUERY_STRING=" + query_string);
    env_vect.push_back("SCRIPT_FILENAME=" + script_path);
    env_vect.push_back("SCRIPT_NAME=" + script_name);
    env_vect.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env_vect.push_back("SERVER_PROTOCOL=HTTP/1.0");
    env_vect.push_back("SERVER_SOFTWARE=WebServ/1.0");
    env_vect.push_back("REMOTE_ADDR=" + req.clientAddr);
    env_vect.push_back(("SERVER_PORT=" + ultostr(req.serverPort)));
    env_vect.push_back("SERVER_NAME=" + req.serverName);

    buildHeaders(req);

    // for the bonus part php and python
    pos = script_name.find_last_of('.');
    std::string ext = (pos != std::string::npos) ? script_name.substr(pos) : "";
    if (ext == ".py")
        script_interpreter = "/usr/bin/python3";
    else if (ext == ".php")
        script_interpreter = "/usr/bin/php";
    else
    {
        cgi_status = CGI_ENV_ERROR;
        std::cerr << "Error : CGI: Unsupported script type: " << ext << std::endl;
        return;
    }
    
    envp = new char*[env_vect.size() + 1];
    for (size_t i = 0; i < env_vect.size(); i++)
    {
        envp[i] = new char[env_vect[i].size() + 1];
        std::strcpy(envp[i], env_vect[i].c_str());
    }
    envp[env_vect.size()] = NULL;
}



bool Cgi::checkTimeout()
{
    if (cgi_status != CGI_SUCCESS)
        return false;
    time_t curr = time(NULL);
    if (difftime(curr, start_time) > timeout)
    {
        cgi_status = CGI_TIMEOUT;
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
        return true;
    }
    return false;
}


HttpResponse Cgi::getResponse() const
{
    return res;
}
