#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include "LocationConfig.hpp"

class ServerConfig {
public:
    int number;
    int port;
    std::string host;
    std::string server_name;
    std::map<int, std::string> error_page;
    long unsigned int client_max_body_size;
    std::string index;
    std::string root;
    std::map<std::string, LocationConfig> locations;
    char** method_location;
    char** methods;

    ServerConfig();

    void print() const;
    void addLocation(const std::string& path, const LocationConfig& location);
    bool isMethodAllowed(const std::string& location, char m) const;
    void struct_method_allowed();
    void clearMethods();
};

#endif