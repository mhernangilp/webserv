#include "ServerConfig.hpp"
#include <cstring>
#include <sys/stat.h>

ServerConfig::ServerConfig() : port(8002), host("127.0.0.1"), server_name("localhost"), client_max_body_size(1024), index("index.html"), root("docs/kebab_web/") {
    LocationConfig location = LocationConfig();
    location.location = "/";
    location.autoindex = "off";
    location.allow_methods.push_back("GET");
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

void ServerConfig::struct_method_allowed() {

    char** method_allowed = new char*[locations.size() + 1];
    method_allowed[locations.size()] = NULL;

    method_location = new char*[locations.size() + 1];
    method_location[locations.size()] = NULL;

    int i = 0;
    for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        const std::vector<std::string>& allowed_methods = it->second.allow_methods;

        std::string methods_str;
        if (allowed_methods.size() == 0)
            methods_str += 'E';
        for (size_t j = 0; j < allowed_methods.size(); ++j) {
            if (allowed_methods[j] == "GET") {
                methods_str += 'G';
            } else if (allowed_methods[j] == "POST") {
                methods_str += 'P';
            } else if (allowed_methods[j] == "DELETE") {
                methods_str += 'D';
            }
        }

        method_allowed[i] = new char[methods_str.size() + 1];
        strcpy(method_allowed[i], methods_str.c_str());

        method_location[i] = new char[it->first.size() + 1]; // Asignar memoria para la ubicación
        strcpy(method_location[i], it->first.c_str()); // Copiar la ubicación
        ++i;
    }

    methods = method_allowed;
}

void ServerConfig::clearMethods() {
    if (methods) {
        for (int i = 0; methods[i] != NULL; ++i) {
            delete[] methods[i];
        }
        delete[] methods;
        methods = NULL; // Evitar accesos no válidos
    }

    if (method_location) {
        for (int i = 0; method_location[i] != NULL; ++i) {
            delete[] method_location[i];
        }
        delete[] method_location;
        method_location = NULL;
    }
}


int findCharFromEnd(const std::string& str, char ch) {
    for (int i = str.size() - 2; i >= 0; --i) {
        if (str[i] == ch) {
            return i;
        }
    }
    return -1;
}

bool checkDirectory(const std::string& path) {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) == 0) {
        return S_ISDIR(buffer.st_mode);
    }
    return false;
}

bool exists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

bool ServerConfig::isMethodAllowed(const std::string& location, char m) const{

    if (method_location == NULL || method_location[0] == NULL) {
        return false;
    }

    for (int i = 0; method_location[i] != NULL; ++i) {

        std::string dir_location = location;
        if (dir_location.size() == 0)
                dir_location = "/";

        while (dir_location.size() > 0){
            if (strcmp(method_location[i], dir_location.c_str()) == 0) {
                std::string met = methods[i];
                if (met.find(m) != std::string::npos)
                    return true;
                else if (met.find('E') == std::string::npos)
                    return false;
                else{
                    for (int i = 0; method_location[i] != NULL; ++i) {
                        if (strcmp(method_location[i], "/") == 0){
                            std::string met = methods[i];
                            if (met.find(m) != std::string::npos)
                                return true;
                            else
                                return false;
                        }
                    }
                }
            }
            int dir = findCharFromEnd(dir_location, '/');
            dir_location = location.substr(0, dir);
        }

    }
    std::string dir_location = location;
    while (dir_location.size() > 0){
        int dir = findCharFromEnd(dir_location, '/');
        dir_location = location.substr(0, dir);
        if (exists(root + dir_location) && checkDirectory(root + dir_location))
            return false;
    }
    return true;
}