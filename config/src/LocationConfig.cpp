#include "../include/LocationConfig.hpp"

LocationConfig::LocationConfig()
	: path(""),
	  methods(),
	  root(""),
	  index(""),
	  autoindex(false),
	  _uploadStore(""),
	  _cgiExtension(""),
	  _redirectCode(0),
	  _redirectUrl("")
{
}

LocationConfig::LocationConfig(const std::string& path)
	: path(path),
	  methods(),
	  root(""),
	  index(""),
	  autoindex(false),
	  _uploadStore(""),
	  _cgiExtension(""),
	  _redirectCode(0),
	  _redirectUrl("")
{
}

LocationConfig::LocationConfig(const LocationConfig& other)
	: path(other.path),
	  methods(other.methods),
	  root(other.root),
	  index(other.index),
	  autoindex(other.autoindex),
	  _uploadStore(other._uploadStore),
	  _cgiExtension(other._cgiExtension),
	  _redirectCode(other._redirectCode),
	  _redirectUrl(other._redirectUrl)
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
		_uploadStore = other._uploadStore;
		_cgiExtension = other._cgiExtension;
		_redirectCode = other._redirectCode;
		_redirectUrl = other._redirectUrl;
	}
	return *this;
}

LocationConfig::~LocationConfig()
{
}

void LocationConfig::setRedirect(int code, const std::string& url)
{
	_redirectCode = code;
	_redirectUrl = url;
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
