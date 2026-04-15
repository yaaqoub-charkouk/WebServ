# include "../../include/server/server.hpp"



std::string Server::getRequest(int clientFd)
{
    std::map<int, Client>::iterator it = clients.find(clientFd);

    if (it == clients.end())
        return "";

    return it->second.request_str;
}


void    Server::sendResponse(int clientFd, const std::string& data)
{
    clients[clientFd].response_str = data;

    for (size_t i = 0; i < pollFds.size(); i++)
    {
        if (pollFds[i].fd == clientFd)
        {
            pollFds[i].events = POLLOUT;
            // pollFds[i].revents = 0; // important ??
            break;
        }
    }
}
