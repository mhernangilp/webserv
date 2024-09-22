#include "ServerConfig.hpp"

ServerConfig::ServerConfig() : port(8001), host("127.0.0.1"), server_name("localhost"), client_max_body_size(1024), index("index.html"), root("docs/kebab_web/") {
    error_page[404] = "custom_error/404.html";
    
    LocationConfig location = LocationConfig();
    location.location = "/";
    location.autoindex = false;
    location.allow_methods.push_back("DELETE");
    location.allow_methods.push_back("POST");
    location.allow_methods.push_back("GET");
    this->addLocation(location.location, location);
    
    location = LocationConfig();
    location.location = "/kebabs";
    location.autoindex = true;
    location.allow_methods.push_back("GET");
    location.allow_methods.push_back("POST");
    location.index = "kebab.html";
    this->addLocation(location.location, location);

    location = LocationConfig();
    location.location = "/secret-sauce";
    location.return_path = "/kebabs";
    this->addLocation(location.location, location);

    location = LocationConfig();
    location.location = "/cgi-bin";
    location.root = "./";
    location.allow_methods.push_back("GET");
    location.allow_methods.push_back("POST");
    location.allow_methods.push_back("DELETE");
    location.index = "time.py";
    this->addLocation(location.location, location);
}

void ServerConfig::print() const {
    std::cout << "--Server Config:\n";
    std::cout << "\tport: " << port << "\n";
    std::cout << "\tHost: " << host << "\n";
    std::cout << "\tServer Name: " << server_name << "\n";
    std::cout << "\tError Page:\n";
    for (std::map<int, std::string>::const_iterator it = error_page.begin(); it != error_page.end(); ++it) {
        std::cout << "\t\tError type: " << it->first << ", error path: " << it->second << std::endl;
    }
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