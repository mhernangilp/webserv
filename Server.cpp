#include "Server.hpp"

#define POLL_TIMEOUT_MS 1000  // Timeout de 1 segundos

Server::Server(const std::vector<ServerConfig>& config) : config(config) {} 

Server::~Server() {}

void Server::start() {
    int sockfds[config.size()];
    sockaddr_in sockaddrs[config.size()];
    clients.resize(config.size());

	std::cout << LIGHT_BLUE << "[INFO] Initializing Server ..." << RESET << std::endl;

    for (size_t i = 0; i < config.size(); i++) {
        // Create the socket
        if ((sockfds[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << RED << "Error. Failed to create socket" << RESET << std::endl;
            exit(EXIT_FAILURE);
        }

        // Set socket options
        int opt = 1;
        if (setsockopt(sockfds[i], SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << RED << "Error. Failed to set socket options" << RESET << std::endl;
            exit(EXIT_FAILURE);
        }

        // Configuration to assign the port to the socket
        memset(&sockaddrs[i], 0, sizeof(sockaddrs[i]));
        sockaddrs[i].sin_family = AF_INET;
        
        // Configurar la dirección IP
        if (inet_pton(AF_INET, config[i].host.c_str(), &sockaddrs[i].sin_addr) <= 0) {
            // Si la conversión a IP falla, intentar como un nombre de host
            struct hostent* he = gethostbyname(config[i].host.c_str());
            if (he == NULL) {
                std::cerr << RED << "Error. Invalid host: " << config[i].host << RESET << std::endl;
                exit(EXIT_FAILURE);
            }
            sockaddrs[i].sin_addr = *(struct in_addr*)he->h_addr_list[0];
        }
        sockaddrs[i].sin_port = htons(config[i].port);

        // Bind the port to the socket
        if (bind(sockfds[i], (struct sockaddr *) &sockaddrs[i], sizeof(sockaddrs[i])) < 0) {
            std::cerr << RED << "Error. Failed to bind to port " << config[i].port << RESET << std::endl;
            exit(EXIT_FAILURE);
        }

        // Set the socket in listening mode
        if (listen(sockfds[i], 3) < 0) {
            std::cerr << RED << "Error. Failed to listen on socket" << RESET << std::endl;
            exit(EXIT_FAILURE);
        }

        // Add the main socket to the poll file descriptors list
        pollfd main_socket_pollfd;
        main_socket_pollfd.fd = sockfds[i];
        main_socket_pollfd.events = POLLIN;
        main_poll_fds.push_back(main_socket_pollfd);
    }
    int addrlen = sizeof(sockaddrs[0]);

    for (size_t i = 0; i < config.size(); i++) {
	    std::cout << LIGHT_BLUE << "[INFO] Server Online: ServerName[" << config[i].server_name << "] Host[" << config[i].host << "] Port[" << config[i].port <<"]" << RESET << std::endl;
    }

    while (1) {
		int main_poll_count = poll(main_poll_fds.data(), main_poll_fds.size(), POLL_TIMEOUT_MS);
		
        for (size_t i = 0; i < clients.size(); i++) {
            for (size_t j = 0; j < clients[i].size(); j++) {
                if (time(NULL) - clients[i][j].getLastReadTime() > 5) {
                    std::cout << "[INFO] Client " << clients[i][j].getIndex() - config.size() - 2 << " Time Out, Closing Connection ..." << std::endl;
                    close(clients[i][j].getIndex());
                    removeClient(clients[i][j].getIndex(), i);
                }
            }
        }

        if (main_poll_count < 0) {
			std::cerr << "[ERR] Poll failed" << std::endl;
    		exit(EXIT_FAILURE);
		} else if (main_poll_count == 0) {
			// Timeout: No hay actividad, pero el servidor continúa ejecutándose
			continue;
		}

        for (size_t i = 0; i < main_poll_fds.size(); i++) {
            if (main_poll_fds[i].revents & POLLIN) {
                if (i < config.size()) {
                    int connection = accept(sockfds[i], (struct sockaddr*)&sockaddrs[i], (socklen_t*)&addrlen);
                    if (connection < 0) {
                        std::cerr << "[ERR] Failed to grab connection" << std::endl;
                        exit(EXIT_FAILURE);
                    }

                    pollfd new_client_pollfd;
                    new_client_pollfd.fd = connection;
                    new_client_pollfd.events = POLLIN;
                    main_poll_fds.push_back(new_client_pollfd);
                    Client new_client;
                    new_client.setIndex(new_client_pollfd.fd);
                    new_client.setLastReadTime(time(NULL));
                    clients[i].push_back(new_client);
                    
                    std::cout << LIGHT_BLUE <<"[INFO] New Connection Accepted [" << config[i].host << ":" << config[i].port << "], Set Identifier " << new_client_pollfd.fd - config.size() - 2 << RESET << std::endl;
                } else {
                    int server_number = locateClientServer(main_poll_fds[i].fd);
                    if (server_number == -1) {
                        std::cerr << RED << "[ERR] internal error when locating server_number" << RESET << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    processClientRequest(main_poll_fds[i].fd, server_number);
                }
            }
        }
        for (size_t i = 0; i < main_poll_fds.size(); i++) {
            if ((main_poll_fds[i].revents & POLLOUT) && pending_responses.count(main_poll_fds[i].fd)) {
                int client_fd = main_poll_fds[i].fd;
                int server_number = locateClientServer(client_fd);

                if (server_number != -1) {
                    Request& client_request = pending_responses[client_fd];
                    method(client_request, client_fd, config[server_number], client_fd - config.size() - 2);

                    // Cerrar y limpiar después de enviar
                    std::cout << "[INFO] Client " << client_fd - config.size() - 2 << " Disconnected, Closing Connection ..." << std::endl;
                    close(client_fd);
                    removeClient(client_fd, server_number);
                    pending_responses.erase(client_fd); // Limpiar respuesta pendiente
                }
            }
        }
	}
}

void Server::processClientRequest(int client_fd, int server_number) {
    std::string accumulated_request;
    char buffer[16384];
    int bytesRead;
    bool headersRead = false;
    size_t contentLength = 0;

    while (true) {
        bytesRead = read(client_fd, buffer, sizeof(buffer));

        if (bytesRead > 0) {
            size_t index = client_fd - config.size() - 3;

            // Verificar que el índice esté dentro de los límites del contenedor `clients[server_number]`
            if (index < clients[server_number].size()) {
                clients[server_number][index].setLastReadTime(time(NULL)); // Reiniciar temporizador en caso de actividad
                accumulated_request.append(buffer, bytesRead);

                // Verificar si el tamaño acumulado supera el límite permitido
                if (accumulated_request.size() > config[server_number].client_max_body_size) {
                    std::cerr << "[INFO] Client " << client_fd - config.size() - 2 << " exceeded maximum body size "  << accumulated_request.size() << ". Closing connection ..." <<  std::endl;
                    max_body(client_fd, server_number);
                    close(client_fd);
                    removeClient(client_fd, server_number);
                    return;
                }
            } else {
                close(client_fd);
                removeClient(client_fd, server_number);
                return;
            }
        } else if (bytesRead <= 0) {
            // Cliente desconectado o error
            std::cout << "[INFO] Client " << client_fd - config.size() - 2 << " Disconnected, Closing Connection ..." << std::endl;
            close(client_fd);
            removeClient(client_fd, server_number);
            return;
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
    request.setServer(server_number);
    clients[server_number][client_fd - config.size() - 3].setRequest(request);

    std::cout << BLUE << "[INFO] Message received from client " << client_fd - config.size() - 2 << ", Method = <" << request.getMethod() << ">  URL = <" << request.getUrl() << ">" << RESET << std::endl;

    pending_responses[client_fd] = clients[server_number][client_fd - config.size() - 3].getRequest();

    for (size_t i = 0; i < main_poll_fds.size(); i++) {
        if (main_poll_fds[i].fd == client_fd) {
            main_poll_fds[i].events = POLLOUT;
            break;
        }
    }
    return ;
}


void Server::removeClient(int client_fd, int server_number) {
    int changed = 0;

	for (size_t i = 0; i < main_poll_fds.size(); i++) {
        if (main_poll_fds[i].fd == client_fd) {
            main_poll_fds.erase(main_poll_fds.begin() + i);
            changed++;
            break;
        }
    }
    for (size_t i = 0; i < clients[server_number].size(); i++) {
        if (clients[server_number][i].getIndex() == client_fd) {
            clients[server_number].erase(clients[server_number].begin() + i);
            changed++;
            break;
        }
    }
    if (changed != 2) {
        std::cerr << "[ERR] Error on client deletion" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Server::max_body(int client_fd, int server_number){
    body_limit(client_fd, config[server_number], client_fd - config.size() - 2);
    std::cout << "[INFO] Client " << client_fd - config.size() - 2 << " Disconnected, Closing Connection ..." << std::endl;
}

int Server::locateClientServer(int client_fd) {
    for (size_t server_number = 0; server_number < clients.size(); ++server_number) {
        for (size_t client_index = 0; client_index < clients[server_number].size(); ++client_index) {
            if (clients[server_number][client_index].getIndex() == client_fd) {
                return server_number; // Retorna el servidor al que pertenece el cliente
            }
        }
    }
    return -1; // No se encontró el cliente
}