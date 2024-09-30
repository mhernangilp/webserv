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