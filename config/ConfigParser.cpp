#include "ConfigParser.hpp"

int ConfigParser::parseConfig(const std::string& filename) {
    std::ifstream config_file(filename.c_str());
    std::string line;

    if (!config_file.is_open()) {
        std::cout << ORANGE << "Cannot open config file, using default one ..." << RESET << std::endl;
        std::string default_conf = "config/default.conf";
        config_file.open(default_conf.c_str());
        if (!config_file.is_open()) {
            std::cerr << RED <<"Error: cannot open default config file" << RESET <<std::endl;
            return 1;
        }
    }

    while (std::getline(config_file, line)) {
        // Eliminar comentarios y espacios en blanco
        line = line.substr(0, line.find("#"));
        trim(line);
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "server") {
            current_block = "server";
        } else if (key == "location") {
            std::string location_path;
            iss >> location_path;
            current_block = "location";
            current_location = LocationConfig(); // Reiniciar localización
        } else if (key == "}") {
            if (current_block == "location") {
                server.addLocation(current_location.root, current_location);
                current_block = "server"; // Volver al bloque del servidor
            }
        } else {
            parseKey(key, iss);
        }
    }
    return 0;
}

void ConfigParser::parseKey(const std::string& key, std::istringstream& iss) {
    std::string value;

    if (current_block == "server") {
        if (key == "listen") {
            iss >> server.listen;
        } else if (key == "host") {
            iss >> server.host;
        } else if (key == "server_name") {
            iss >> server.server_name;
        } else if (key == "error_page") {
            iss >> server.error_page;
        } else if (key == "client_max_body_size") {
            iss >> server.client_max_body_size;
        } else if (key == "root") {
            iss >> server.root;
        } else if (key == "index") {
            iss >> server.index;
        }
    } else if (current_block == "location") {
        if (key == "root") {
            iss >> current_location.root;
        } else if (key == "autoindex") {
            iss >> current_location.autoindex;
        } else if (key == "allow_methods") {
            std::string method;
            while (iss >> method) {
                current_location.allow_methods.push_back(method);
            }
        } else if (key == "index") {
            iss >> current_location.index;
        } else if (key == "return") {
            iss >> current_location.return_path;
        } else if (key == "alias") {
            iss >> current_location.alias;
        }
    }
}

void ConfigParser::trim(std::string& s) {
    // Elimina espacios en blanco a los lados
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start != std::string::npos)
        s = s.substr(start);
    size_t end = s.find_last_not_of(" \t\n\r");
    if (end != std::string::npos)
        s = s.substr(0, end + 1);
}

ServerConfig ConfigParser::getServerConfig() const {
    return server;
}