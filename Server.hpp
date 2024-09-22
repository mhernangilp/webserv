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
        int PORT;
        int sockfd;
        std::vector<pollfd> poll_fds;
        std::vector<Client> clients;

    public:
        Server();
        ~Server();

        //void    setup();
        void    start();
};

#endif