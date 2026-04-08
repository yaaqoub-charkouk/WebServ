#include <iostream>
#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "config/Validator.hpp"
#include "server/server.hpp"

int main(int ac, char **av)
{
	if (ac != 2)
	{
		std::cerr << "Usage: " << av[0] << " <config_file>" << std::endl;
		return 1;
	}

	try
	{
		std::cout << "==========================================\n" << std::endl;

		Lexer lexer(av[1]);

		Parser parser(lexer.getTokens());
		parser.parse();
		std::cout << "--- Parsing Complete ---" << std::endl;

		Validator validator(parser.getServers());
		validator.validate();
		std::cout << "--- Validator Complete ---" << std::endl;

		Server		server(parser.getServers());
		server.run();

		return 0;
	}
	catch (std::exception& e)
	{
		std::cerr << "\n✗ Error: " << e.what() << std::endl;
		return 1;
	}
}
