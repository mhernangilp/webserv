#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include "Request.hpp"

class Request;

class Client {
    private:
        Request request;
        std::string response;

    public:
        Client();
        ~Client();

        Request getRequest() const;
        void setRequest(const Request& new_request);
};

#endif