#include "Server.hpp"
#include <cstring>
#include <netdb.h>
#include <arpa/inet.h>

Server::Server() {}

Server::~Server() {}

void Server::start(const ServerConfig& config) {
	const char spinner[] = {'/', '-', '\\', '|'};
	int j = 0;

    // Create the socket
    if ((this->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		std::cout << "Failed to create socket. errno: " << errno << std::endl;
    	exit(EXIT_FAILURE);
	}

    // Configuration to assign the port to the socket
    sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    int addrlen = sizeof(sockaddr);
    sockaddr.sin_family = AF_INET;
    
    // Configurar la dirección IP
    if (inet_pton(AF_INET, config.host.c_str(), &sockaddr.sin_addr) <= 0) {
        // Si la conversión a IP falla, intentar como un nombre de host
        struct hostent* he = gethostbyname(config.host.c_str());
        if (he == NULL) {
            std::cerr << "Invalid host: " << config.host << std::endl;
            exit(EXIT_FAILURE);
        }
        sockaddr.sin_addr = *(struct in_addr*)he->h_addr_list[0];
    }
	sockaddr.sin_port = htons(config.port);

    // Bind the port to the socket
	if (bind(this->sockfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0) {
		std::cout << "Failed to bind to port " << config.port << ". errno: " << errno << std::endl;
    	exit(EXIT_FAILURE);
	}

    // Set the socket in listening mode
	if (listen(this->sockfd, 3) < 0) {
		std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
    	exit(EXIT_FAILURE);
	}

    // Add the main socket to the poll file descriptors list
	pollfd main_socket_pollfd;
	main_socket_pollfd.fd = this->sockfd;
	main_socket_pollfd.events = POLLIN;
	this->poll_fds.push_back(main_socket_pollfd);


    while (1) {

		if (j++ >= 400000)
			j = 0;
		std::cout << "\rWating for activity [" << spinner[j / 100000] << "]" << std::flush;

		/*if (j++ >= 1000000)
			j = 0;
		if (j == 0 || j == 250000 || j == 500000 || j == 750000)
			std::cout << "\rWating for activity [" << spinner[j / 250000] << "]" << std::flush;*/

		int poll_count = poll(this->poll_fds.data(), this->poll_fds.size(), -1);
		if (poll_count < 0) {
			std::cout << "Poll failed. errno: " << errno << std::endl;
    		exit(EXIT_FAILURE);
		}

		for (size_t i = 0; i < this->poll_fds.size(); i++) {
			if (this->poll_fds[i].revents & POLLIN) { // Check if there is read activity
				if (this->poll_fds[i].fd == this->sockfd) { // New incoming connection
					int connection = accept(this->sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
					if (connection < 0) {
						std::cout << "Failed to grab connection. errno: " << errno << std::endl;
						exit(EXIT_FAILURE);
					}

					std::cout << "New connection accepted! Set identifier " << this->poll_fds.size() << std::endl;

					// Add the new client socket to poll
					pollfd new_client_pollfd;
					new_client_pollfd.fd = connection;
					new_client_pollfd.events = POLLIN;
					this->poll_fds.push_back(new_client_pollfd);
                    Client new_client;
                    this->clients.push_back(new_client);
				} else { // Read data from the client
					char buffer[1024] = {0};
					int bytesRead = read(this->poll_fds[i].fd, buffer, 1024);
					if (bytesRead <= 0) { // If there is no data or the connection is closed
						std::cout << "Client disconnected" << std::endl;
						close(this->poll_fds[i].fd);
						this->poll_fds.erase(this->poll_fds.begin() + i);
                        this->clients.erase(this->clients.begin() + (i - 1));
						i--;
					} else {
						std::string raw_request(buffer, bytesRead);
                        Request request(raw_request);
						clients[i - 1].setRequest(request);
                        std::cout << "Message received from client " << i << ": " << buffer << std::endl;

						method(clients[i - 1].getRequest(), this->poll_fds[i].fd);

						// Send a response to the client
                        //std::string response = "Hello! Welcome to webserv. This is a default response\n";
						//send(this->poll_fds[i].fd, response.c_str(), response.size(), 0);
                        std::cout << "Response sent!" << std::endl;
					}
				}
			}
		}
	}
}