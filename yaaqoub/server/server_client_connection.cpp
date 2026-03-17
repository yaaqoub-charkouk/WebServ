# include "server.hpp"
#include "srcs/HttpRequest.hpp"
#include "srcs/RequestHandler.hpp"
#include <sstream>


void    Server::acceptClient(int serverFd)
{
    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    memset(&client, 0, len);

    int client_fd = accept(serverFd, reinterpret_cast<sockaddr*>(&client), &len);
    if (client_fd == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        throw std::runtime_error("Failed to add new client accept failed");
    }

    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        close(client_fd);
        throw std::runtime_error("Failed to make client_fd non block");
    }

    struct pollfd pfd;
    pfd.fd      = client_fd;
    pfd.events  = POLLIN;
    pfd.revents = 0;

    pollFds.push_back(pfd);

    Client c;
    c.listenPort = listenFdToPort.count(serverFd) ? listenFdToPort[serverFd] : 0;
    clients[client_fd] = c;
}


void    Server::readFromClient(struct pollfd& pfd)
{
    char buffer[4096];

    while (true)
    {
        int n = recv(pfd.fd, buffer, sizeof(buffer), 0);

        if (n > 0)
        {
            clients[pfd.fd].request.append(buffer, static_cast<size_t>(n));

            // Check if request is complete
            if (HttpRequest::isRequestComplete(clients[pfd.fd].request))
            {
                // Determine the server config to get client_max_body_size.
                // Try to extract Host header from what we have so far.
                int         port    = clients[pfd.fd].listenPort;
                std::string rawReq  = clients[pfd.fd].request;
                std::string host;
                size_t      hostPos = rawReq.find("\r\nHost:");
                if (hostPos == std::string::npos)
                    hostPos = rawReq.find("\r\nhost:");
                if (hostPos != std::string::npos)
                {
                    size_t vs = hostPos + 7; // skip "\r\nHost:"
                    while (vs < rawReq.size() && rawReq[vs] == ' ')
                        ++vs;
                    size_t ve = rawReq.find("\r\n", vs);
                    if (ve != std::string::npos)
                        host = rawReq.substr(vs, ve - vs);
                }

                const ServerConfig& cfg = findServerConfig(port, host);
                size_t maxBody = cfg.getClientMaxBodySize();

                // Check for oversize body
                size_t headerEnd = rawReq.find("\r\n\r\n");
                if (headerEnd != std::string::npos)
                {
                    size_t bodySize = rawReq.size() - (headerEnd + 4);
                    if (maxBody > 0 && bodySize > maxBody)
                    {
                        HttpResponse errResp = HttpResponse::makeErrorRes(413, "");
                        std::string  raw     = errResp.getResponse();
                        clients[pfd.fd].response       = raw;
                        clients[pfd.fd].rawCgiResponse  = false;
                        pfd.events  = POLLOUT;
                        pfd.revents = 0;
                        return;
                    }
                }
                processRequest(pfd);
                return;
            }
        }
        else if (n == 0)
        {
            closeClient(pfd.fd);
            return;
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            closeClient(pfd.fd);
            return;
        }
    }
}


void    Server::processRequest(struct pollfd& pfd)
{
    Client& client = clients[pfd.fd];

    HttpRequest req;
    if (!req.parse(client.request))
    {
        int code = (req.errorCode != 0) ? req.errorCode : 400;
        HttpResponse errResp = HttpResponse::makeErrorRes(code, "");
        client.response      = errResp.getResponse();
        client.rawCgiResponse = false;
        pfd.events  = POLLOUT;
        pfd.revents = 0;
        return;
    }

    // Find matching server config
    std::string host = req.getHeader("host");
    const ServerConfig& cfg = findServerConfig(client.listenPort, host);

    RequestHandler handler(cfg);
    HttpResponse   resp = handler.handle(req);

    // If the response body is already a full raw HTTP response (from CGI),
    // send it verbatim; otherwise use getResponse()
    std::string raw = resp.getResponse();
    client.response      = raw;
    client.rawCgiResponse = false;
    pfd.events  = POLLOUT;
    pfd.revents = 0;
}


void Server::writeToClient(struct pollfd& pfd)
{
    Client& client = clients[pfd.fd];

    if (client.response.empty())
    {
        closeClient(pfd.fd);
        return;
    }

    ssize_t sent = send(pfd.fd,
                        client.response.c_str(),
                        client.response.size(),
                        0);

    if (sent > 0)
    {
        client.response.erase(0, static_cast<size_t>(sent));
        if (client.response.empty())
            closeClient(pfd.fd);
    }
    else if (sent == 0 || (errno != EAGAIN && errno != EWOULDBLOCK))
    {
        closeClient(pfd.fd);
    }
}


void Server::closeClient(int clientFd)
{
    close(clientFd);

    for (size_t i = 0; i < pollFds.size(); ++i)
    {
        if (pollFds[i].fd == clientFd)
        {
            pollFds.erase(pollFds.begin() + i);
            break;
        }
    }

    clients.erase(clientFd);
}

