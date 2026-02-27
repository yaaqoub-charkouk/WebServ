#include "../include/ServerConfig.hpp"

ServerConfig::ServerConfig()
	: _port(80),
	  _host("0.0.0.0"),
	  _serverName(""),
	  _root(""),
	  _index("index.html"),
	  _clientMaxBodySize(1048576), // 1MB default
	  _errorPages(),
	  _locations()
{
}

ServerConfig::ServerConfig(const ServerConfig& other)
	: _port(other._port),
	  _host(other._host),
	  _serverName(other._serverName),
	  _root(other._root),
	  _index(other._index),
	  _clientMaxBodySize(other._clientMaxBodySize),
	  _errorPages(other._errorPages),
	  _locations(other._locations)
{
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other)
{
	if (this != &other)
	{
		_port = other._port;
		_host = other._host;
		_serverName = other._serverName;
		_root = other._root;
		_index = other._index;
		_clientMaxBodySize = other._clientMaxBodySize;
		_errorPages = other._errorPages;
		_locations = other._locations;
	}
	return *this;
}

ServerConfig::~ServerConfig()
{
}

std::string ServerConfig::getErrorPage(int code) const
{
	std::map<int, std::string>::const_iterator it = _errorPages.find(code);
	if (it != _errorPages.end())
		return it->second;
	return "";
}

const LocationConfig* ServerConfig::findLocation(const std::string& path) const
{
	const LocationConfig* bestMatch = NULL;
	size_t bestMatchLength = 0;

	for (size_t i = 0; i < _locations.size(); ++i)
	{
		const std::string& locPath = _locations[i].getPath();
		size_t locLen = locPath.length();

		// Check if location path is a prefix of the requested path
		if (path.compare(0, locLen, locPath) == 0)
		{
			if (locLen > bestMatchLength)
			{
				bestMatch = &_locations[i];
				bestMatchLength = locLen;
			}
		}
	}

	return bestMatch;
}
