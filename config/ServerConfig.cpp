#include "ServerConfig.hpp"

ServerConfig::ServerConfig() : listen(8001), host("127.0.0.1"), server_name("localhost"), client_max_body_size(1024), index("index.html"), root("docs/kebab_web/") {
    error_page = "404";
    //LocationConfig location = LocationConfig();
    //location.autoindex = false;
}

void ServerConfig::print() const {
    std::cout << "--Server Config:\n";
    std::cout << "\tListen: " << listen << "\n";
    std::cout << "\tHost: " << host << "\n";
    std::cout << "\tServer Name: " << server_name << "\n";
    std::cout << "\tError Page: " << error_page << "\n";
    std::cout << "\tMax Body Size: " << client_max_body_size << " bytes\n";
    std::cout << "\tRoot: " << root << "\n";
    std::cout << "\tIndex: " << index << "\n";
    std::cout << "\n--Locations:\n";
    for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        std::cout << "Location: '" << it->first << "'\n";
        it->second.print();
    }
}

void ServerConfig::addLocation(const std::string& path, const LocationConfig& location) {
    locations[path] = location;
}