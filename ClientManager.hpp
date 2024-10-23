#ifndef CLIENTMANAGER_HPP
#define CLIENTMANAGER_HPP

#include <set>
#include <vector>
#include <poll.h>
#include "Client.hpp"

class ClientManager {
    public:
        pollfd poll_fd;
        Client client;
        int id;
        
        ClientManager(pollfd& poll_fd, Client& client, int id);
        ClientManager(pollfd& poll_fd, int id);
        ~ClientManager();
};

#endif