#ifndef CONFIGEXCEPTION_HPP
#define CONFIGEXCEPTION_HPP

#include <exception>
#include <string>

class ConfigException : public std::exception
{
private:
	std::string message;

public:
	explicit ConfigException(const std::string& message) : message(message) {}
	virtual ~ConfigException() throw() {}

	virtual const char* what() const throw()
	{
		return message.c_str();
	}
};

class LexerException : public ConfigException
{
public:
	explicit LexerException(const std::string& message) 
		: ConfigException("Lexer Error: " + message) {}
	virtual ~LexerException() throw() {}
};


#endif
