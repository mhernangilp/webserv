#include "Server.hpp"

Server::Server() {}

Server::~Server() {}

void Server::start(const ServerConfig& config) {
    nextId = 0;
	std::cout << LIGHT_BLUE << "[INFO] Initializing Server ..." << RESET << std::endl;

    // Create the socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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
    ClientManager* clientManager = new ClientManager(main_socket_pollfd, nextId++);
	clientManagers.push_back(clientManager);

	std::cout << LIGHT_BLUE << "[INFO] Server Online: ServerName[" << config.server_name << "] Host[" << config.host << "] Port[" << config.port <<"]" << RESET << std::endl;

    std::vector<pollfd> new_poll_fds;

    for (std::vector<ClientManager*>::iterator it = clientManagers.begin(); it != clientManagers.end(); ++it) {
        new_poll_fds.push_back((*it)->poll_fd);  // Extraer el pollfd de cada ClientManager*
    }

    while (1) {
		int poll_count = poll(new_poll_fds.data(), new_poll_fds.size(), -1);
		if (poll_count < 0) {
			std::cerr << "[ERR] Poll failed" << std::endl;
    		exit(EXIT_FAILURE);
		}

		for (size_t i = 0; i < clientManagers.size(); i++) {
			if (clientManagers[i]->poll_fd.revents & POLLIN) { // Check if there is read activity
				if (clientManagers[i]->poll_fd.fd == sockfd) { // New incoming connection
					int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
					if (connection < 0) {
						std::cerr << "[ERR] Failed to grab connection" << std::endl;
						exit(EXIT_FAILURE);
					}

					// Add the new client socket to poll
                    std::cout << LIGHT_BLUE <<"[INFO] New Connection Accepted, Set Identifier " << clientManagers.size() << RESET << std::endl;
					pollfd new_client_pollfd;
					new_client_pollfd.fd = connection;
					new_client_pollfd.events = POLLIN;
                    Client new_client;
                    clientManager = new ClientManager(new_client_pollfd, new_client, nextId++);
	                clientManagers.push_back(clientManager);

				} else { // Read data from the client
					bool clientConnected = processClientRequest(clientManagers[i]->poll_fd.fd, i);
					if (!clientConnected)
						i--;
				}
			}
		}
	}
}

bool Server::processClientRequest(int client_fd, int client_index) {
    std::string accumulated_request;
    char buffer[16384];
    int bytesRead;
    bool headersRead = false;
    size_t contentLength = 0;

    while (true) {
        bytesRead = read(client_fd, buffer, 16384);

        if (bytesRead <= 0) {
            std::cout << "[INFO] Client " << client_index << " Disconnected, Closing Connection ..." << std::endl;
            close(client_fd);
            clientManagers.erase(clientManagers.begin() + client_index);
            return false;
        }

        accumulated_request.append(buffer, bytesRead);

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
    clientManagers[client_index]->client.setRequest(request);

    std::cout << BLUE << "[INFO] Message received from client " << client_index  << ", Method = <"<< request.getMethod() << ">  URL = <" << request.getUrl() << ">" << RESET << std::endl;

    method(clientManagers[client_index]->client.getRequest(), client_fd, config);

    return true;
}

void    Server::setConfig(ServerConfig& config)
{
	this->config = config;
}