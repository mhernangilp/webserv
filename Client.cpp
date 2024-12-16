#include "Client.hpp"

Client::Client() {}

Client::~Client() {}

Request Client::getRequest() const {
    return request;
}

// Setter para request (recibe un objeto Request)
void Client::setRequest(const Request& new_request) {
    if (new_request.isValid() == false) {
        std::cerr << "Error. Request is null" << std::endl;
        return;
    }
    request = new_request;
}

void Client::setIndex(const int i) {
    index = i;
}

int Client::getIndex() {
    return index;
}

void Client::setLastReadTime(time_t time) {
    lastReadTime = time;
}
time_t Client::getLastReadTime() {
    return lastReadTime;
}