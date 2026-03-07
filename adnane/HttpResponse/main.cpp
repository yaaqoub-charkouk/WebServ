#include "HttpResponse.hpp"
#include <iostream>

int main()
{
    // 200 ok
    HttpResponse res = HttpResponse::makeFileRes("index.html");
    std::cout << res.getResponse() << std::endl;

    // 404 not found
    HttpResponse err = HttpResponse::makeErrorRes(404, "");
    std::cout << err.getResponse() << std::endl;

    // 301 redir
    HttpResponse red = HttpResponse::makeRedireRes(301, "/new-page");
    std::cout << red.getResponse() << std::endl;

    return 0;
}