#include "config/LocationConfig.hpp"

LocationConfig::LocationConfig()
	: path(""),
	  methods(),
	  root(""),
	  index(""),
	  autoindex(false),
	  uploadStore(""),
	  cgiExtensions(),
	  redirectCode(0),
	  redirectUrl("")
{
}

LocationConfig::LocationConfig(const std::string& path)
	: path(path),
	  methods(),
	  root(""),
	  index(""),
	  autoindex(false),
	  uploadStore(""),
	  cgiExtensions(),
	  redirectCode(0),
	  redirectUrl("")
{
}

LocationConfig::LocationConfig(const LocationConfig& other)
	: path(other.path),
	  methods(other.methods),
	  root(other.root),
	  index(other.index),
	  autoindex(other.autoindex),
	  uploadStore(other.uploadStore),
	  cgiExtensions(other.cgiExtensions),
	  redirectCode(other.redirectCode),
	  redirectUrl(other.redirectUrl)
{
}

LocationConfig& LocationConfig::operator=(const LocationConfig& other)
{
	if (this != &other)
	{
		path = other.path;
		methods = other.methods;
		root = other.root;
		index = other.index;
		autoindex = other.autoindex;
		uploadStore = other.uploadStore;
		cgiExtensions = other.cgiExtensions;
		redirectCode = other.redirectCode;
		redirectUrl = other.redirectUrl;
	}
	return *this;
}

LocationConfig::~LocationConfig()
{
}

void LocationConfig::setRedirect(int code, const std::string& url)
{
	redirectCode = code;
	redirectUrl = url;
}

bool LocationConfig::hasMethod(const std::string& method) const
{
	for (size_t i = 0; i < methods.size(); ++i)
	{
		if (methods[i] == method)
			return true;
	}
	return false;
}
