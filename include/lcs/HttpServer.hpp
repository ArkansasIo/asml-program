// Minimal HTTP/1.1 server for the LCS REST API.
// Binds to localhost only; no physical equipment control is exposed.
#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <string>

namespace lcs::api
{

    struct HttpRequest
    {
        std::string method;
        std::string path;
        std::string body;
    };

    struct HttpResponse
    {
        int status{200};
        std::string contentType{"application/json"};
        std::string body;
    };

    using HttpHandler = std::function<HttpResponse(const HttpRequest &)>;

    // Blocking TCP server. Call serve() once; it runs until the process exits.
    class HttpServer
    {
    public:
        HttpServer(unsigned short port, HttpHandler handler);
        ~HttpServer();
        HttpServer(const HttpServer &) = delete;
        HttpServer &operator=(const HttpServer &) = delete;

        void serve(); // accept loop, blocks forever

    private:
        unsigned short port_;
        HttpHandler handler_;
    };

} // namespace lcs::api
