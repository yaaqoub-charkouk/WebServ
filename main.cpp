#include <iostream>
#include "include/config/Lexer.hpp"
#include "include/config/Parser.hpp"
#include "include/config/Validator.hpp"
#include "include/server/server.hpp"
#include "include/logger/Logger.hpp"

int main(int ac, char **av)
{
	if (ac != 2)
	{
		std::cerr << "Usage: " << av[0] << " <config_file>" << std::endl;
		return 1;
	}

	try
	{
		Logger::info("Starting WebServ");

		Lexer lexer(av[1]);

		Parser parser(lexer.getTokens());
		parser.parse();
		Logger::info("Parsing complete");

		Validator validator(parser.getServers());
		validator.validate();
		Logger::info("Validation complete");

		Server		server(parser.getServers());
		server.run();

		return 0;
	}
	catch (std::exception& e)
	{
		Logger::error(std::string("Error: ") + e.what());
		return 1;
	}
}
