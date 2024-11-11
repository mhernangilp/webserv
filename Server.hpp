#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string>
#include <iostream>
#include <poll.h>
#include <vector>
#include <cstdlib>
#include <cerrno>
#include "Client.hpp"
#include "config/ServerConfig.hpp"
#include <cstring>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <ctime>

#define RESET      "\033[0m"
#define RED        "\033[31m"
#define GREEN      "\033[32m"
#define YELLOW     "\033[33m"
#define ORANGE     "\033[38;5;214m"
#define PURPLE     "\033[35m"
#define BLUE       "\033[34m"
#define LIGHT_BLUE "\033[38;5;51m"

class Server {
    private:
        int sockfd;
        std::vector<pollfd> poll_fds;
        std::vector<Client> clients;
        const std::vector<ServerConfig>& config;

    public:
        Server(const std::vector<ServerConfig>& config);
        ~Server();

        void start(const ServerConfig& config);
        bool processClientRequest(int client_fd, const ServerConfig& config);
        void removeClient(int client_fd);
};

void method(Request request, int socket, const ServerConfig& serverConfig);

#endif