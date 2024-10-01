#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include "LocationConfig.hpp"

class ServerConfig {
public:
    int port;
    std::string host;
    std::string server_name;
    std::map<int, std::string> error_page;
    int client_max_body_size;
    std::string index;
    std::string root;
    std::map<std::string, LocationConfig> locations;

    ServerConfig();

    void print() const;
    void addLocation(const std::string& path, const LocationConfig& location);
    bool isDeleteAllowed(const std::string& url) const;
};

#endif