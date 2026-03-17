# include "server.hpp"
#include <sys/poll.h>



std::string Server::getRequest(int clientFd)
{
    std::map<int, Client>::iterator it = clients.find(clientFd);

    if (it == clients.end())
        return "";

    return it->second.request;
}


void    Server::sendResponse(int clientFd, const std::string& data)
{
    std::map<int, Client>::iterator it = clients.find(clientFd);
    if (it == clients.end())
        return;

    it->second.response = data;

    for (size_t i = 0; i < pollFds.size(); i++)
    {
        if (pollFds[i].fd == clientFd)
        {
            pollFds[i].events = POLLOUT;
            break;
        }
    }
}
