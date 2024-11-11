#include "ServerConfig.hpp"
#include <unistd.h>
#include <cstring>
#include <iostream>

#define RESET      "\033[0m"
#define RED        "\033[31m"
#define GREEN      "\033[32m"
#define YELLOW     "\033[33m"
#define ORANGE     "\033[38;5;214m"
#define PURPLE     "\033[35m"
#define BLUE       "\033[34m"
#define LIGHT_BLUE "\033[38;5;51m"

class ConfigParser {
    private:
        std::vector<ServerConfig> servers;
        LocationConfig current_location;
        std::string current_block;
        ServerConfig current_server;
        bool found_listen;
        bool found_server_name;
        bool found_host;
        bool found_max_body_size;
        bool found_root;
        bool found_index;

        int parseKey(const std::string& key, std::istringstream& iss);
        void trim(std::string& s);
        bool isValidIP(const std::string& ip);
        
    public:
        int parseConfig(const std::string& filename);
        const std::vector<ServerConfig>& getServerConfig() const;
};
