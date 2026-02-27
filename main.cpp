#include <iostream>
#include "config/include/Lexer.hpp"
#include "config/include/Parser.hpp"
#include "config/include/Validator.hpp"

void printServerInfo(const ServerConfig& server)
{
	std::cout << "\n=== Server Configuration ===" << std::endl;
	std::cout << "Port: " << server.getPort() << std::endl;
	std::cout << "Host: " << server.getHost() << std::endl;
	std::cout << "Server Name: " << server.getServerName() << std::endl;
	std::cout << "Root: " << server.getRoot() << std::endl;
	std::cout << "Index: " << server.getIndex() << std::endl;
	std::cout << "Client Max Body Size: " << server.getClientMaxBodySize() << " bytes" << std::endl;

	const std::map<int, std::string>& errorPages = server.getErrorPages();
	if (!errorPages.empty())
	{
		std::cout << "\nError Pages:" << std::endl;
		for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
		{
			std::cout << "  " << it->first << " -> " << it->second << std::endl;
		}
	}

	const std::vector<LocationConfig>& locations = server.getLocations();
	if (!locations.empty())
	{
		std::cout << "\nLocations:" << std::endl;
		for (size_t i = 0; i < locations.size(); ++i)
		{
			const LocationConfig& loc = locations[i];
			std::cout << "  Location: " << loc.getPath() << std::endl;

			const std::vector<std::string>& methods = loc.getMethods();
			if (!methods.empty())
			{
				std::cout << "    Methods: ";
				for (size_t j = 0; j < methods.size(); ++j)
				{
					std::cout << methods[j];
					if (j < methods.size() - 1)
						std::cout << ", ";
				}
				std::cout << std::endl;
			}

			if (!loc.getRoot().empty())
				std::cout << "    Root: " << loc.getRoot() << std::endl;
			if (!loc.getIndex().empty())
				std::cout << "    Index: " << loc.getIndex() << std::endl;
			if (loc.getAutoindex())
				std::cout << "    Autoindex: on" << std::endl;
			if (!loc.getUploadStore().empty())
				std::cout << "    Upload Store: " << loc.getUploadStore() << std::endl;
			if (!loc.getCgiExtension().empty())
				std::cout << "    CGI Extension: " << loc.getCgiExtension() << std::endl;
			if (loc.hasRedirect())
				std::cout << "    Redirect: " << loc.getRedirectCode() << " -> " << loc.getRedirectUrl() << std::endl;
		}
	}
	std::cout << std::endl;
}

int main(int ac, char **av)
{
	if (ac != 2)
	{
		std::cerr << "Usage: " << av[0] << " <config_file>" << std::endl;
		return 1;
	}

	try
	{
		std::cout << "Parsing configuration file: " << av[1] << std::endl;
		std::cout << "==========================================\n" << std::endl;

		// Lexical analysis
		Lexer lexer(av[1]);
		std::cout << "✓ Lexical analysis complete" << std::endl;

		// Parsing
		Parser parser(lexer.getTokens());
		parser.parse();
		std::cout << "✓ Parsing complete" << std::endl;

		// Validation
		Validator validator(parser.getServers());
		validator.validate();
		std::cout << "✓ Validation complete" << std::endl;

		// Print configuration summary
		const std::vector<ServerConfig>& servers = parser.getServers();
		std::cout << "\nFound " << servers.size() << " server block(s)" << std::endl;

		for (size_t i = 0; i < servers.size(); ++i)
		{
			printServerInfo(servers[i]);
		}

		std::cout << "\n✓ Configuration is valid!" << std::endl;
		return 0;
	}
	catch (std::exception& e)
	{
		std::cerr << "\n✗ Error: " << e.what() << std::endl;
		return 1;
	}
}
