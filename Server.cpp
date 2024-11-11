#include "Server.hpp"

#define POLL_TIMEOUT_MS 1000  // Timeout de 1 segundos

Server::Server(const std::vector<ServerConfig>& config) : config(config) {} 

Server::~Server() {}

void Server::start(const ServerConfig& config) {
	std::cout << LIGHT_BLUE << "[INFO] Initializing Server ..." << RESET << std::endl;

    // Create the socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		std::cerr << RED << "Error. Failed to create socket" << RESET << std::endl;
    	exit(EXIT_FAILURE);
	}

    // Set socket options
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << RED << "Error. Failed to set socket options" << RESET << std::endl;
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
	if (bind(sockfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0) {
		std::cerr << RED << "Error. Failed to bind to port " << config.port << RESET << std::endl;
    	exit(EXIT_FAILURE);
	}

    // Set the socket in listening mode
	if (listen(sockfd, 3) < 0) {
		std::cerr << RED << "Error. Failed to listen on socket" << RESET << std::endl;
    	exit(EXIT_FAILURE);
	}

    // Add the main socket to the poll file descriptors list
	pollfd main_socket_pollfd;
	main_socket_pollfd.fd = sockfd;
	main_socket_pollfd.events = POLLIN;
	poll_fds.push_back(main_socket_pollfd);

	std::cout << LIGHT_BLUE << "[INFO] Server Online: ServerName[" << config.server_name << "] Host[" << config.host << "] Port[" << config.port <<"]" << RESET << std::endl;
    while (1) {
		int poll_count = poll(poll_fds.data(), poll_fds.size(), POLL_TIMEOUT_MS);
		
        for (size_t i = 0; i < clients.size(); i++) {
            if (time(NULL) - clients[i].getLastReadTime() > 5) {
                std::cout << "[INFO] Client " << clients[i].getIndex() - 3 << " Time Out, Closing Connection ..." << std::endl;
                close(clients[i].getIndex());
                removeClient(clients[i].getIndex());
            }
        }

        if (poll_count < 0) {
			std::cerr << "[ERR] Poll failed" << std::endl;
    		exit(EXIT_FAILURE);
		} else if (poll_count == 0) {
			// Timeout: No hay actividad, pero el servidor continúa ejecutándose
			continue;
		}

		for (size_t i = 0; i < poll_fds.size(); i++) {
			if (poll_fds[i].revents & POLLIN) { // Check if there is read activity
				if (poll_fds[i].fd == sockfd) { // New incoming connection
					int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
					if (connection < 0) {
						std::cerr << "[ERR] Failed to grab connection" << std::endl;
						exit(EXIT_FAILURE);
					}

					// Add the new client socket to poll
					pollfd new_client_pollfd;
					new_client_pollfd.fd = connection;
					new_client_pollfd.events = POLLIN;
					poll_fds.push_back(new_client_pollfd);
                    Client new_client;
                    new_client.setIndex(new_client_pollfd.fd);
                    new_client.setLastReadTime(time(NULL));
                    clients.push_back(new_client);
                    std::cout << LIGHT_BLUE <<"[INFO] New Connection Accepted, Set Identifier " << new_client_pollfd.fd - 3 << RESET << std::endl;

				} else { // Read data from the client
					bool clientConnected = processClientRequest(poll_fds[i].fd, config);
					if (!clientConnected)
						continue;
				}
			}
		}
	}
}

bool Server::processClientRequest(int client_fd, const ServerConfig& configServer) {
    std::string accumulated_request;
    char buffer[16384];
    int bytesRead;
    bool headersRead = false;
    size_t contentLength = 0;

    while (true) {
        bytesRead = read(client_fd, buffer, sizeof(buffer));

        if (bytesRead > 0) {
            clients[client_fd - 4].setLastReadTime(time(NULL));  // Reiniciar temporizador en caso de actividad
            accumulated_request.append(buffer, bytesRead);

            // Verificar si el tamaño acumulado supera el límite permitido
            if (accumulated_request.size() > configServer.client_max_body_size) {
                std::cerr << "[INFO] Client " << client_fd - 3 << " exceeded maximum body size. Closing connection ..." << std::endl;
                close(client_fd);
                removeClient(client_fd);
                return false;
            }
        } else if (bytesRead <= 0) {
            // Cliente desconectado o error
            std::cout << "[INFO] Client " << client_fd - 3 << " Disconnected, Closing Connection ..." << std::endl;
            close(client_fd);
            removeClient(client_fd);
            return false;
        }

        if (!headersRead) {
            size_t headerEnd = accumulated_request.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                headersRead = true;

                // Verificar la posición de Content-Length
                size_t contentLengthPos = accumulated_request.find("Content-Length:");
                if (contentLengthPos != std::string::npos) {
                    contentLength = std::strtoul(accumulated_request.substr(contentLengthPos + 15).c_str(), NULL, 10);
                }

                size_t bodyStart = headerEnd + 4; // 4 bytes para saltar "\r\n\r\n"
                // Comprueba si hemos leído el cuerpo completo
                if (accumulated_request.size() - bodyStart >= contentLength) {
                    break; 
                }
            }
        } else { // Si ya hemos leído los encabezados
            size_t bodyStart = accumulated_request.find("\r\n\r\n") + 4;
            if (accumulated_request.size() - bodyStart >= contentLength) {
                break; // Si ya tenemos el cuerpo completo
            }
        }
    }

    Request request(accumulated_request);
    clients[client_fd - 4].setRequest(request);

    std::cout << BLUE << "[INFO] Message received from client " << client_fd - 3 << ", Method = <"<< request.getMethod() << ">  URL = <" << request.getUrl() << ">" << RESET << std::endl;

    method(clients[client_fd - 4].getRequest(), client_fd, configServer);

    std::cout << "[INFO] Client " << client_fd - 3 << " Disconnected, Closing Connection ..." << std::endl;
    close(client_fd);
    removeClient(client_fd);
    return false;
}

void Server::removeClient(int client_fd) {
    int changed = 0;

	for (size_t i = 0; i < poll_fds.size(); i++) {
        if (poll_fds[i].fd == client_fd) {
            poll_fds.erase(poll_fds.begin() + i);
            changed++;
            break;
        }
    }
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getIndex() == client_fd) {
            clients.erase(clients.begin() + i);
            changed++;
            break;
        }
    }
    if (changed != 2) {
        std::cerr << "[ERR] Error on client deletion" << std::endl;
        exit(EXIT_FAILURE);
    }
}