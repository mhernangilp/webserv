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

class Server {
    private:
        const int PORT;
        int sockfd;
        std::vector<pollfd> poll_fds;

    public:
        Server();
        Server(int PORT);
        Server(const Server& original);
        Server& operator=(const Server& original);
        ~Server();

        void    start();
};

#endif