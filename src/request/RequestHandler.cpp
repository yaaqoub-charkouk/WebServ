#include "request/RequestHandler.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

HttpResponse RequestHandler::handleRequest(
    const std::string& method,
    const std::string& uri,
    const std::string& body,
    const ServerConfig& server,
    size_t contentLength)
{
    if (contentLength > server.getClientMaxBodySize())
    {
        return makeErrorResponse(413, server);
    }

    const LocationConfig* location = findLocation(uri, server);

    if (method == "GET")
        return handleGet(uri, server, location);
    else if (method == "POST")
        return handlePost(uri, body, server, location);
    else if (method == "DELETE")
        return handleDelete(uri, server, location);

    return makeErrorResponse(405, server);
}

const LocationConfig* RequestHandler::findLocation(
    const std::string& uri,
    const ServerConfig& server)
{
    const std::vector<LocationConfig>& locations = server.getLocations();
    const LocationConfig* bestMatch = 0;
    size_t bestLength = 0;

    for (size_t i = 0; i < locations.size(); ++i)
    {
        const std::string& locPath = locations[i].getPath();
        size_t locLen = locPath.length();

        if (uri.find(locPath) == 0 && locLen > bestLength)
        {
            bestMatch = &locations[i];
            bestLength = locLen;
        }
    }

    return bestMatch;
}

HttpResponse RequestHandler::handleGet(
    const std::string& uri,
    const ServerConfig& server,
    const LocationConfig* location)
{
    if (location && location->hasRedirect())
    {
        return HttpResponse::makeRedireRes(
            location->getRedirectCode(),
            location->getRedirectUrl());
    }

    std::string filePath = buildFilePath(uri, server, location);

    if (!fileExists(filePath))
        return makeErrorResponse(404, server);

    if (directoryExists(filePath))
    {
        if (location && location->getAutoindex())
        {
            if (location->getIndex().empty())
                return HttpResponse::makeFileRes(filePath + "/index.html");
            return HttpResponse::makeFileRes(filePath + "/" + location->getIndex());
        }
        return makeErrorResponse(403, server);
    }

    return HttpResponse::makeFileRes(filePath);
}

HttpResponse RequestHandler::handlePost(
    const std::string& uri,
    const std::string& body,
    const ServerConfig& server,
    const LocationConfig* location)
{
    if (!location || location->getUploadStore().empty())
    {
        return makeErrorResponse(403, server);
    }

    if (!location->hasMethod("POST"))
    {
        return makeErrorResponse(405, server);
    }

    std::string uploadDir = location->getUploadStore();
    if (uploadDir.empty())
        uploadDir = server.getRoot();

    std::string filename = "upload_" + uri;
    std::string savePath = joinPath(uploadDir, filename);

    std::ofstream outfile(savePath.c_str(), std::ios::binary);
    if (!outfile.is_open())
        return makeErrorResponse(500, server);

    outfile << body;
    outfile.close();

    HttpResponse response;
    response.setStatus(201, "Created");
    response.setHeaders("Content-Type", "text/plain");
    response.setBody("File uploaded successfully");
    return response;
}

HttpResponse RequestHandler::handleDelete(
    const std::string& uri,
    const ServerConfig& server,
    const LocationConfig* location)
{
    if (location && !location->hasMethod("DELETE"))
        return makeErrorResponse(405, server);

    std::string filePath = buildFilePath(uri, server, location);

    if (!fileExists(filePath) || directoryExists(filePath))
        return makeErrorResponse(404, server);

    if (unlink(filePath.c_str()) != 0)
        return makeErrorResponse(500, server);

    HttpResponse response;
    response.setStatus(204, "No Content");
    response.setHeaders("Content-Type", "text/plain");
    response.setBody("");
    return response;
}

std::string RequestHandler::buildFilePath(
    const std::string& uri,
    const ServerConfig& server,
    const LocationConfig* location)
{
    std::string root;

    if (location && !location->getRoot().empty())
        root = location->getRoot();
    else
        root = server.getRoot();

    if (root.empty())
        root = ".";

    std::string filePath = joinPath(root, uri);

    if (fileExists(filePath + ".html"))
        return filePath + ".html";
    else if (fileExists(filePath + "/index.html"))
        return filePath + "/index.html";

    return filePath;
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
    if (a.empty()) return b;
    if (b.empty()) return a;

    if (a[a.length() - 1] == '/')
        return a + b;
    return a + "/" + b;
}
