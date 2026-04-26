#include "../../include/cookie/Cookie.hpp"
#include <iostream>
#include <sstream>
Cookie::Cookie(): token(""), base("0123456789ABCDEF"), shouldSetCookie(false) {}



void    Cookie::checkRequest(const HttpRequest &req)
{
    // for (auto it = req.headers.begin(); it != req.headers.end(); it++)
    // {
    //     std::cout << it->first << "---" << it->second << std::endl;
    // }
    if (req.headers.find("cookie") != req.headers.end())
    {
        const std::map<std::string , std::string>::const_iterator it = req.headers.find("cookie");
        if (it != req.headers.end())
        {
            extractToken(it->second);
            if (tokens.find(token) != tokens.end() && time(NULL) - tokens.at(token) < HALF_HOUR)
            {
                shouldSetCookie = false;
            //     std::cout << "++++------------+++++++++++ COOKIE IS ALREADY SET\n" ;
            }
            else
                generateToken();
        }
        else
        // {
            generateToken();
        //     std::cout << "++++------------+++++++++++ COOKIE GENERATION\n" ;

        // }
    }
    else
    // {
        generateToken();
        // std::cout << "++++------------+++++++++++ COOKIE GENERATION\n" ;
    // }
}

// HTTP/1.0 200 OK
// Content-type: text/html
// Set-Cookie: theme=light
// Set-Cookie: sessionToken=abc123; Expires=Wed, 9 Jun 2021 10:18:14 GMT

void    Cookie::generateToken()
{
    expireTime = time(NULL);
    size_t t = expireTime;
    token = "";

    for (int i = 0; i < 15; i++)
    {
        std::stringstream ss;
        ss << base.at(t % 16);
        token.append(ss.str());
        t /= 16;
    }
    tokens.insert(std::make_pair(token, expireTime));
    shouldSetCookie = true;
    key = "Set-Cookie";
    value =  "sessionToken=" + token + "; Path=/; HttpOnly";
}


// GET /spec.html HTTP/1.1
// Host: www.example.org
// Cookie: theme=light; sessionToken=abc123

void    Cookie::extractToken(const std::string &cookie)
{
    size_t pos = cookie.find("sessionToken=");
    if (pos != std::string::npos)
        token = cookie.substr(pos + 13, 16);
    else
        token = "";
}


