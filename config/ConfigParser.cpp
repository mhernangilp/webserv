#include "ConfigParser.hpp"
#include "../Utils.hpp"

int ConfigParser::parseConfig(const std::string& filename) {
    std::ifstream config_file(filename.c_str());
    std::string line;
    int brace_count = 0;
    bool finished = false;
    bool passed_server = false;
    found_listen = false;
    found_host = false;
    found_index = false;
    found_max_body_size = false;
    found_root = false;
    found_server_name = false;

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
            if (location_path != "/")
                current_location.location = normalizeUrl(location_path);
            else
                current_location.location = location_path;
            if (current_location.location != "/" && server.locations.find(current_location.location) != server.locations.end()) {
                std::cerr << RED << "[ERR] found duplicate location" << RESET << std::endl;
                return 1;
            }
        } else if (key == "}") {
            if (current_block == "location") {
                server.addLocation(current_location.location, current_location);
                std::cout << RESET;
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
        std::cerr << RED << "[ERR] Invalid index." << server.root << server.index << RESET << std::endl;
        return 1;
    }

    return 0;
}

int ConfigParser::parseKey(const std::string& key, std::istringstream& iss) {
    std::string value;

    if (current_block == "server") {
        if (key == "listen") {
            if (found_listen) {
                std::cerr << RED << "[ERR] port already set ";
                return 1;
            }
            found_listen = true;
            iss >> server.port;
            if (server.port < 1 || server.port > 65535) {
                std::cerr << RED << "[ERR] port value not valid ";
                return 1;
            }
        } else if (key == "max_body_size") {
            if (found_max_body_size) {
                std::cerr << RED << "[ERR] client maz body size already set ";
                return 1;
            }
            found_max_body_size = true;
            iss >> server.client_max_body_size;
            if (server.client_max_body_size < 1024) {
                std::cerr << RED << "[ERR] min value is 1024 bytes ";
                return 1;
            }
        } else if (key == "host") {
            if (found_host) {
                std::cerr << RED << "[ERR] host already set ";
                return 1;
            }
            found_host = true;
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
            if (found_server_name) {
                std::cerr << RED << "[ERR] server name already set ";
                return 1;
            }
            found_server_name = true;
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
                std::cerr << RED << "[ERR] error type not valid ";
                return 1;
            }
            if (server.error_page.find(error_type) != server.error_page.end()) {
                std::cerr << RED << "[ERR] duplicate custom error page ";
                return 1;
            }
            server.error_page[error_type] = normalizeUrl(error_path.substr(0, error_path.size() - 1));
        } else if (key == "root") {
            if (found_root) {
                std::cerr << RED << "[ERR] root already set ";
                return 1;
            }
            found_root = true;
            std::string temp_root;
            iss >> temp_root;
            if (!temp_root.empty()) {
                server.root = normalizeUrl(temp_root.substr(0, temp_root.size() - 1)) + "/";
            }
        } else if (key == "index") {
            if (found_index) {
                std::cerr << RED << "[ERR] index already set ";
                return 1;
            }
            found_index = true;
            std::string temp_index;
            iss >> temp_index;
            if (!temp_index.empty()) {
                server.index = normalizeUrl(temp_index.substr(0, temp_index.size() - 1));
            }
        }
    } else if (current_block == "location") {
        if (key == "root") {
            std::string temp_root;
            iss >> temp_root;
            if (!temp_root.empty()) {
                current_location.root = normalizeUrl(temp_root.substr(0, temp_root.size() - 1));
            }
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
                current_location.index = normalizeUrl(temp_index.substr(0, temp_index.size() - 1));
            }
            if (!current_location.index.empty() && current_location.index[0] == '/') {
                std::cerr << RED << "Error. location index can't be absolute ";
                return 1;
            } 
        } else if (key == "return") {
            std::string temp_return_path;
            iss >> temp_return_path;
            if (!temp_return_path.empty()) {
                current_location.return_path = normalizeUrl(temp_return_path.substr(0, temp_return_path.size() - 1));
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
