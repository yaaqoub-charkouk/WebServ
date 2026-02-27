#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include "LocationConfig.hpp"

class ServerConfig
{
private:
	int _port;
	std::string _host;
	std::string _serverName;
	std::string _root;
	std::string _index;
	size_t _clientMaxBodySize;
	std::map<int, std::string> _errorPages;
	std::vector<LocationConfig> _locations;

public:
	ServerConfig();
	ServerConfig(const ServerConfig& other);
	ServerConfig& operator=(const ServerConfig& other);
	~ServerConfig();

	// Getters
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

	// Helpers
	std::string getErrorPage(int code) const;
	const LocationConfig* findLocation(const std::string& path) const;
};

// Inline implementations for simple getters
inline int ServerConfig::getPort() const { return _port; }
inline const std::string& ServerConfig::getHost() const { return _host; }
inline const std::string& ServerConfig::getServerName() const { return _serverName; }
inline const std::string& ServerConfig::getRoot() const { return _root; }
inline const std::string& ServerConfig::getIndex() const { return _index; }
inline size_t ServerConfig::getClientMaxBodySize() const { return _clientMaxBodySize; }
inline const std::map<int, std::string>& ServerConfig::getErrorPages() const { return _errorPages; }
inline const std::vector<LocationConfig>& ServerConfig::getLocations() const { return _locations; }

inline void ServerConfig::setPort(int port) { _port = port; }
inline void ServerConfig::setHost(const std::string& host) { _host = host; }
inline void ServerConfig::setServerName(const std::string& serverName) { _serverName = serverName; }
inline void ServerConfig::setRoot(const std::string& root) { _root = root; }
inline void ServerConfig::setIndex(const std::string& index) { _index = index; }
inline void ServerConfig::setClientMaxBodySize(size_t size) { _clientMaxBodySize = size; }
inline void ServerConfig::addErrorPage(int code, const std::string& path) { _errorPages[code] = path; }
inline void ServerConfig::addLocation(const LocationConfig& location) { _locations.push_back(location); }

#endif
