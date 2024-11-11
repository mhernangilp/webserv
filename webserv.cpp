#include "Server.hpp"
#include "config/ConfigParser.hpp"

int main(int argc, char **argv)
{
	ConfigParser parser;
	const char *config_file;

	if (argc > 2) {
		std::cerr << RED << "Error: number of arguments not valid\n" << RESET << "Usage: ./webserv [config file]" << std::endl;
		return 1;
	}
	if (argc == 1) {
		config_file = "config/default.conf";
		std::cout << LIGHT_BLUE << "[INFO] Using Default Config File" << RESET << std::endl;
	} else {
		config_file = argv[1];
	}
	if (parser.parseConfig(config_file) == 1)
		return 2; 
	std::vector<ServerConfig>& config = parser.getServerConfig();
	for (size_t i = 0; i < config.size(); i++) {
		std::cout << LIGHT_BLUE << "SERVER " << i << ":" << RESET << std::endl;
		config[i].print();
	}
	Server server(config);
	for (size_t i = 0; i < config.size(); i++) {
		config[i].struct_method_allowed();
	}
	//server.start(config);
	return (0);
}
