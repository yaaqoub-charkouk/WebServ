#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include "LocationConfig.hpp"

class ServerConfig
{
private:
	int port;
	std::string host; // should i handle 
	std::string serverName; // out of scope ghi haydo . 
	std::string root;
	std::string index;
	size_t clientMaxBodySize;
	std::map<int, std::string> errorPages;
	std::vector<LocationConfig> locations;

public:
	ServerConfig();
	ServerConfig(const ServerConfig& other);
	ServerConfig& operator=(const ServerConfig& other);
	~ServerConfig();

	int getPort() const;
	const std::string& getHost() const;
	const std::string& getServerName() const;
	const std::string& getRoot() const;
	const std::string& getIndex() const;
	size_t getClientMaxBodySize() const;
	const std::map<int, std::string>& getErrorPages() const;
	const std::vector<LocationConfig>& getLocations() const;

	void setPort(int port);
	void setHost(const std::string& host);
	void setServerName(const std::string& serverName);
	void setRoot(const std::string& root);
	void setIndex(const std::string& index);
	void setClientMaxBodySize(size_t size);
	void addErrorPage(int code, const std::string& path);
	void addLocation(const LocationConfig& location);

	std::string getErrorPage(int code) const;
	const LocationConfig* findLocation(const std::string& path) const;
};

// Inline implementations for simple getters
inline int ServerConfig::getPort() const { return port; }
inline const std::string& ServerConfig::getHost() const { return host; }
inline const std::string& ServerConfig::getServerName() const { return serverName; }
inline const std::string& ServerConfig::getRoot() const { return root; }
inline const std::string& ServerConfig::getIndex() const { return index; }
inline size_t ServerConfig::getClientMaxBodySize() const { return clientMaxBodySize; }
inline const std::map<int, std::string>& ServerConfig::getErrorPages() const { return errorPages; }
inline const std::vector<LocationConfig>& ServerConfig::getLocations() const { return locations; }

inline void ServerConfig::setPort(int port) { this->port = port; }
inline void ServerConfig::setHost(const std::string& host) { this->host = host; }
inline void ServerConfig::setServerName(const std::string& serverName) { this->serverName = serverName; }
inline void ServerConfig::setRoot(const std::string& root) { this->root = root; }
inline void ServerConfig::setIndex(const std::string& index) { this->index = index; }
inline void ServerConfig::setClientMaxBodySize(size_t size) { this->clientMaxBodySize = size; }
inline void ServerConfig::addErrorPage(int code, const std::string& path) { errorPages[code] = path; }
inline void ServerConfig::addLocation(const LocationConfig& location) { locations.push_back(location); }

#endif
