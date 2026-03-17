#include "srcs/HttpRequest.hpp"
#include <sstream>
#include <vector>
#include <cctype>
#include <algorithm>

HttpRequest::HttpRequest()
    : method(""), path(""), queryString(""), version(""),
      headers(), body(""), complete(false), errorCode(0)
{
}

static std::string strToLower(const std::string& s)
{
    std::string result = s;
    for (size_t i = 0; i < result.size(); ++i)
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    return result;
}

static std::string trimStr(const std::string& s)
{
    size_t start = 0;
    size_t end   = s.size();
    while (start < end && std::isspace(static_cast<unsigned char>(s[start])))
        ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
        --end;
    return s.substr(start, end - start);
}

static std::string urlDecode(const std::string& s)
{
    std::string result;
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '%' && i + 2 < s.size() &&
            std::isxdigit(static_cast<unsigned char>(s[i + 1])) &&
            std::isxdigit(static_cast<unsigned char>(s[i + 2])))
        {
            char hex[3] = { s[i + 1], s[i + 2], '\0' };
            result += static_cast<char>(std::strtol(hex, NULL, 16));
            i += 2;
        }
        else if (s[i] == '+')
            result += ' ';
        else
            result += s[i];
    }
    return result;
}

bool HttpRequest::parse(const std::string& raw)
{
    size_t headerEnd = raw.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
    {
        complete = false;
        return false;
    }

    std::string headerSection = raw.substr(0, headerEnd);

    // Parse request line
    size_t lineEnd = headerSection.find("\r\n");
    if (lineEnd == std::string::npos)
    {
        errorCode = 400;
        return false;
    }

    std::string requestLine = headerSection.substr(0, lineEnd);
    std::istringstream lineStream(requestLine);

    if (!(lineStream >> method >> path >> version))
    {
        errorCode = 400;
        return false;
    }

    // Check version
    if (version != "HTTP/1.0" && version != "HTTP/1.1")
    {
        errorCode = 400;
        return false;
    }

    // Validate method (we only accept these)
    if (method != "GET" && method != "POST" && method != "DELETE" && method != "HEAD")
    {
        errorCode = 405;
        return false;
    }

    // Split path and query string
    size_t qpos = path.find('?');
    if (qpos != std::string::npos)
    {
        queryString = path.substr(qpos + 1);
        path        = path.substr(0, qpos);
    }

    // URL-decode path
    path = urlDecode(path);

    // Normalize path: resolve '..' and '.' segments, collapse slashes.
    // Uses a segment stack so 'a..b' is treated as a normal segment (not traversal).
    {
        // Preserve trailing slash (signals directory intent)
        bool trailingSlash = path.size() > 1 &&
                             path[path.size() - 1] == '/';

        std::vector<std::string> segments;
        size_t j   = 0;
        size_t len = path.size();

        while (j < len)
        {
            // Skip slashes
            while (j < len && path[j] == '/')
                ++j;
            if (j >= len)
                break;

            // Read segment
            size_t segStart = j;
            while (j < len && path[j] != '/')
                ++j;
            std::string seg = path.substr(segStart, j - segStart);

            if (seg == "..")
            {
                if (!segments.empty())
                    segments.erase(segments.end() - 1);
                // If already at root, ".." is a no-op
            }
            else if (seg != "." && !seg.empty())
            {
                segments.push_back(seg);
            }
        }

        path.clear();
        for (size_t k = 0; k < segments.size(); ++k)
        {
            path += '/';
            path += segments[k];
        }
        if (path.empty())
            path = "/";
        else if (trailingSlash)
            path += '/';
    }

    // Parse headers
    size_t pos = lineEnd + 2;
    while (pos < headerSection.size())
    {
        size_t nextLine = headerSection.find("\r\n", pos);
        if (nextLine == std::string::npos)
            nextLine = headerSection.size();

        std::string headerLine = headerSection.substr(pos, nextLine - pos);
        size_t colon = headerLine.find(':');
        if (colon != std::string::npos)
        {
            std::string key = strToLower(trimStr(headerLine.substr(0, colon)));
            std::string val = trimStr(headerLine.substr(colon + 1));
            headers[key]    = val;
        }

        pos = nextLine + 2;
    }

    // Body
    body = raw.substr(headerEnd + 4);

    complete  = true;
    errorCode = 0;
    return true;
}

std::string HttpRequest::getHeader(const std::string& name) const
{
    std::string lname = strToLower(name);
    std::map<std::string, std::string>::const_iterator it = headers.find(lname);
    if (it != headers.end())
        return it->second;
    return "";
}

size_t HttpRequest::getContentLength() const
{
    std::string cl = getHeader("content-length");
    if (cl.empty())
        return 0;
    std::istringstream ss(cl);
    size_t len = 0;
    ss >> len;
    return len;
}

bool HttpRequest::isRequestComplete(const std::string& raw)
{
    size_t headerEnd = raw.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return false;

    std::string headersSection = raw.substr(0, headerEnd);

    // Look for content-length header (case-insensitive)
    std::string headersLower = strToLower(headersSection);
    size_t clPos = headersLower.find("content-length:");
    if (clPos == std::string::npos)
        return true; // No body expected

    size_t valueStart = clPos + 15;
    while (valueStart < headersLower.size() && headersLower[valueStart] == ' ')
        ++valueStart;

    size_t valueEnd = headersLower.find("\r\n", valueStart);
    if (valueEnd == std::string::npos)
        valueEnd = headersLower.size();

    std::string clValue = headersSection.substr(valueStart, valueEnd - valueStart);
    std::istringstream ss(clValue);
    size_t contentLength = 0;
    ss >> contentLength;

    size_t bodyReceived = raw.size() - (headerEnd + 4);
    return bodyReceived >= contentLength;
}
