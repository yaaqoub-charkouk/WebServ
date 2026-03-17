#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>

class LocationConfig
{
private:
	std::string path;
	std::vector<std::string> methods;
	std::string root;
	std::string index;
	bool autoindex;
	std::string uploadStore;
	std::string cgiExtension;
	int redirectCode;
	std::string redirectUrl;

public:
	LocationConfig();
	LocationConfig(const std::string& path);
	LocationConfig(const LocationConfig& other);
	LocationConfig& operator=(const LocationConfig& other);
	~LocationConfig();

	const std::string& getPath() const;
	const std::vector<std::string>& getMethods() const;
	const std::string& getRoot() const;
	const std::string& getIndex() const;
	bool getAutoindex() const;
	const std::string& getUploadStore() const;
	const std::string& getCgiExtension() const;
	int getRedirectCode() const;
	const std::string& getRedirectUrl() const;

	void setPath(const std::string& path);
	void addMethod(const std::string& method);
	void setRoot(const std::string& root);
	void setIndex(const std::string& index);
	void setAutoindex(bool autoindex);
	void setUploadStore(const std::string& uploadStore);
	void setCgiExtension(const std::string& cgiExtension);
	void setRedirect(int code, const std::string& url);

	bool hasMethod(const std::string& method) const;
	bool hasRedirect() const;
};

// Inline implementations for simple getters
inline const std::string& LocationConfig::getPath() const { return path; }
inline const std::vector<std::string>& LocationConfig::getMethods() const { return methods; }
inline const std::string& LocationConfig::getRoot() const { return root; }
inline const std::string& LocationConfig::getIndex() const { return index; }
inline bool LocationConfig::getAutoindex() const { return autoindex; }
inline const std::string& LocationConfig::getUploadStore() const { return uploadStore; }
inline const std::string& LocationConfig::getCgiExtension() const { return cgiExtension; }
inline int LocationConfig::getRedirectCode() const { return redirectCode; }
inline const std::string& LocationConfig::getRedirectUrl() const { return redirectUrl; }

inline void LocationConfig::setPath(const std::string& path) { this->path = path; }
inline void LocationConfig::addMethod(const std::string& method) { methods.push_back(method); }
inline void LocationConfig::setRoot(const std::string& root) { this->root = root; }
inline void LocationConfig::setIndex(const std::string& index) { this->index = index; }
inline void LocationConfig::setAutoindex(bool autoindex) { this->autoindex = autoindex; }
inline void LocationConfig::setUploadStore(const std::string& uploadStore) { this->uploadStore = uploadStore; }
inline void LocationConfig::setCgiExtension(const std::string& cgiExtension) { this->cgiExtension = cgiExtension; }
inline bool LocationConfig::hasRedirect() const { return redirectCode != 0; }

#endif
