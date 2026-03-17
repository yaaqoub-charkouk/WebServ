#include <iostream>
#include <map>
#include <vector>
#include "tahalla/include/Lexer.hpp"
#include "tahalla/include/Parser.hpp"
#include "tahalla/include/Validator.hpp"
#include "tahalla/include/ServerConfig.hpp"
#include "yaaqoub/server/server.hpp"

int main(int ac, char **av)
{
    if (ac != 2)
    {
        std::cerr << "Usage: " << av[0] << " <config_file>" << std::endl;
        return 1;
    }

    try
    {
        // Parse and validate configuration
        Lexer lexer(av[1]);
        Parser parser(lexer.getTokens());
        parser.parse();

        Validator validator(parser.getServers());
        validator.validate();

        const std::vector<ServerConfig>& servers = parser.getServers();

        if (servers.empty())
        {
            std::cerr << "Error: no server blocks found in config." << std::endl;
            return 1;
        }

        // Group server configs by port
        std::map<int, std::vector<ServerConfig> > portMap;
        for (size_t i = 0; i < servers.size(); ++i)
            portMap[servers[i].getPort()].push_back(servers[i]);

        // Build the server
        Server server;
        for (std::map<int, std::vector<ServerConfig> >::iterator it = portMap.begin();
             it != portMap.end(); ++it)
        {
            server.addListeningSocket(it->first, it->second);
            std::cout << "Listening on port " << it->first << std::endl;
        }

        std::cout << "WebServ started. Press Ctrl+C to stop." << std::endl;
        server.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
