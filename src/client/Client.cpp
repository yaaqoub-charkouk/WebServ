# include "../../include/client/Client.hpp"



Client::Client(const ServerConfig& serverConfig,  int clientPort, std::string clientAddress)
                 : serverConfig(serverConfig), clientPort(clientPort), clientAddress(clientAddress),
                  state(READING_HEADERS), isCgi(false), isCgiResponseError(false), cgi(NULL), bytes_sent(0) {
    request.contentLength = 0;
    error_code = 400;

    // uint32_t ip = ntohl(client.sin_addr.s_addr);
    // std::ostringstream oss;
    // oss << ((ip >> 24) & 0xFF) << "."
    //     << ((ip >> 16) & 0xFF) << "."
    //     << ((ip >> 8) & 0xFF) << "."
    //     << (ip & 0xFF);
    // std::string ipStr = oss.str();
}

Client::~Client()
{
    if (cgi != NULL)
    {
        delete cgi;
        cgi = NULL;
    }
}


void Client::parseRequest()
{
    if (state == READING_HEADERS)
    {
        parseRequestHeaders();
    }

    if (state == READING_BODY)
    {
        parseRequestBody();
    }
}

void    Client::parseRequestHeaders()
{
    size_t  pos = request_str.find("\r\n\r\n");
    if (pos == std::string::npos) {
        return ;
    }
    header_end_pos = pos;

    header_str = request_str.substr(0, header_end_pos);

    std::istringstream header_stream(header_str);

    // READING_HEADERS
    std::string line;
    if (std::getline(header_stream, line)) // first line
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        
        std::istringstream streamLine(line);
        std::string extra;
        if (!(streamLine >> request.method >> request.uri >> request.version) || (streamLine >> extra)) {
            state = ERROR;
            error_code = 400;
            return ;
        };
        if (!(request.method == "GET" || request.method == "POST" || request.method == "DELETE")) { // || PUT
            state = ERROR;
            error_code = 501;
            return ;
        }
    }
    else {
        state = ERROR;
        error_code = 400;
        return ;
    }

    // parsing headers
    while (std::getline(header_stream, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (line.empty())
            break;
        
        size_t  p = line.find(":");
        if (p == std::string::npos) {
            state = ERROR;
            error_code = 400;
            return ;
        }
        std::string key = line.substr(0, p);
        std::string value = line.substr(p + 1);
        key = trim(key);
        value = trim(value);

        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        
        request.headers[key] = value;
    }
    if (request.headers.find("content-length") != request.headers.end())
        state = READING_BODY;
    else
        state = COMPLETE;
}

void    Client::parseRequestBody()
{
    if (request.headers.find("content-length") != request.headers.end())
    {
        if (request.method == "GET") {
            state = ERROR; // !!!!!! still need to check
            return ;
        }
        std::istringstream length_stream(request.headers["content-length"]);
        size_t  length;
        char extra;
        if (!(length_stream >> length) || (length_stream >> extra)) {
            state = ERROR;
            error_code = 400;
            return ;
        }
        size_t client_max_body_size = serverConfig.getClientMaxBodySize(); // using serverConfig instead
        if (length > client_max_body_size) {
            state = ERROR;
            error_code = 413;
            return ;
        }

        request.contentLength = length;
        size_t body_start = header_end_pos + 4;
        if (request_str.size() < body_start + request.contentLength) {
            // state = INCOMPLETE;
            return ;
        }
        request.body.clear();
        request.body = request_str.substr(body_start, request.contentLength);
    }
    state = COMPLETE;
}

std::string trim(const std::string& s)
{
    size_t start = 0;
    while (start < s.size() && (s[start] == ' ' || s[start] == '\t'))
        start++;

    size_t end = s.size();
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t'))
        end--;

    return s.substr(start, end - start);
}

CgiClient::CgiClient(int client_fd, Cgi* cgi, bool is_write) : is_write_end(is_write), http_client_fd(client_fd), cgi(cgi), processed(false) { }