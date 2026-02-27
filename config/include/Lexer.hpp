#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <fstream>
#include "Token.hpp"
#include "ConfigException.hpp"

class Lexer
{
private:
	std::string _filename;
	std::string _content;
	std::vector<Token> _tokens;
	size_t _pos;
	size_t _line;
	size_t _column;

	// Core lexing functions
	void readFile();
	void removeComments();
	void tokenize();
	
	// Helper functions
	bool isWhitespace(char c) const;
	bool isWordChar(char c) const;
	void skipWhitespace();
	Token readWord();
	void addToken(TokenType type, const std::string& value);
	char currentChar() const;
	char peekChar(size_t offset = 1) const;
	void advance();

public:
	explicit Lexer(const std::string& filename);
	~Lexer();

	const std::vector<Token>& getTokens() const;
	void printTokens() const; // For debugging
};

#endif
