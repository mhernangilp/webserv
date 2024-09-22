#include "ServerConfig.hpp"

ServerConfig::ServerConfig() : listen(8001), host("127.0.0.1"), client_max_body_size(1024), index("index.html") {}

void ServerConfig::print() const {
    std::cout << "Server Config:\n";
    std::cout << "\tListen: " << listen << "\n";
    std::cout << "\tHost: " << host << "\n";
    std::cout << "\tServer Name: " << server_name << "\n";
    std::cout << "\tError Page: " << error_page << "\n";
    std::cout << "\tMax Body Size: " << client_max_body_size << " bytes\n";
    std::cout << "\tRoot: " << root << "\n";
    std::cout << "\tIndex: " << index << "\n";
    std::cout << "Locations:\n";
    for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        std::cout << "\tLocation: " << it->first << "\n";  // it->first es la clave (la ruta de la location)
        it->second.print();  // it->second es el objeto LocationConfig
    }
}

void ServerConfig::addLocation(const std::string& path, const LocationConfig& location) {
    locations[path] = location;
}