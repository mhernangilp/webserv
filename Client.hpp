#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include "methods/Request.hpp"

class Request;

class Client {
    private:
        Request request;
        std::string response;
        int index;
        time_t lastReadTime;

    public:
        Client();
        ~Client();

        Request getRequest() const;
        void setRequest(const Request& new_request);
        void setIndex(const int i);
        int getIndex();
        void setLastReadTime(time_t time);
        time_t getLastReadTime();
};

#endif