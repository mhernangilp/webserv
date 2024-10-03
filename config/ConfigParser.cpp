#include "ConfigParser.hpp"

int ConfigParser::parseConfig(const std::string& filename) {
    std::ifstream config_file(filename.c_str());
    std::string line;
    int brace_count = 0;
    bool finished = false;
    bool passed_server = false;

    if (!config_file.is_open()) {
        std::cout << ORANGE << "[WARN] Cannot open config file, using default parameters" << RESET << std::endl;
    }

    while (std::getline(config_file, line)) {
        line = line.substr(0, line.find("#"));
        trim(line);
        if (line.empty()) continue;
        if (!line.empty() && finished) {
            std::cerr << RED << "[ERR] characters out of server scope" << RESET << std::endl;
            return 1;
        }

        if (line[line.size() - 1] != ';' && line[line.size() - 1] != '{' && line[line.size() - 1] != '}') {
            std::cerr << RED << "[ERR] Missing semicolon in line: '" << line << "'" << RESET << std::endl;
            return 1;
        }

        for (size_t i = 0; i < line.size(); ++i) {
            if (line[i] == '{') {
                brace_count++;
            } else if (line[i] == '}') {
                brace_count--;
            }
        }

        int semicolon_count = 0;
        for (size_t i = 0; i < line.size(); ++i) {
            if (line[i] == ';') {
                semicolon_count++;
            }
        }
        if (semicolon_count > 1) {
            std::cerr << RED << "[ERR] Too many semicolons in line: '" << line << "'" << RESET << std::endl;
            return 1;
        }

        if (brace_count == 0)
            finished = true;

        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key != "server" && passed_server == false) {
            std::cerr << RED << "[ERR] invalid basic config file sintax" << RESET << std::endl;
            return 1;
        }
        if (key == "server") {
            current_block = "server";
            passed_server = true;
        } else if (key == "location") {
            std::string location_path;
            iss >> location_path;
            current_block = "location";
            current_location = LocationConfig();
            current_location.location = location_path;
        } else if (key == "}") {
            if (current_block == "location") {
                server.addLocation(current_location.location, current_location);
                current_block = "server";
            }
        } else {
            if (parseKey(key, iss) == 1) {
                std::cerr << "in line '" << line << "'" << RESET << std::endl;
                return 1;
            }
        }
    }

    if (brace_count != 0) {
        std::cerr << RED << "[ERR] Unmatched braces in configuration file." << RESET << std::endl;
        return 1;
    }

    if (access((server.root + server.index).c_str(), R_OK)) {
        std::cerr << RED << "[ERR] Invalid index." << RESET << std::endl;
        return 1;
    }

    return 0;
}

int ConfigParser::parseKey(const std::string& key, std::istringstream& iss) {
    std::string value;

    if (current_block == "server") {
        if (key == "listen") {
            iss >> server.port;
            if (server.port < 1 || server.port > 65535) {
                std::cerr << RED << "[ERR] port value not valid ";
                return 1;
            }
        } else if (key == "host") {
            std::string temp_host;
            iss >> temp_host;
            if (!temp_host.empty()) {
                server.host = temp_host.substr(0, temp_host.size() - 1);
            }
            if (!isValidIP(server.host)) {
                std::cerr << RED << "[ERR] Invalid IP address";
                return 1;
            }
        } else if (key == "server_name") {
            std::string temp_server_name;
            iss >> temp_server_name;
            if (!temp_server_name.empty()) {
                server.server_name = temp_server_name.substr(0, temp_server_name.size() - 1);
            }
        } else if (key == "error_page") {
            int error_type;
            std::string error_path;
            iss >> error_type;
            iss >> error_path;
            if (error_type < 400 || error_type > 599) {
                std::cerr << RED << "[ERR] error type not valid";
                return 1;
            }
            server.error_page[error_type] = error_path.substr(0, error_path.size() - 1);
        } else if (key == "client_max_body_size") {
            iss >> server.client_max_body_size;
        } else if (key == "root") {
            std::string temp_root;
            iss >> temp_root;
            if (!temp_root.empty()) {
                server.root = temp_root.substr(0, temp_root.size() - 1);
            }
        } else if (key == "index") {
            std::string temp_index;
            iss >> temp_index;
            if (!temp_index.empty()) {
                server.index = temp_index.substr(0, temp_index.size() - 1);
            }
        }
    } else if (current_block == "location") {
        if (key == "root") {
            iss >> current_location.root;
        } else if (key == "autoindex") {
            std::string temp_autoindex;
            iss >> temp_autoindex;
            if (!temp_autoindex.empty()) {
                current_location.autoindex = temp_autoindex.substr(0, temp_autoindex.size() - 1);
            }
            if (current_location.autoindex != "on" && current_location.autoindex != "off") {
                std::cerr << RED << "Error. autoindex value not valid ";
                return 1;
            }
        } else if (key == "allow_methods") {
            std::string method;
            while (iss >> method) {
                if (method[method.size() - 1] == ';')
                    method = method.substr(0, method.size() - 1);
                current_location.allow_methods.push_back(method);
            }
        } else if (key == "index") {
            std::string temp_index;
            iss >> temp_index;
            if (!temp_index.empty()) {
                current_location.index = temp_index.substr(0, temp_index.size() - 1);
            }
        } else if (key == "return") {
            std::string temp_return_path;
            iss >> temp_return_path;
            if (!temp_return_path.empty()) {
                current_location.return_path = temp_return_path.substr(0, temp_return_path.size() - 1);
            }
        }
    }
    return 0;
}

void ConfigParser::trim(std::string& s) {
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

bool ConfigParser::isValidIP(const std::string& ip) {
    std::istringstream iss(ip);
    std::string octet;
    int count = 0;

    while (std::getline(iss, octet, '.')) {
        int num;
        if (!(std::istringstream(octet) >> num) || num < 0 || num > 255) {
            return false;
        }
        count++;
    }
    return count == 4;
}
