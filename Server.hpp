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
        ServerConfig config;

    public:
        Server();
        ~Server();

        void    start(const ServerConfig& config);
        void    setConfig(ServerConfig& config);
        bool processClientRequest(int client_fd, int client_index, std::vector<pollfd>& poll_fds, std::vector<Client>& clients, const ServerConfig& config);
};

void method(Request request, int socket, int index, const ServerConfig& serverConfig);

#endif