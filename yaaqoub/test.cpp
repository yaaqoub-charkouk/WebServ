#include <iostream>
#include <unistd.h>
#include <poll.h>

int main()
{
    struct pollfd pfd;
    pfd.fd = 0;          // stdin
    pfd.events = POLLIN; // wait until readable
    pfd.revents = 0;

    std::cout << "Waiting for input...\n";
    
    int ret = poll(&pfd, 1, -1); // block until something happens
    std::cout << "Waiting for input...\n";

    if (ret < 0)
    {
        std::cerr << "poll failed\n";
        return 1;
    }

    if (pfd.revents & POLLIN)
    {
        char buffer[1024];
        int n = read(0, buffer, sizeof(buffer));
        std::cout << "Read " << n << " bytes\n";
    }
}