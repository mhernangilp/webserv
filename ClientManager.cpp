#include "ClientManager.hpp"

ClientManager::ClientManager(pollfd& poll_fd, Client& client, int id) : poll_fd(poll_fd), client(client), id(id) {}

ClientManager::ClientManager(pollfd& poll_fd, int id) : poll_fd(poll_fd), id(id) {}

ClientManager::~ClientManager() {}