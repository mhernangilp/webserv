#include "LocationConfig.hpp"

LocationConfig::LocationConfig() : autoindex("off"), index("index.html") {}

void LocationConfig::print() const {
    std::cout << "Location Config:\n";
    std::cout << "\tRoot: " << root << "\n";
    std::cout << "\tAutoindex: " << autoindex << "\n";
    std::cout << "\tAllowed Methods: ";
    for (std::vector<std::string>::const_iterator it = allow_methods.begin(); it != allow_methods.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n\tIndex: " << index << "\n";
    std::cout << "\tReturn Path: " << return_path << "\n";
    std::cout << "\tAlias: " << alias << "\n";
}