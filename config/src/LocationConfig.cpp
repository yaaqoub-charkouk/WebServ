#include "../include/LocationConfig.hpp"

LocationConfig::LocationConfig()
	: _path(""),
	  _methods(),
	  _root(""),
	  _index(""),
	  _autoindex(false),
	  _uploadStore(""),
	  _cgiExtension(""),
	  _redirectCode(0),
	  _redirectUrl("")
{
}

LocationConfig::LocationConfig(const std::string& path)
	: _path(path),
	  _methods(),
	  _root(""),
	  _index(""),
	  _autoindex(false),
	  _uploadStore(""),
	  _cgiExtension(""),
	  _redirectCode(0),
	  _redirectUrl("")
{
}

LocationConfig::LocationConfig(const LocationConfig& other)
	: _path(other._path),
	  _methods(other._methods),
	  _root(other._root),
	  _index(other._index),
	  _autoindex(other._autoindex),
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
		_path = other._path;
		_methods = other._methods;
		_root = other._root;
		_index = other._index;
		_autoindex = other._autoindex;
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
	for (size_t i = 0; i < _methods.size(); ++i)
	{
		if (_methods[i] == method)
			return true;
	}
	return false;
}
