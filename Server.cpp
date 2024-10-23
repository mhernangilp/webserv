#include "Server.hpp"

Server::Server() {}

Server::~Server() {}

void Server::start(const ServerConfig& config) {
    nextId = 0;
	std::cout << LIGHT_BLUE << "[INFO] Initializing Server ..." << RESET << std::endl;

    // Crear el socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		std::cerr << RED << "Error. Failed to create socket" << RESET << std::endl;
    	exit(EXIT_FAILURE);
	}

    // Configuración para asignar el puerto al socket
    sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    int addrlen = sizeof(sockaddr);
    sockaddr.sin_family = AF_INET;

    // Configurar la dirección IP
    if (inet_pton(AF_INET, config.host.c_str(), &sockaddr.sin_addr) <= 0) {
        // Si falla la conversión a IP, intentar como nombre de host
        struct hostent* he = gethostbyname(config.host.c_str());
        if (he == NULL) {
            std::cerr << RED << "Error. Invalid host: " << config.host << RESET << std::endl;
            exit(EXIT_FAILURE);
        }
        sockaddr.sin_addr = *(struct in_addr*)he->h_addr_list[0];
    }
	sockaddr.sin_port = htons(config.port);

    // Vincular el puerto al socket
	if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
		std::cerr << RED << "Error. Failed to bind to port " << config.port << RESET << std::endl;
    	exit(EXIT_FAILURE);
	}

    // Poner el socket en modo escucha
	if (listen(sockfd, 3) < 0) {
		std::cerr << RED << "Error. Failed to listen on socket" << RESET << std::endl;
    	exit(EXIT_FAILURE);
	}

    // Añadir el socket principal a la lista de descriptores de archivos
	pollfd main_socket_pollfd;
	main_socket_pollfd.fd = sockfd;
	main_socket_pollfd.events = POLLIN;
    poll_fds[nextId++] = main_socket_pollfd;

	std::cout << LIGHT_BLUE << "[INFO] Server Online: ServerName[" << config.server_name << "] Host[" << config.host << "] Port[" << config.port <<"]" << RESET << std::endl;

    while (1) {
        // Convertir el mapa poll_fds en un vector de pollfd para usar en poll()
        std::vector<pollfd> poll_fd_vector;
        for (std::map<int, pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end(); ++it) {
            poll_fd_vector.push_back(it->second);
        }

		int poll_count = poll(&poll_fd_vector[0], poll_fd_vector.size(), -1);
		if (poll_count < 0) {
			std::cerr << "[ERR] Poll failed" << std::endl;
    		exit(EXIT_FAILURE);
		}

		std::map<int, pollfd>::iterator it;
        for (it = poll_fds.begin(); it != poll_fds.end(); ++it) {
            std::cout << "checkeo " << it->first << std::endl;
			if (it->second.revents & POLLIN) { // Verificar actividad de lectura
                std::cout << "hay lectura" << std::endl;
				if (it->second.fd == sockfd) { // Nueva conexión entrante
                    std::cout << "nueva conexion" << std::endl;
					int id;
                    int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
					if (connection < 0) {
						std::cerr << "[ERR] Failed to grab connection" << std::endl;
						continue;
					}

					// Añadir el nuevo socket cliente a poll
                    if (!freeIDs.empty()) {
                        id = *freeIDs.begin();
                        freeIDs.erase(freeIDs.begin()); // Arreglo del doble punto
                    } else {
                        id = nextId++;
                    }
                    std::cout << LIGHT_BLUE << "[INFO] New Connection Accepted, Set Identifier " << id << RESET << std::endl;
					pollfd new_client_pollfd;
					new_client_pollfd.fd = connection;
					new_client_pollfd.events = POLLIN;
                    poll_fds[id] = new_client_pollfd;
                    Client new_client;
                    clients[id] = new_client;

				} else { // Leer datos del cliente
                    std::cout << "conexion cliente" << std::endl;
					bool clientConnected = processClientRequest(it->second.fd, it->first, poll_fds, clients);
					if (!clientConnected)
						it--; // Decrementar el iterador si el cliente se desconecta
				}
			}
		}
	}
}

bool Server::processClientRequest(int client_fd, int client_index, std::map<int, pollfd>& poll_fds, std::map<int, Client>& clients) {
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
            poll_fds.erase(client_index);
            clients.erase(client_index);
            freeIDs.insert(client_index);
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
    clients[client_index].setRequest(request);

    std::cout << BLUE << "[INFO] Message received from client " << client_index  << ", Method = <"<< request.getMethod() << ">  URL = <" << request.getUrl() << ">" << RESET << std::endl;

    method(clients[client_index].getRequest(), client_fd, config);

    return true;
}

void    Server::setConfig(ServerConfig& config)
{
	this->config = config;
}