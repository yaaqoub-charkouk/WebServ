#include "Cgi.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <sstream>
#include <stdexcept>
#include <cstdlib>

Cgi::Cgi(const HttpRequest& req, const ServerConfig& conf,
         const std::string& script, const std::string& root)
    : request(req), config(conf), scriptPath(script), docRoot(root)
{
}

static std::string intToStr(int n)
{
    std::ostringstream ss;
    ss << n;
    return ss.str();
}

static std::string sizeToStr(size_t n)
{
    std::ostringstream ss;
    ss << n;
    return ss.str();
}

std::vector<std::string> Cgi::buildEnv() const
{
    std::vector<std::string> env;

    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_PROTOCOL=" + request.version);
    env.push_back("SERVER_SOFTWARE=WebServ/1.0");
    env.push_back("SERVER_NAME=" + config.getServerName());
    env.push_back("SERVER_PORT=" + intToStr(config.getPort()));
    env.push_back("REQUEST_METHOD=" + request.method);
    env.push_back("SCRIPT_FILENAME=" + scriptPath);
    env.push_back("SCRIPT_NAME=" + request.path);
    env.push_back("PATH_INFO=" + request.path);
    env.push_back("PATH_TRANSLATED=" + docRoot + request.path);
    env.push_back("QUERY_STRING=" + request.queryString);
    env.push_back("REDIRECT_STATUS=200");

    std::string contentLength = request.getHeader("content-length");
    if (!contentLength.empty())
        env.push_back("CONTENT_LENGTH=" + contentLength);
    else
        env.push_back("CONTENT_LENGTH=" + sizeToStr(request.body.size()));

    std::string contentType = request.getHeader("content-type");
    if (!contentType.empty())
        env.push_back("CONTENT_TYPE=" + contentType);

    std::string host = request.getHeader("host");
    if (!host.empty())
        env.push_back("HTTP_HOST=" + host);

    std::string cookie = request.getHeader("cookie");
    if (!cookie.empty())
        env.push_back("HTTP_COOKIE=" + cookie);

    std::string accept = request.getHeader("accept");
    if (!accept.empty())
        env.push_back("HTTP_ACCEPT=" + accept);

    std::string userAgent = request.getHeader("user-agent");
    if (!userAgent.empty())
        env.push_back("HTTP_USER_AGENT=" + userAgent);

    return env;
}

std::string Cgi::parseResponse(const std::string& raw) const
{
    // CGI output may start with headers like "Content-Type: text/html\r\n\r\n<body>"
    // We need to turn it into a proper HTTP/1.1 response.
    size_t headerEnd = raw.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        headerEnd = raw.find("\n\n");

    std::string cgiHeaders;
    std::string cgiBody;

    if (headerEnd == std::string::npos)
    {
        cgiBody = raw;
    }
    else
    {
        // Detect whether separator is \r\n\r\n or \n\n
        size_t sepLen = (raw.substr(headerEnd, 4) == "\r\n\r\n") ? 4 : 2;
        cgiHeaders = raw.substr(0, headerEnd);
        cgiBody    = raw.substr(headerEnd + sepLen);
    }

    // Parse CGI headers
    int         statusCode = 200;
    std::string statusMsg  = "OK";
    std::string extraHeaders;

    std::istringstream ss(cgiHeaders);
    std::string line;
    while (std::getline(ss, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        size_t colon = line.find(':');
        if (colon == std::string::npos)
            continue;

        std::string key = line.substr(0, colon);
        std::string val = line.substr(colon + 1);
        // Trim leading spaces from val
        size_t vs = 0;
        while (vs < val.size() && val[vs] == ' ')
            ++vs;
        val = val.substr(vs);

        if (key == "Status")
        {
            std::istringstream statss(val);
            statss >> statusCode;
            size_t sp = val.find(' ');
            if (sp != std::string::npos)
                statusMsg = val.substr(sp + 1);
        }
        else
        {
            extraHeaders += key + ": " + val + "\r\n";
        }
    }

    std::ostringstream resp;
    resp << "HTTP/1.1 " << statusCode << " " << statusMsg << "\r\n";
    resp << extraHeaders;
    resp << "Content-Length: " << cgiBody.size() << "\r\n";
    resp << "Connection: close\r\n";
    resp << "\r\n";
    resp << cgiBody;

    return resp.str();
}

std::string Cgi::execute()
{
    int stdinPipe[2];
    int stdoutPipe[2];

    if (pipe(stdinPipe) == -1 || pipe(stdoutPipe) == -1)
        throw std::runtime_error("CGI: pipe() failed: " + std::string(strerror(errno)));

    std::vector<std::string> envStrings = buildEnv();
    std::vector<char*> envp;
    for (size_t i = 0; i < envStrings.size(); ++i)
        envp.push_back(const_cast<char*>(envStrings[i].c_str()));
    envp.push_back(NULL);

    // Build argv: interpreter (if needed) + script path
    std::vector<char*> argv;
    argv.push_back(const_cast<char*>(scriptPath.c_str()));
    argv.push_back(NULL);

    pid_t pid = fork();
    if (pid == -1)
    {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        throw std::runtime_error("CGI: fork() failed: " + std::string(strerror(errno)));
    }

    if (pid == 0)
    {
        // Child: redirect stdin/stdout
        close(stdinPipe[1]);
        close(stdoutPipe[0]);

        if (dup2(stdinPipe[0], STDIN_FILENO) == -1)
            _exit(1);
        if (dup2(stdoutPipe[1], STDOUT_FILENO) == -1)
            _exit(1);

        close(stdinPipe[0]);
        close(stdoutPipe[1]);

        // Change to script directory
        std::string dir = scriptPath;
        size_t slash = dir.rfind('/');
        if (slash != std::string::npos)
        {
            dir = dir.substr(0, slash);
            if (chdir(dir.c_str()) == -1)
                _exit(1);
        }

        execve(scriptPath.c_str(), argv.data(), envp.data());
        _exit(1);
    }

    // Parent
    close(stdinPipe[0]);
    close(stdoutPipe[1]);

    // Write request body to child stdin
    const std::string& body = request.body;
    size_t written = 0;
    while (written < body.size())
    {
        ssize_t n = write(stdinPipe[1], body.c_str() + written, body.size() - written);
        if (n <= 0)
            break;
        written += static_cast<size_t>(n);
    }
    close(stdinPipe[1]);

    // Read child stdout with timeout via select
    std::string output;
    char buf[4096];
    ssize_t n;
    while ((n = read(stdoutPipe[0], buf, sizeof(buf))) > 0)
        output.append(buf, static_cast<size_t>(n));
    close(stdoutPipe[0]);

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        throw std::runtime_error("CGI script exited with non-zero status");

    return parseResponse(output);
}
