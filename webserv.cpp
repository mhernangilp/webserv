#include "Server.hpp"
#include "config/ConfigParser.hpp"

int main(int argc, char **argv)
{
	Server server;
	ConfigParser parser;

	if (argc != 2) {
		std::cerr << RED << "Error: number of arguments not valid\n" << RESET << "Usage: ./webserv <config file>" << std::endl;
		return 1;
	}
	if (parser.parseConfig(argv[1]) == 1)
		return 2; 
	ServerConfig config = parser.getServerConfig();
	config.print();
	server.start(config);
	return (0);
}
