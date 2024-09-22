#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

class LocationConfig {
public:
    std::string root;
    std::string autoindex;
    std::vector<std::string> allow_methods;
    std::string index;
    std::string return_path;
    std::string alias;

    LocationConfig();

    void print() const;
};
