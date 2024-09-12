#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

class Client {
    private:
        std::string request;
        std::string response;

    public:
        Client();
        ~Client();
};

#endif