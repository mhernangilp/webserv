#include "LocationConfig.hpp"

class ServerConfig {
public:
    int listen;
    std::string host;
    std::string server_name;
    std::string error_page;
    int client_max_body_size;
    std::string root;
    std::string index;
    std::map<std::string, LocationConfig> locations;

    ServerConfig();

    void print() const;
    void addLocation(const std::string& path, const LocationConfig& location);
};