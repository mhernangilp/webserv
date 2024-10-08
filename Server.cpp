#include "Server.hpp"
#include <cstring>
#include <netdb.h>
#include <arpa/inet.h>

Server::Server() {}

Server::~Server() {}

void Server::start(const ServerConfig& config) {
	const char spinner[] = {'/', '-', '\\', '|'};
	int j = 0;

	std::cout << LIGHT_BLUE << "[INFO] Initializing Server ..." << RESET << std::endl;

    // Create the socket
    if ((this->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		std::cerr << RED << "Error. Failed to create socket" << RESET << std::endl;
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
            std::cerr << RED << "Error. Invalid host: " << config.host << RESET << std::endl;
            exit(EXIT_FAILURE);
        }
        sockaddr.sin_addr = *(struct in_addr*)he->h_addr_list[0];
    }
	sockaddr.sin_port = htons(config.port);

    // Bind the port to the socket
	if (bind(this->sockfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0) {
		std::cerr << RED << "Error. Failed to bind to port " << config.port << RESET << std::endl;
    	exit(EXIT_FAILURE);
	}

    // Set the socket in listening mode
	if (listen(this->sockfd, 3) < 0) {
		std::cerr << RED << "Error. Failed to listen on socket" << RESET << std::endl;
    	exit(EXIT_FAILURE);
	}

    // Add the main socket to the poll file descriptors list
	pollfd main_socket_pollfd;
	main_socket_pollfd.fd = this->sockfd;
	main_socket_pollfd.events = POLLIN;
	this->poll_fds.push_back(main_socket_pollfd);

	std::cout << LIGHT_BLUE << "[INFO] Server Online: ServerName[" << config.server_name << "] Host[" << config.host << "] Port[" << config.port <<"]" << RESET << std::endl;

    while (1) {

		if (j++ >= 400000)
			j = 0;
		std::cout << PURPLE <<"\r[INFO] Wating For Activity [" << spinner[j / 100000] << "]" << std::flush << RESET;

		int poll_count = poll(this->poll_fds.data(), this->poll_fds.size(), -1);
		if (poll_count < 0) {
			std::cerr << "[ERR] Poll failed" << std::endl;
    		exit(EXIT_FAILURE);
		}

		for (size_t i = 0; i < this->poll_fds.size(); i++) {
			if (this->poll_fds[i].revents & POLLIN) { // Check if there is read activity
				if (this->poll_fds[i].fd == this->sockfd) { // New incoming connection
					int connection = accept(this->sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
					if (connection < 0) {
						std::cerr << "[ERR] Failed to grab connection" << std::endl;
						exit(EXIT_FAILURE);
					}

					std::cout << LIGHT_BLUE <<"\n[INFO] New Connection Accepted, Set Identifier " << this->poll_fds.size() << RESET << std::endl;

					// Add the new client socket to poll
					pollfd new_client_pollfd;
					new_client_pollfd.fd = connection;
					new_client_pollfd.events = POLLIN;
					this->poll_fds.push_back(new_client_pollfd);
                    Client new_client;
                    this->clients.push_back(new_client);
				} else { // Read data from the client
					bool clientConnected = processClientRequest(poll_fds[i].fd, i, poll_fds, clients);
					if (!clientConnected)
						i--;
                    std::cout << BLUE << "[INFO] Response sent!" << RESET << std::endl;
				}
			}
		}
	}
}

bool Server::processClientRequest(int client_fd, int client_index, std::vector<pollfd>& poll_fds, std::vector<Client>& clients) {
    std::string accumulated_request;
    char buffer[16384];
    int bytesRead;
    bool headersRead = false;
    size_t contentLength = 0;

    while ((bytesRead = read(client_fd, buffer, sizeof(buffer))) > 0) {
        accumulated_request.append(buffer, bytesRead);

        if (!headersRead) {
            size_t headerEnd = accumulated_request.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                headersRead = true;

                size_t contentLengthPos = accumulated_request.find("Content-Length:");
                if (contentLengthPos != std::string::npos) {
                    contentLength = std::stoul(accumulated_request.substr(contentLengthPos + 15));
                }

                size_t bodyStart = headerEnd + 4;
                if (accumulated_request.size() - bodyStart >= contentLength) {
                    break; 
                }
            }
        } else {
            if (accumulated_request.size() - (accumulated_request.find("\r\n\r\n") + 4) >= contentLength) {
                break;
            }
        }
    }

    if (bytesRead < 0) {
        std::cerr << "[ERROR] Error reading from client " << client_index << std::endl;
        return false;
    }

    if (bytesRead == 0) {
        std::cout << "[INFO] Client " << client_index << " Disconnected, Closing Connection ..." << std::endl;
        close(client_fd);
        poll_fds.erase(poll_fds.begin() + client_index);
        clients.erase(clients.begin() + (client_index - 1));
        return false;
    }

    Request request(accumulated_request);
    clients[client_index - 1].setRequest(request);

    std::cout << "[INFO] Message received from client " << client_index << std::endl;

    method(clients[client_index - 1].getRequest(), client_fd, config);

    return true;
}


void    Server::setConfig(ServerConfig& config)
{
	this->config = config;
}