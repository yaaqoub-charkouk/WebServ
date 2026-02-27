#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include <vector>
#include <set>
#include <string>
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "ConfigException.hpp"

class Validator
{
private:
	const std::vector<ServerConfig>& _servers;

	// Validation functions
	void validateServers();
	void validateServer(const ServerConfig& server);
	void validateLocation(const LocationConfig& location);
	
	// Specific validation checks
	void checkDuplicatePorts();
	bool isValidPort(int port) const;
	bool isValidRedirectCode(int code) const;
	bool isValidMethod(const std::string& method) const;
	bool isValidCgiExtension(const std::string& ext) const;
	
public:
	explicit Validator(const std::vector<ServerConfig>& servers);
	~Validator();

	void validate();
};

#endif
