#ifndef LOCATION_CONFIG_HPP
#define LOCATION_CONFIG_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

class LocationConfig {
public:
    std::string location;
    std::string root;
    std::string autoindex;
    std::vector<std::string> allow_methods;
    std::string index;
    std::string return_path;

    LocationConfig();

    void print() const;
};

#endif