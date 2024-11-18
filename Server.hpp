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
#include "methods/Request.hpp"
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
        std::vector<pollfd> main_poll_fds;
        std::vector<std::vector<Client> > clients;
        const std::vector<ServerConfig>& config;
        std::map<int, Request> pending_responses;

    public:
        Server(const std::vector<ServerConfig>& config);
        ~Server();

        void start();
        void processClientRequest(int client_fd, int server_number);
        void removeClient(int client_fd, int server_number);
        void max_body(int client_fd, int server_number);
};

void method(Request request, int socket, const ServerConfig& serverConfig);
void body_limit(int client_socket, const ServerConfig& serverConfig);

#endif