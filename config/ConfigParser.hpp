#include "ServerConfig.hpp"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define ORANGE  "\033[38;5;214m"
#define PURPLE  "\033[35m"


class ConfigParser {
private:
    ServerConfig server;
    LocationConfig current_location;
    std::string current_block;

public:
    int parseConfig(const std::string& filename);

    void parseKey(const std::string& key, std::istringstream& iss);

    void trim(std::string& s);
    ServerConfig getServerConfig() const;
};
