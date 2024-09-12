#include "Server.hpp"

Server::Server() : PORT(8080) {}

Server::Server(int PORT) : PORT(PORT) {}

Server::~Server() {}

void Server::start() {

    // Create the socket
    if ((this->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		std::cout << "Failed to create socket. errno: " << errno << std::endl;
    	exit(EXIT_FAILURE);
	}

    // Configuration to assign the port to the socket
    sockaddr_in sockaddr;
    int addrlen = sizeof(sockaddr);
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(this->PORT);

    // Bind the port to the socket
	if (bind(this->sockfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0) {
		std::cout << "Failed to bind to port " << this->PORT << ". errno: " << errno << std::endl;
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
		std::cout << "\n+++++++ Waiting for activity ++++++++\n" << std::endl;

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
				} else { // Read data from the client
					char buffer[1024] = {0};
					int bytesRead = read(this->poll_fds[i].fd, buffer, 1024);
					if (bytesRead <= 0) { // If there is no data or the connection is closed
						std::cout << "Client disconnected" << std::endl;
						close(this->poll_fds[i].fd);
						this->poll_fds.erase(this->poll_fds.begin() + i);
						i--;
					} else {
                        std::cout << "Message received from client " << i << ": " << buffer << std::endl;

						// Send a response to the client
                        std::string response = "Hello! Welcome to webserv. This is a default response\n";
						send(this->poll_fds[i].fd, response.c_str(), response.size(), 0);
                        std::cout << "Response sent!" << std::endl;
					}
				}
			}
		}
	}
}