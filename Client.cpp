#include "Client.hpp"

Client::Client() {}

Client::~Client() {}

Request Client::getRequest() const {
    return request;
}

// Setter para request (recibe un objeto Request)
void Client::setRequest(const Request& new_request) {
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
long int Client::getLastReadTime() {
    return lastReadTime;
}