------WebKitFormBoundaryXUJ6fWRBIc17OkLF
Content-Disposition: form-data; name="file"; filename="Client.hpp"
Content-Type: application/octet-stream

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include "methods/Request.hpp"

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
------WebKitFormBoundaryXUJ6fWRBIc17OkLF--
