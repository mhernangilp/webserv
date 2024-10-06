#ifndef METHOD_HPP
#define METHOD_HPP 

#include "Request.hpp"
#include "../config/ServerConfig.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

void getResponse(const std::string& url, int client_socket, const ServerConfig& serverConfig);
void deleteResponse(const std::string& url, int client_socket, const ServerConfig& serverConfig);
void postResponse(int socket, Request request);

std::string convertToString(int number);
std::string getFileContent(const std::string& filePath);
std::string urlDecode(const std::string& url);

#endif