#include "../include/ServerConfig.hpp"

ServerConfig::ServerConfig()
	: port(80),
	  host("0.0.0.0"),
	  serverName(""),
	  root(""),
	  index("index.html"),
	  clientMaxBodySize(1048576),
	  errorPages(),
	  locations()
{
}

ServerConfig::ServerConfig(const ServerConfig& other)
	: port(other.port),
	  host(other.host),
	  serverName(other.serverName),
	  root(other.root),
	  index(other.index),
	  clientMaxBodySize(other.clientMaxBodySize),
	  errorPages(other.errorPages),
	  locations(other.locations)
{
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other)
{
	if (this != &other)
	{
		port = other.port;
		host = other.host;
		serverName = other.serverName;
		root = other.root;
		index = other.index;
		clientMaxBodySize = other.clientMaxBodySize;
		errorPages = other.errorPages;
		locations = other.locations;
	}
	return *this;
}

ServerConfig::~ServerConfig()
{
}

std::string ServerConfig::getErrorPage(int code) const
{
	std::map<int, std::string>::const_iterator it = errorPages.find(code);
	if (it != errorPages.end())
		return it->second;
	return "";
}

const LocationConfig* ServerConfig::findLocation(const std::string& path) const
{
	const LocationConfig* bestMatch = NULL;
	size_t bestMatchLength = 0;

	for (size_t i = 0; i < locations.size(); ++i)
	{
		const std::string& locPath = locations[i].getPath();
		size_t locLen = locPath.length();

		if (path.compare(0, locLen, locPath) == 0)
		{
			if (locLen > bestMatchLength)
			{
				bestMatch = &locations[i];
				bestMatchLength = locLen;
			}
		}
	}

	return bestMatch;
}
