#include "../include/Parser.hpp"
#include <sstream>
#include <cstdlib>
#include <cctype>

Parser::Parser(const std::vector<Token>& tokens)
	: token(tokens),
	  index(0),
	  _servers()
{
}

Parser::~Parser()
{
}

void Parser::parse()
{
	parseConfiguration();
}

const std::vector<ServerConfig>& Parser::getServers() const
{
	return _servers;
}

// ============================================================================
// Token Navigation
// ============================================================================

const Token& Parser::currentToken() const
{
	if (index >= token.size())
		return token[token.size() - 1]; // Return END token
	return token[index];
}

const Token& Parser::peekToken(size_t offset) const
{
	size_t peekIndex = index + offset;
	if (peekIndex >= token.size())
		return token[token.size() - 1];
	return token[peekIndex];
}

void Parser::advance()
{
	if (index < token.size())
		++index;
}

bool Parser::isAtEnd() const
{
	return currentToken().type == TOKEN_END;
}

// ============================================================================
// Expectation Helpers
// ============================================================================

void Parser::expect(TokenType type, const std::string& context)
{
	const Token& tok = currentToken();
	if (tok.type != type)
	{
		std::stringstream ss;
		ss << "Expected ";
		
		switch (type)
		{
			case TOKEN_WORD: ss << "word"; break;
			case TOKEN_SEMICOLON: ss << "semicolon"; break;
			case TOKEN_OPEN_BRACE: ss << "open brace"; break;
			case TOKEN_CLOSE_BRACE: ss << "close brace"; break;
			case TOKEN_END: ss << "end of file"; break;
		}
		
		ss << " in " << context;
		throw ParserException(formatError(ss.str(), tok));
	}
}

void Parser::expectWord(const std::string& expected, const std::string& context)
{
	expect(TOKEN_WORD, context);
	const Token& tok = currentToken();
	
	if (tok.value != expected)
	{
		std::stringstream ss;
		ss << "Expected '" << expected << "' in " << context << ", got '" << tok.value << "'";
		throw ParserException(formatError(ss.str(), tok));
	}
}

std::string Parser::expectWord(const std::string& context)
{
	expect(TOKEN_WORD, context);
	std::string value = currentToken().value;
	advance();
	return value;
}

// ============================================================================
// Main Parsing
// ============================================================================

void Parser::parseConfiguration()
{
	while (!isAtEnd())
	{
		const Token& tok = currentToken();
		
		if (tok.type == TOKEN_WORD && tok.value == "server")
		{
			parseServer();
		}
		else
		{
			throw ParserException(formatError("Only 'server' blocks are allowed at top level", tok));
		}
	}

	if (_servers.empty())
		throw ParserException("Configuration must contain at least one server block");
}

void Parser::parseServer()
{
	expectWord("server", "server block");
	advance();
	
	expect(TOKEN_OPEN_BRACE, "server block");
	advance();

	ServerConfig server;

	while (!isAtEnd() && currentToken().type != TOKEN_CLOSE_BRACE)
	{
		parseServerDirective(server);
	}

	expect(TOKEN_CLOSE_BRACE, "server block");
	advance();

	_servers.push_back(server);
}

void Parser::parseServerDirective(ServerConfig& server)
{
	const Token& tok = currentToken();
	
	if (tok.type != TOKEN_WORD)
		throw ParserException(formatError("Expected directive name", tok));

	const std::string& directive = tok.value;

	if (directive == "listen")
		parseListen(server);
	else if (directive == "host")
		parseHost(server);
	else if (directive == "server_name")
		parseServerName(server);
	else if (directive == "root")
		parseRoot(server);
	else if (directive == "index")
		parseIndex(server);
	else if (directive == "client_max_body_size")
		parseClientMaxBodySize(server);
	else if (directive == "error_page")
		parseErrorPage(server);
	else if (directive == "location")
		parseLocation(server);
	else
	{
		std::stringstream ss;
		ss << "Unknown server directive: " << directive;
		throw ParserException(formatError(ss.str(), tok));
	}
}

void Parser::parseLocation(ServerConfig& server)
{
	advance(); // skip 'location'
	
	std::string path = expectWord("location path");
	
	expect(TOKEN_OPEN_BRACE, "location block");
	advance();

	LocationConfig location(path);

	while (!isAtEnd() && currentToken().type != TOKEN_CLOSE_BRACE)
	{
		parseLocationDirective(location);
	}

	expect(TOKEN_CLOSE_BRACE, "location block");
	advance();

	server.addLocation(location);
}

void Parser::parseLocationDirective(LocationConfig& location)
{
	const Token& tok = currentToken();
	
	if (tok.type != TOKEN_WORD)
		throw ParserException(formatError("Expected directive name", tok));

	const std::string& directive = tok.value;

	if (directive == "methods")
		parseMethods(location);
	else if (directive == "root")
		parseLocationRoot(location);
	else if (directive == "index")
		parseLocationIndex(location);
	else if (directive == "autoindex")
		parseAutoindex(location);
	else if (directive == "upload_store")
		parseUploadStore(location);
	else if (directive == "cgi_extension")
		parseCgiExtension(location);
	else if (directive == "return")
		parseReturn(location);
	else
	{
		std::stringstream ss;
		ss << "Unknown location directive: " << directive;
		throw ParserException(formatError(ss.str(), tok));
	}
}

// ============================================================================
// Server Directive Parsers
// ============================================================================

void Parser::parseListen(ServerConfig& server)
{
	advance(); // skip 'listen'
	
	std::string portStr = expectWord("listen directive");
	int port = parseNumber(portStr);
	
	expect(TOKEN_SEMICOLON, "listen directive");
	advance();

	server.setPort(port);
}

