#include "../include/Validator.hpp"
#include <sstream>

Validator::Validator(const std::vector<ServerConfig>& servers)
	: _servers(servers)
{
}

Validator::~Validator()
{
}

void Validator::validate()
{
	if (_servers.empty())
		throw ValidatorException("No servers to validate");

	validateServers();
	checkDuplicatePorts();
}

void Validator::validateServers()
{
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		validateServer(_servers[i]);
	}
}

void Validator::validateServer(const ServerConfig& server)
{
	// Validate port
	int port = server.getPort();
	if (!isValidPort(port))
	{
		std::stringstream ss;
		ss << "Invalid port number: " << port << " (must be between 1 and 65535)";
		throw ValidatorException(ss.str());
	}

	// Validate root
	if (server.getRoot().empty())
		throw ValidatorException("Server root cannot be empty");

	// Validate client_max_body_size
	if (server.getClientMaxBodySize() == 0)
		throw ValidatorException("client_max_body_size must be greater than 0");

	// Validate host
	if (server.getHost().empty())
		throw ValidatorException("Server host cannot be empty");

	// Validate error pages
	const std::map<int, std::string>& errorPages = server.getErrorPages();
	for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
	{
		if (it->first < 100 || it->first > 599)
		{
			std::stringstream ss;
			ss << "Invalid error page code: " << it->first;
			throw ValidatorException(ss.str());
		}
		if (it->second.empty())
		{
			std::stringstream ss;
			ss << "Error page path cannot be empty for code " << it->first;
			throw ValidatorException(ss.str());
		}
	}

	// Validate locations
	const std::vector<LocationConfig>& locations = server.getLocations();
	for (size_t i = 0; i < locations.size(); ++i)
	{
		validateLocation(locations[i]);
	}
}

void Validator::validateLocation(const LocationConfig& location)
{
	// Validate path
	if (location.getPath().empty())
		throw ValidatorException("Location path cannot be empty");

	// Validate methods
	const std::vector<std::string>& methods = location.getMethods();
	for (size_t i = 0; i < methods.size(); ++i)
	{
		if (!isValidMethod(methods[i]))
		{
			std::stringstream ss;
			ss << "Invalid HTTP method: " << methods[i] << " (only GET, POST, DELETE allowed)";
			throw ValidatorException(ss.str());
		}
	}

	// Validate redirect
	if (location.hasRedirect())
	{
		int code = location.getRedirectCode();
		if (!isValidRedirectCode(code))
		{
			std::stringstream ss;
			ss << "Invalid redirect code: " << code << " (must be between 300 and 399)";
			throw ValidatorException(ss.str());
		}

		if (location.getRedirectUrl().empty())
			throw ValidatorException("Redirect URL cannot be empty");
	}

	// Validate CGI extension
	if (!location.getCgiExtension().empty())
	{
		if (!isValidCgiExtension(location.getCgiExtension()))
		{
			std::stringstream ss;
			ss << "CGI extension must start with dot: " << location.getCgiExtension();
			throw ValidatorException(ss.str());
		}
	}

	// Validate upload_store with POST method
	if (!location.getUploadStore().empty())
	{
		if (!location.hasMethod("POST"))
		{
			throw ValidatorException("upload_store requires POST method in location " + location.getPath());
		}
	}
}

void Validator::checkDuplicatePorts()
{
	std::set<int> ports;

	for (size_t i = 0; i < _servers.size(); ++i)
	{
		int port = _servers[i].getPort();
		
		if (ports.find(port) != ports.end())
		{
			std::stringstream ss;
			ss << "Duplicate port number: " << port;
			throw ValidatorException(ss.str());
		}
		
		ports.insert(port);
	}
}

bool Validator::isValidPort(int port) const
{
	return port > 0 && port <= 65535;
}

bool Validator::isValidRedirectCode(int code) const
{
	return code >= 300 && code <= 399;
}

bool Validator::isValidMethod(const std::string& method) const
{
	return method == "GET" || method == "POST" || method == "DELETE";
}

bool Validator::isValidCgiExtension(const std::string& ext) const
{
	return !ext.empty() && ext[0] == '.';
}
