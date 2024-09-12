#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <iostream>
#include <poll.h>
#include <vector>

int main(void)
{
	const int PORT = 8080;
	int sockfd, connection;
	sockaddr_in sockaddr;
	int addrlen = sizeof(sockaddr);
	std::string response;
	std::vector<pollfd> poll_fds;

	// Crear el socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		std::cout << "Failed to create socket. errno: " << errno << std::endl;
    	exit(EXIT_FAILURE);
	}

	// Configuración para asignar el puerto al socket
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(PORT);

	// Asignar el puerto al socket
	if (bind(sockfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0) {
		std::cout << "Failed to bind to port " << PORT << ". errno: " << errno << std::endl;
    	exit(EXIT_FAILURE);
	}

	// Poner el socket en modo de escucha
	if (listen(sockfd, 3) < 0) {
		std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
    	exit(EXIT_FAILURE);
	}

	// Añadir el socket principal a la lista de descriptores para poll()
	pollfd main_socket_pollfd;
	main_socket_pollfd.fd = sockfd;
	main_socket_pollfd.events = POLLIN;
	poll_fds.push_back(main_socket_pollfd);

	while (1) {
		std::cout << "\n+++++++ Waiting for activity ++++++++\n" << std::endl;

		int poll_count = poll(poll_fds.data(), poll_fds.size(), -1);
		if (poll_count < 0) {
			std::cout << "Poll failed. errno: " << errno << std::endl;
    		exit(EXIT_FAILURE);
		}

		for (size_t i = 0; i < poll_fds.size(); i++) {
			if (poll_fds[i].revents & POLLIN) { // Miramos si hay actividad de lectura
				if (poll_fds[i].fd == sockfd) { // Nueva conexión entrante
					if ((connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen)) < 0) {
						std::cout << "Failed to grab connection. errno: " << errno << std::endl;
						exit(EXIT_FAILURE);
					}

					std::cout << "New connection accepted! Set to client " << poll_fds.size() << std::endl;

					// Añadir el nuevo socket de cliente a poll()
					pollfd new_client_pollfd;
					new_client_pollfd.fd = connection;
					new_client_pollfd.events = POLLIN;
					poll_fds.push_back(new_client_pollfd);
				} else { // Leemos datos del cliente
					char buffer[1024] = {0};
					int bytesRead = read(poll_fds[i].fd, buffer, 1024);
					if (bytesRead <= 0) { // Si no hay datos o se cerró la conexión
						std::cout << "Client disconnected" << std::endl;
						close(poll_fds[i].fd);
						poll_fds.erase(poll_fds.begin() + i);
						i--;
					} else {
                        std::cout << "Message received from client " << i << ": " << buffer << std::endl;

						// Enviamos una respuesta al cliente
                        response = "HTTP/1.1 200 OK\nContent-Length: 27\n\nHello! Welcome to webserv\n";
						send(poll_fds[i].fd, response.c_str(), response.size(), 0);
                        std::cout << "Response sent!" << std::endl;
					}
				}
			}
		}
	}
	return (0);
}
