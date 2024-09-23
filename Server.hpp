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
#include "config/ServerConfig.hpp"

class Server {
    private:
        int sockfd;
        std::vector<pollfd> poll_fds;
        std::vector<Client> clients;

    public:
        Server();
        ~Server();

        void    start(const ServerConfig& config);
};

void handleGetRequest(const std::string& url, int client_socket);

#endif