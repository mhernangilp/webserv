#ifndef CLIENTMANAGER_HPP
#define CLIENTMANAGER_HPP

#include <set>
#include <vector>
#include <poll.h>
#include "Client.hpp"

class ClientManager {
    public:
        int id;
        std::vector<pollfd> poll_fds;
        std::vector<Client> clients;
        
        ClientManager();
        ~ClientManager();
};

#endif