#include "Client.hpp"

Client::Client() { std::cout << "Se crea cliente" << std::endl;}

Client::~Client() { std::cout << "Se elimina cliente" << std::endl;}

Request Client::getRequest() const {
    return request;
}

// Setter para request (recibe un objeto Request)
void Client::setRequest(const Request& new_request) {
    request = new_request;
}