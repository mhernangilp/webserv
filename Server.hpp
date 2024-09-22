#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <iostream>
#include <poll.h>
#include <vector>
#include <cstdlib>
#include <cerrno>
#include "Client.hpp"

class Server {
    private:
        const int PORT;
        int sockfd;
        std::vector<pollfd> poll_fds;
        std::vector<Client> clients;

    public:
        Server();
        Server(int PORT);
        ~Server();

        void    start();
};

void handleGetRequest(const std::string& url, int client_socket);

#endif