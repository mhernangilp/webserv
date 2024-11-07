#ifndef METHOD_HPP
#define METHOD_HPP 

#include "Request.hpp"
#include "../config/ServerConfig.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "signal.h"

int getResponse(Request request, int client_socket, const ServerConfig& serverConfig);
int deleteResponse(Request request, int client_socket, const ServerConfig& serverConfig);
int postResponse(Request request, int client_socket, const ServerConfig& serverConfig);

std::string convertToString(int number);
std::string getFileContent(const std::string& filePath);
std::string urlDecode(const std::string& url);
bool isDirectory(const std::string& path);
int checkdir(const std::string& url, int client_socket);
bool urlRecoil(const std::string &url);
std::string getErrorPage(int error, const ServerConfig& serverConfig);
bool fileExists(const std::string& filename);
void sendHttpResponse(int client_socket, const std::string& statusCode, const std::string& contentType, const std::string& body);

#endif