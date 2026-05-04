#include "../../include/request/RequestHandler.hpp"
#include <sys/stat.h>
#include <iostream>

std::string RequestHandler::buildFilePath(
    const std::string& uri,
    const ServerConfig& server,
    const LocationConfig* location)
{
    std::string root;
    std::string uriPath = stripQueryString(uri);

    if (location && !location->getRoot().empty())
        root = location->getRoot();
    else
        root = server.getRoot();

    if (location && location->getRoot().empty())
    {
        const std::string& uploadStore = location->getUploadStore();
        if (!uploadStore.empty())
        {
            std::string locPath = location->getPath();
            if (!locPath.empty() && locPath[0] == '/')
                locPath = locPath.substr(1);

            std::string storePath = uploadStore;
            if (!storePath.empty() && storePath[0] == '/')
                storePath = storePath.substr(1);

            if (!locPath.empty() && locPath == storePath)
                root = joinPath(server.getRoot(), storePath);
        }
    }

    if (root.empty())
        root = ".";

    std::string relativePath = uriPath;
    if (location)
    {
        const std::string& locationPath = location->getPath();
        if (!locationPath.empty() && uriPath.find(locationPath) == 0)
        {
            relativePath = uriPath.substr(locationPath.length());
            if (relativePath.empty())
                relativePath = "/";
        }
    }

    if (!relativePath.empty() && relativePath[0] == '/')
        relativePath = relativePath.substr(1);

    std::string filePath = joinPath(root, relativePath);

    if (fileExists(filePath + ".html"))
        return filePath + ".html";

    std::string indexName;
    if (location && !location->getIndex().empty())
        indexName = location->getIndex();
    else if (!server.getIndex().empty())
        indexName = server.getIndex();

    if (!indexName.empty())
    {
        std::string indexPath = joinPath(filePath, indexName);
        if (fileExists(indexPath))
            return indexPath;
    }
    else if (fileExists(filePath + "/index.html"))
    {
        return filePath + "/index.html";
    }

    return filePath;
}

std::string RequestHandler::stripQueryString(const std::string& uri)
{
    size_t qPos = uri.find('?');
    std::cout << "uri : " << uri << std::endl;
    if (qPos == std::string::npos)
        return uri;
    return uri.substr(0, qPos);
}

bool RequestHandler::isCgiRequest(const std::string& uri, const LocationConfig* location)
{
    if (!location)
        return false;

    const std::map<std::string, std::string>& cgiExt = location->getCgiExtensions();
    std::map<std::string, std::string>::const_iterator it;

    if (cgiExt.empty())
        return false;

    std::string uriPath = stripQueryString(uri);
    // Need to be removed now that we have multiple cgi extensions
    // if (uriPath.length() < cgiExt.begin()->first.length())
    //     return false;
    std::string ext;
    for (it = cgiExt.begin(); it != cgiExt.end(); it++)
    {
        ext = it->first;
        if (uriPath.length() >= ext.length() &&
            uriPath.compare(uriPath.length() - ext.length(), ext.length(), ext) == 0)
            return true;
    }
    return false;
}

HttpResponse RequestHandler::makeErrorResponse(
    int code,
    const ServerConfig& server)
{
    std::string customPage = server.getErrorPage(code);
    if (!customPage.empty())
    {
        std::string fullPath = joinPath(server.getRoot(), customPage);
        return HttpResponse::makeErrorRes(code, fullPath);
    }

    return HttpResponse::makeErrorRes(code, "");
}

bool RequestHandler::fileExists(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

bool RequestHandler::directoryExists(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

std::string RequestHandler::joinPath(const std::string& a, const std::string& b)
{
    if (a.empty())
        return b;
    if (b.empty())
        return a;

    if (a[a.length() - 1] == '/')
        return a + b;
    return a + "/" + b;
}
