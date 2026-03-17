#include "srcs/RequestHandler.hpp"
#include "adnane/HttpResponse/FileUtils.hpp"
#include "adnane/cgi/Cgi.hpp"
#include <sstream>
#include <fstream>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <unistd.h>

RequestHandler::RequestHandler(const ServerConfig& cfg)
    : config(cfg)
{
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string intToString(int n)
{
    std::ostringstream ss;
    ss << n;
    return ss.str();
}

std::string RequestHandler::resolveRoot(const LocationConfig* loc) const
{
    if (loc && !loc->getRoot().empty())
        return loc->getRoot();
    return config.getRoot();
}

std::string RequestHandler::resolveIndex(const LocationConfig* loc) const
{
    if (loc && !loc->getIndex().empty())
        return loc->getIndex();
    return config.getIndex();
}

std::string RequestHandler::resolveFilePath(const std::string& urlPath,
                                             const LocationConfig* loc) const
{
    std::string root = resolveRoot(loc);

    // Strip trailing slash from root
    while (!root.empty() && root[root.size() - 1] == '/')
        root.erase(root.size() - 1);

    // If the location has a path prefix, strip it from the URL path
    // so that the remainder maps into root
    std::string relPath = urlPath;
    if (loc)
    {
        const std::string& locPath = loc->getPath();
        if (relPath.compare(0, locPath.size(), locPath) == 0)
            relPath = relPath.substr(locPath.size());
        if (relPath.empty() || relPath[0] != '/')
            relPath = "/" + relPath;
    }

    return root + relPath;
}

bool RequestHandler::isCgiRequest(const std::string& filePath,
                                   const LocationConfig* loc) const
{
    if (!loc || loc->getCgiExtension().empty())
        return false;

    const std::string& ext = loc->getCgiExtension();
    if (filePath.size() < ext.size())
        return false;

    return filePath.compare(filePath.size() - ext.size(), ext.size(), ext) == 0;
}

HttpResponse RequestHandler::getErrorResponse(int code) const
{
    std::string errPage = config.getErrorPage(code);
    if (!errPage.empty())
    {
        // errPage is like /404.html → resolve relative to root
        std::string path = config.getRoot();
        while (!path.empty() && path[path.size() - 1] == '/')
            path.erase(path.size() - 1);
        path += errPage;
        FileInfo info = FileUtils::getInfo(path);
        if (info.exist && info.isFile && info.isReadable)
            return HttpResponse::makeErrorRes(code, path);
    }
    return HttpResponse::makeErrorRes(code, "");
}

// ---------------------------------------------------------------------------
// Main dispatch
// ---------------------------------------------------------------------------

HttpResponse RequestHandler::handle(const HttpRequest& req) const
{
    // Find matching location
    const LocationConfig* loc = config.findLocation(req.path);

    // Handle redirect
    if (loc && loc->hasRedirect())
        return HttpResponse::makeRedireRes(loc->getRedirectCode(), loc->getRedirectUrl());

    // Check allowed methods (if the location specifies methods)
    if (loc && !loc->getMethods().empty() && !loc->hasMethod(req.method))
        return getErrorResponse(405);

    // Default allowed methods when no location (or location has no method list)
    if (!loc || loc->getMethods().empty())
    {
        if (req.method != "GET" && req.method != "POST" && req.method != "DELETE"
            && req.method != "HEAD")
            return getErrorResponse(405);
    }

    // Check client_max_body_size
    if (config.getClientMaxBodySize() > 0 &&
        req.body.size() > config.getClientMaxBodySize())
        return getErrorResponse(413);

    if (req.method == "GET" || req.method == "HEAD")
        return handleGet(req, loc);
    if (req.method == "POST")
        return handlePost(req, loc);
    if (req.method == "DELETE")
        return handleDelete(req, loc);

    return getErrorResponse(405);
}

// ---------------------------------------------------------------------------
// GET / HEAD
// ---------------------------------------------------------------------------

HttpResponse RequestHandler::handleGet(const HttpRequest& req,
                                        const LocationConfig* loc) const
{
    std::string filePath = resolveFilePath(req.path, loc);
    FileInfo    info     = FileUtils::getInfo(filePath);

    if (!info.exist)
        return getErrorResponse(404);

    if (info.isDir)
    {
        // Ensure URL ends with /
        std::string normPath = req.path;
        if (normPath.empty() || normPath[normPath.size() - 1] != '/')
        {
            // Redirect to path/
            return HttpResponse::makeRedireRes(301, req.path + "/");
        }

        // Try index file
        std::string idxFile = filePath;
        if (idxFile[idxFile.size() - 1] != '/')
            idxFile += '/';
        idxFile += resolveIndex(loc);

        FileInfo idxInfo = FileUtils::getInfo(idxFile);
        if (idxInfo.exist && idxInfo.isFile && idxInfo.isReadable)
        {
            if (isCgiRequest(idxFile, loc))
            {
                std::string root = resolveRoot(loc);
                Cgi cgi(req, config, idxFile, root);
                std::string rawResp = cgi.execute();
                HttpResponse resp;
                resp.setStatus(200, "OK");
                resp.setBody(rawResp);
                return resp;
            }
            return HttpResponse::makeFileRes(idxFile);
        }

        // Autoindex
        if (loc && loc->getAutoindex())
            return HttpResponse::makeAutoindexRes(filePath, req.path);

        return getErrorResponse(403);
    }

    if (!info.isReadable)
        return getErrorResponse(403);

    // CGI?
    if (isCgiRequest(filePath, loc))
    {
        std::string root = resolveRoot(loc);
        Cgi         cgi(req, config, filePath, root);
        std::string rawResp = cgi.execute();
        // CGI returns full HTTP response — we relay it directly via body
        HttpResponse resp;
        resp.setStatus(200, "OK");
        resp.setBody(rawResp);
        return resp;
    }

    HttpResponse resp = HttpResponse::makeFileRes(filePath);
    if (req.method == "HEAD")
    {
        // HEAD: same headers, no body
        HttpResponse head;
        head.setStatus(200, "OK");
        head.setHeaders("Content-Type",
                        HttpResponse::getFileType(
                            HttpResponse::getFileExtension(filePath)));
        head.setHeaders("Content-Length", intToString(static_cast<int>(info.size)));
        return head;
    }
    return resp;
}

// ---------------------------------------------------------------------------
// POST
// ---------------------------------------------------------------------------

HttpResponse RequestHandler::handlePost(const HttpRequest& req,
                                         const LocationConfig* loc) const
{
    std::string filePath = resolveFilePath(req.path, loc);
    FileInfo    info     = FileUtils::getInfo(filePath);

    // CGI POST
    if (info.exist && info.isFile && isCgiRequest(filePath, loc))
    {
        std::string root = resolveRoot(loc);
        Cgi         cgi(req, config, filePath, root);
        std::string rawResp = cgi.execute();
        HttpResponse resp;
        resp.setStatus(200, "OK");
        resp.setBody(rawResp);
        return resp;
    }

    // File upload
    if (loc && !loc->getUploadStore().empty())
    {
        std::string uploadDir = loc->getUploadStore();
        if (uploadDir.empty() || uploadDir[uploadDir.size() - 1] != '/')
            uploadDir += '/';

        // Determine filename from URL path or Content-Disposition
        std::string filename;
        std::string cd = req.getHeader("content-disposition");
        if (!cd.empty())
        {
            size_t fnPos = cd.find("filename=\"");
            if (fnPos != std::string::npos)
            {
                fnPos += 10;
                size_t fnEnd = cd.find('"', fnPos);
                if (fnEnd != std::string::npos)
                    filename = cd.substr(fnPos, fnEnd - fnPos);
            }
        }
        if (filename.empty())
        {
            // Use last segment of path
            std::string p = req.path;
            size_t last = p.rfind('/');
            if (last != std::string::npos && last + 1 < p.size())
                filename = p.substr(last + 1);
        }
        if (filename.empty())
            filename = "upload";

        std::string destPath = uploadDir + filename;

        std::ofstream ofs(destPath.c_str(), std::ios::binary | std::ios::trunc);
        if (!ofs.is_open())
            return getErrorResponse(500);

        ofs.write(req.body.c_str(), static_cast<std::streamsize>(req.body.size()));
        ofs.close();

        HttpResponse resp;
        resp.setStatus(201, "Created");
        resp.setHeaders("Content-Length", "0");
        resp.setHeaders("Location", req.path);
        return resp;
    }

    // No upload store configured and no CGI — not allowed
    return getErrorResponse(403);
}

// ---------------------------------------------------------------------------
// DELETE
// ---------------------------------------------------------------------------

HttpResponse RequestHandler::handleDelete(const HttpRequest& req,
                                           const LocationConfig* loc) const
{
    std::string filePath = resolveFilePath(req.path, loc);
    FileInfo    info     = FileUtils::getInfo(filePath);

    if (!info.exist)
        return getErrorResponse(404);

    if (info.isDir)
        return getErrorResponse(403);

    if (std::remove(filePath.c_str()) != 0)
        return getErrorResponse(403);

    HttpResponse resp;
    resp.setStatus(204, "No Content");
    resp.setHeaders("Content-Length", "0");
    return resp;
}
