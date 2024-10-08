#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "../config/ServerConfig.hpp"
#include <fstream> 
#include <sstream>
#include <string> 
#include <iostream> 
#include <sys/socket.h>
#include <unistd.h>
#include <map>

class Request {
    private:
        std::string method;
        std::string url;
        std::string http_version;
        std::string host;
        std::map<std::string, std::string> headers;
        std::string body;
        std::string file_name;
        int code;

    public:
    Request();
        Request(const std::string& raw_request);

        std::string getMethod() const;
        std::string getUrl() const;
        std::string getHttpVersion() const;
        std::string getHost() const;
        std::map<std::string, std::string> getHeaders() const;
        std::string getBody() const;
        std::string getFileName() const;
        int getCode();

    private:
        void parseRequest(const std::string& raw_request);
};

#endif
