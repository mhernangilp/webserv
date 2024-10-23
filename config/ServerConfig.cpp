#include "ServerConfig.hpp"
#include <cstring>


ServerConfig::ServerConfig() : port(8001), host("127.0.0.1"), server_name("localhost"), client_max_body_size(1024), index("index.html"), root("docs/kebab_web/") {
    error_page[404] = "custom_error/404.html";
    
    LocationConfig location = LocationConfig();
    location.location = "/";
    location.autoindex = "off";
    location.allow_methods.push_back("DELETE");
    location.allow_methods.push_back("POST");
    location.allow_methods.push_back("GET");
    this->addLocation(location.location, location);
    
    location = LocationConfig();
    location.location = "/kebabs";
    location.autoindex = "on";
    location.allow_methods.push_back("GET");
    location.allow_methods.push_back("POST");
    location.index = "kebab.html";
    this->addLocation(location.location, location);

    location = LocationConfig();
    location.location = "/secret-sauce";
    location.return_path = "/kebabs";
    this->addLocation(location.location, location);

    location = LocationConfig();
    location.location = "/cgi-bin";
    location.root = "./";
    location.allow_methods.push_back("GET");
    location.allow_methods.push_back("POST");
    location.allow_methods.push_back("DELETE");
    location.index = "time.py";
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
    // Crear un arreglo para las rutas
    char** method_allowed = new char*[locations.size() + 1];
    method_allowed[locations.size()] = NULL; // Terminador

    // Inicializar method_location
    method_location = new char*[locations.size() + 1];
    method_location[locations.size()] = NULL; // Terminador

    int i = 0;
    for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        const std::vector<std::string>& allowed_methods = it->second.allow_methods;

        // Crear una cadena para almacenar los caracteres 'G', 'P', 'D'
        std::string methods_str;

        // Verificar qué métodos están permitidos y construir la cadena
        for (size_t j = 0; j < allowed_methods.size(); ++j) {
            if (allowed_methods[j] == "GET") {
                methods_str += 'G';
            } else if (allowed_methods[j] == "POST") {
                methods_str += 'P';
            } else if (allowed_methods[j] == "DELETE") {
                methods_str += 'D';
            }
        }

        // Crear espacio para la cadena en method_allowed
        method_allowed[i] = new char[methods_str.size() + 1];
        strcpy(method_allowed[i], methods_str.c_str());

        // Asignar la ubicación
        method_location[i] = new char[it->first.size() + 1]; // Asignar memoria para la ubicación
        strcpy(method_location[i], it->first.c_str()); // Copiar la ubicación

        std::cout << "Methods Allowed: " << method_allowed[i] << " on location " << method_location[i] << std::endl;

        ++i;
    }

    methods = method_allowed; // Guardar el arreglo de métodos permitidos
}





bool ServerConfig::isMethodAllowed(const std::string& location, char m) const{

    if (method_location == NULL || method_location[0] == NULL) {
        return false;
    }

   for (int i = 0; method_location[i] != NULL; ++i) {
        if (strcmp(method_location[i], "/") == 0){
            std::string met = methods[i];
            if (met.find(m) != std::string::npos) {
                return true;
            }
        }
    }
    for (int i = 0; method_location[i] != NULL; ++i) {
        if (strcmp(method_location[i], location.c_str()) == 0) {
            std::string met = methods[i];
            std::cout << "Methods: " << met << std::endl;
            if (met.find(m) != std::string::npos) {
                return true;
            }
            else 
                return false;
        }
    }
    return false;
}