void Parser::parseHost(ServerConfig& server)
{
	advance(); // skip 'host'
	
	std::string host = expectWord("host directive");
	
	expect(TOKEN_SEMICOLON, "host directive");
	advance();

	server.setHost(host);
}

void Parser::parseServerName(ServerConfig& server)
{
	advance(); // skip 'server_name'
	
	std::string name = expectWord("server_name directive");
	
	expect(TOKEN_SEMICOLON, "server_name directive");
	advance();

	server.setServerName(name);
}

void Parser::parseRoot(ServerConfig& server)
{
	advance(); // skip 'root'
	
	std::string root = expectWord("root directive");
	
	expect(TOKEN_SEMICOLON, "root directive");
	advance();

	server.setRoot(root);
}

void Parser::parseIndex(ServerConfig& server)
{
	advance(); // skip 'index'
	
	std::string index = expectWord("index directive");
	
	expect(TOKEN_SEMICOLON, "index directive");
	advance();

	server.setIndex(index);
}

void Parser::parseClientMaxBodySize(ServerConfig& server)
{
	advance(); // skip 'client_max_body_size'
	
	std::string sizeStr = expectWord("client_max_body_size directive");
	size_t size = parseBodySize(sizeStr);
	
	expect(TOKEN_SEMICOLON, "client_max_body_size directive");
	advance();

	server.setClientMaxBodySize(size);
}

void Parser::parseErrorPage(ServerConfig& server)
{
	advance(); // skip 'error_page'
	
	std::string codeStr = expectWord("error_page directive");
	int code = parseNumber(codeStr);
	
	std::string path = expectWord("error_page directive");
	
	expect(TOKEN_SEMICOLON, "error_page directive");
	advance();

	server.addErrorPage(code, path);
}

// ============================================================================
// Location Directive Parsers
// ============================================================================

void Parser::parseLocationRoot(LocationConfig& location)
{
	advance(); // skip 'root'
	
	std::string root = expectWord("root directive");
	
	expect(TOKEN_SEMICOLON, "root directive");
	advance();

	location.setRoot(root);
}

void Parser::parseLocationIndex(LocationConfig& location)
{
	advance(); // skip 'index'
	
	std::string index = expectWord("index directive");
	
	expect(TOKEN_SEMICOLON, "index directive");
	advance();

	location.setIndex(index);
}

void Parser::parseMethods(LocationConfig& location)
{
	advance(); // skip 'methods'
	
	// Read methods until semicolon
	while (!isAtEnd() && currentToken().type == TOKEN_WORD)
	{
		std::string method = currentToken().value;
		location.addMethod(method);
		advance();
	}
	
	expect(TOKEN_SEMICOLON, "methods directive");
	advance();
}

void Parser::parseAutoindex(LocationConfig& location)
{
	advance(); // skip 'autoindex'
	
	std::string value = expectWord("autoindex directive");
	
	expect(TOKEN_SEMICOLON, "autoindex directive");
	advance();

	if (value == "on")
		location.setAutoindex(true);
	else if (value == "off")
		location.setAutoindex(false);
	else
	{
		std::stringstream ss;
		ss << "Invalid autoindex value: " << value << " (expected 'on' or 'off')";
		throw ParserException(ss.str());
	}
}

void Parser::parseUploadStore(LocationConfig& location)
{
	advance(); // skip 'upload_store'
	
	std::string path = expectWord("upload_store directive");
	
	expect(TOKEN_SEMICOLON, "upload_store directive");
	advance();

	location.setUploadStore(path);
}

void Parser::parseCgiExtension(LocationConfig& location)
{
	advance(); // skip 'cgi_extension'
	
	std::string ext = expectWord("cgi_extension directive");
	
	expect(TOKEN_SEMICOLON, "cgi_extension directive");
	advance();

	location.setCgiExtension(ext);
}

void Parser::parseReturn(LocationConfig& location)
{
	advance(); // skip 'return'
	
	std::string codeStr = expectWord("return directive");
	int code = parseNumber(codeStr);
	
	std::string url = expectWord("return directive");
	
	expect(TOKEN_SEMICOLON, "return directive");
	advance();

	location.setRedirect(code, url);
}

// ============================================================================
// Utility Functions
// ============================================================================

size_t Parser::parseBodySize(const std::string& value)
{
	if (value.empty())
		throw ParserException("Empty body size value");

	std::string numPart;
	char suffix = '\0';

	for (size_t i = 0; i < value.length(); ++i)
	{
		char c = value[i];
		if (std::isdigit(c))
			numPart += c;
		else if (i == value.length() - 1 && (c == 'K' || c == 'M' || c == 'G'))
			suffix = c;
		else
		{
			std::stringstream ss;
			ss << "Invalid body size format: " << value;
			throw ParserException(ss.str());
		}
	}

	if (numPart.empty())
		throw ParserException("Body size must contain a number");

	size_t num = static_cast<size_t>(std::atol(numPart.c_str()));

	if (suffix == 'K')
		return num * 1024;
	else if (suffix == 'M')
		return num * 1024 * 1024;
	else if (suffix == 'G')
		return num * 1024 * 1024 * 1024;
	else
		return num;
}

int Parser::parseNumber(const std::string& value)
{
	if (value.empty())
		throw ParserException("Empty number value");

	for (size_t i = 0; i < value.length(); ++i)
	{
		if (!std::isdigit(value[i]))
		{
			std::stringstream ss;
			ss << "Invalid number format: " << value;
			throw ParserException(ss.str());
		}
	}

	return std::atoi(value.c_str());
}

std::string Parser::formatError(const std::string& message, const Token& token) const
{
	std::stringstream ss;
	ss << message << " at line " << token.line << ", column " << token.column;
	return ss.str();
}
