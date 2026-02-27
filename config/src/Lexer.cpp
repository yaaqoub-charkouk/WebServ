#include "../include/Lexer.hpp"
#include <iostream>
#include <sstream>

Lexer::Lexer(const std::string& filename)
	: _filename(filename),
	  _content(""),
	  _tokens(),
	  _pos(0),
	  _line(1),
	  _column(1)
{
	readFile();
	removeComments();
	tokenize();
	addToken(TOKEN_END, "");
}

Lexer::~Lexer()
{
}

void Lexer::readFile()
{
	std::ifstream file(_filename.c_str());
	if (!file.is_open())
		throw LexerException("Cannot open file: " + _filename);

	std::stringstream buffer;
	buffer << file.rdbuf();
	_content = buffer.str();
	file.close();

	if (_content.empty())
		throw LexerException("File is empty: " + _filename);
}

void Lexer::removeComments()
{
	std::string result;
	result.reserve(_content.size());

	for (size_t i = 0; i < _content.size(); ++i)
	{
		if (_content[i] == '#')
		{
			// Skip until end of line
			while (i < _content.size() && _content[i] != '\n')
				++i;
			if (i < _content.size())
				result += _content[i]; // Keep the newline
		}
		else
		{
			result += _content[i];
		}
	}

	_content = result;
}

void Lexer::tokenize()
{
	_pos = 0;
	_line = 1;
	_column = 1;

	while (_pos < _content.size())
	{
		skipWhitespace();
		
		if (_pos >= _content.size())
			break;

		char c = currentChar();

		if (c == ';')
		{
			addToken(TOKEN_SEMICOLON, ";");
			advance();
		}
		else if (c == '{')
		{
			addToken(TOKEN_OPEN_BRACE, "{");
			advance();
		}
		else if (c == '}')
		{
			addToken(TOKEN_CLOSE_BRACE, "}");
			advance();
		}
		else if (isWordChar(c))
		{
			_tokens.push_back(readWord());
		}
		else
		{
			std::stringstream ss;
			ss << "Unexpected character '" << c << "' at line " << _line << ", column " << _column;
			throw LexerException(ss.str());
		}
	}
}

bool Lexer::isWhitespace(char c) const
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool Lexer::isWordChar(char c) const
{
	return (c >= 'a' && c <= 'z') ||
	       (c >= 'A' && c <= 'Z') ||
	       (c >= '0' && c <= '9') ||
	       c == '_' || c == '-' || c == '.' || c == '/' ||
	       c == ':' || c == '*';
}

void Lexer::skipWhitespace()
{
	while (_pos < _content.size() && isWhitespace(currentChar()))
	{
		if (currentChar() == '\n')
		{
			++_line;
			_column = 0;
		}
		advance();
	}
}

Token Lexer::readWord()
{
	size_t startLine = _line;
	size_t startColumn = _column;
	std::string word;

	while (_pos < _content.size() && isWordChar(currentChar()))
	{
		word += currentChar();
		advance();
	}

	return Token(TOKEN_WORD, word, startLine, startColumn);
}

void Lexer::addToken(TokenType type, const std::string& value)
{
	_tokens.push_back(Token(type, value, _line, _column));
}

char Lexer::currentChar() const
{
	if (_pos >= _content.size())
		return '\0';
	return _content[_pos];
}

char Lexer::peekChar(size_t offset) const
{
	if (_pos + offset >= _content.size())
		return '\0';
	return _content[_pos + offset];
}

void Lexer::advance()
{
	if (_pos < _content.size())
	{
		++_pos;
		++_column;
	}
}

const std::vector<Token>& Lexer::getTokens() const
{
	return _tokens;
}

void Lexer::printTokens() const
{
	for (size_t i = 0; i < _tokens.size(); ++i)
	{
		const Token& tok = _tokens[i];
		std::cout << "Token(";
		
		switch (tok.type)
		{
			case TOKEN_WORD: std::cout << "WORD"; break;
			case TOKEN_SEMICOLON: std::cout << "SEMICOLON"; break;
			case TOKEN_OPEN_BRACE: std::cout << "OPEN_BRACE"; break;
			case TOKEN_CLOSE_BRACE: std::cout << "CLOSE_BRACE"; break;
			case TOKEN_END: std::cout << "END"; break;
		}
		
		std::cout << ", \"" << tok.value << "\", " << tok.line << ":" << tok.column << ")" << std::endl;
	}
}
