#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <iostream>

int main(void)
{
	const int PORT = 8080;
	int sockfd, connection, bytesRead;
	sockaddr_in sockaddr;
	int addrlen = sizeof(sockaddr);
	std::string response;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		std::cout << "Failed to create socket. errno: " << errno << std::endl;
    	exit(EXIT_FAILURE);
	}

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(PORT);

	if (bind(sockfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0) {
		std::cout << "Failed to bind to port " << PORT << ". errno: " << errno << std::endl;
    	exit(EXIT_FAILURE);
	}

	if (listen(sockfd, 3) < 0) {
		std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
    	exit(EXIT_FAILURE);
	}

	while (1) {
		std::cout << "\n+++++++ Waiting for new connection ++++++++\n" << std::endl;

		if ((connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen)) < 0) {
			std::cout << "Failed to grab connection. errno: " << errno << std::endl;
    		exit(EXIT_FAILURE);
		}

		char buffer[1024] = {0};
		//char buffer[1024];
		bytesRead = read(connection, buffer, 1024);
		std::cout << "The message was: " << buffer << std::endl;

		response = "Hello! Welcome to webserv\n";
		send(connection, response.c_str(), response.size(), 0);
		std::cout << "------------Welcome message sent-------------" << std::endl;

		close(connection);
	}
	return (0);
}
