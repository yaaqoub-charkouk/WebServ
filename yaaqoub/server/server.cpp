# include "server.hpp"
#include <cerrno>
#include <cstring>
#include <iterator>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sstream>

static std::string portToStr(int port)
{
    std::ostringstream ss;
    ss << port;
    return ss.str();
}

void    Server::addListeningSocket(int port, const std::vector<ServerConfig>& configs)
{
    // If a socket for this port already exists, just add configs
    for (size_t i = 0; i < listenSockets.size(); ++i)
    {
        if (listenFdToPort.count(listenSockets[i]) &&
            listenFdToPort[listenSockets[i]] == port)
        {
            std::vector<ServerConfig>& existing = portToConfigs[port];
            for (size_t j = 0; j < configs.size(); ++j)
                existing.push_back(configs[j]);
            return;
        }
    }

    int newServSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (newServSocket < 0)
        throw std::runtime_error("Failed to create listening socket port : " + portToStr(port));

    int opt = 1;
    if (setsockopt(newServSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        close(newServSocket);
        throw std::runtime_error("Failed to set socket options " + portToStr(port));
    }

    struct sockaddr_in nic;
    memset(&nic, 0, sizeof(nic));
    nic.sin_family = AF_INET;
    nic.sin_port = htons(static_cast<uint16_t>(port));
    if (inet_pton(nic.sin_family, "0.0.0.0", &nic.sin_addr) <= 0) {
        close(newServSocket);
        throw std::runtime_error("Failed to set listening ip " + portToStr(port));
    }

    if (bind(newServSocket, reinterpret_cast<sockaddr*>(&nic), sizeof(nic)) == -1) {
        close(newServSocket);
        throw std::runtime_error("Failed to bind socket with port " + portToStr(port));
    }

    if (listen(newServSocket, 10) == -1) {
        close(newServSocket);
        throw std::runtime_error("Failed to listen on socket binded to port : " + portToStr(port));
    }

    if (fcntl(newServSocket, F_SETFL, O_NONBLOCK) == -1) {
        close(newServSocket);
        throw std::runtime_error("Failed to make the socket non Blocking" + portToStr(port));
    }

    struct pollfd pfd;
    pfd.fd     = newServSocket;
    pfd.events = POLLIN;
    pfd.revents = 0;

    listenSockets.push_back(newServSocket);
    pollFds.push_back(pfd);
    listenFdToPort[newServSocket] = port;
    portToConfigs[port]           = configs;
}


void    Server::run()
{
    while (1337)
    {
        int ret = poll(pollFds.data(), pollFds.size(), -1);

        if (ret < 0)
        {
            if (errno == EINTR)
                continue;
            throw std::runtime_error("poll failed can't listen on servers sockets");
        }

        for (size_t i = 0; i < pollFds.size(); ++i)
        {
            if (pollFds[i].revents == 0)
                continue;
            if (pollFds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                if (!isListeningSocket(pollFds[i].fd))
                    closeClient(pollFds[i].fd);
                continue;
            }
            if (pollFds[i].revents & POLLIN)
            {
                if (isListeningSocket(pollFds[i].fd))
                    acceptClient(pollFds[i].fd);
                else
                    readFromClient(pollFds[i]);
            }
            if (pollFds[i].revents & POLLOUT)
            {
                writeToClient(pollFds[i]);
            }
        }
    }
}


bool Server::isListeningSocket(int fd)
{
    return std::find(listenSockets.begin(),
                     listenSockets.end(),
                     fd) != listenSockets.end();
}

const ServerConfig& Server::findServerConfig(int port,
                                              const std::string& host) const
{
    std::map<int, std::vector<ServerConfig> >::const_iterator it =
        portToConfigs.find(port);

    if (it == portToConfigs.end() || it->second.empty())
        throw std::runtime_error("No server config for port");

    const std::vector<ServerConfig>& cfgs = it->second;

    // Try to match by server_name / Host header
    std::string reqHost = host;
    size_t colon = reqHost.find(':');
    if (colon != std::string::npos)
        reqHost = reqHost.substr(0, colon);

    for (size_t i = 0; i < cfgs.size(); ++i)
    {
        if (cfgs[i].getServerName() == reqHost)
            return cfgs[i];
    }

    // Default: first config for this port
    return cfgs[0];
}
