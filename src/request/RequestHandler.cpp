#include "../../include/request/RequestHandler.hpp"
#include <unistd.h>
#include <fstream>
#include <sstream>
#include "../../include/cgi/Cgi.hpp"
#include "../../include/client/Client.hpp"

HttpResponse RequestHandler::handleRequest(const Client& client)
{
    const HttpRequest& request = client.request;
    const ServerConfig& server = client.serverConfig;

    if (request.contentLength > server.getClientMaxBodySize())
    {
        return makeErrorResponse(413, server);
    }

    const LocationConfig* location = findLocation(request.uri, server);
    
    // you don't need this anymore TAHALLA :
    // if ((request.method == "GET" || request.method == "POST") && isCgiRequest(request.uri, location))
    //     return handleCgi(request.method, request.uri, request.body, server, location);

    if (request.method == "GET")
        return handleGet(request.uri, server, location);
    else if (request.method == "POST")
        return handlePost(request.uri, request.body, server, location);
    else if (request.method == "DELETE")
        return handleDelete(request.uri, server, location);

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

    if (directoryExists(filePath))// This condition needs to be checked first for the autoindex else it will always return 404
    {
        if (location && location->getAutoindex())
        {
            if (location->getIndex().empty())
                return HttpResponse::makeAutoindexRes(filePath);//I changed this because I need the file path not the uri
            return HttpResponse::makeFileRes(filePath + "/" + location->getIndex());
        }
        return makeErrorResponse(403, server);
    }
    
    if (!fileExists(filePath))
        return makeErrorResponse(404, server);

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

// changed
void RequestHandler::handleCgi(Client& client, const LocationConfig* location)
{
    if (!location) {
        client.response = makeErrorResponse(404, client.serverConfig);
        client.isCgiResponseError = true;
        return ;
    }

    if (!location->hasMethod(client.request.method)) {
        client.response = makeErrorResponse(405, client.serverConfig);
        client.isCgiResponseError = true;
        return ;
    }
    std::string scriptPath = buildFilePath(client.request.uri, client.serverConfig, location);
    if (!fileExists(scriptPath)) {
        client.response = makeErrorResponse(404, client.serverConfig);
        client.isCgiResponseError = true;
        return ;
    }
    if (directoryExists(scriptPath)) {
        client.response = makeErrorResponse(403, client.serverConfig);
        client.isCgiResponseError = true;
        return ;
    }

    HttpRequest cgiRequest;
    cgiRequest.method = client.request.method;
    cgiRequest.uri = client.request.uri;
    cgiRequest.body = client.request.body;
    cgiRequest.contentLength = client.request.body.size();
    cgiRequest.serverName = client.serverConfig.getServerName();
    cgiRequest.serverPort = client.serverConfig.getPort();
    std::ostringstream contentLength;
    contentLength << cgiRequest.contentLength;
    cgiRequest.headers["Content-Length"] = contentLength.str();

    Cgi* cgi_init = new Cgi(cgiRequest, scriptPath); // allocate;
    // assign cgi to client 
    client.cgi = cgi_init;

    
    // return cgi_init.getResponse(); // need to be deleted !!!
}
