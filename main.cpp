#include <iostream>
#include "tahalla/include/Lexer.hpp"

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
		return 0;
	}
	catch (std::exception& e)
	{
		std::cerr << "\n✗ Error: " << e.what() << std::endl;
		return 1;
	}
}
