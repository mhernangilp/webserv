#include "method.hpp"

bool fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

void postResponse(Request request, int client_socket, const ServerConfig& serverConfig){

    std::string body = request.getBody();
    std::string name = request.getFileName();

    std::string newUrl = request.getUrl();
    if (access(request.getUrl().c_str(), F_OK) == -1)
        newUrl = urlDecode(request.getUrl());
    if (!serverConfig.isDeleteAllowed(newUrl)) {
        std::string response_body = getFileContent("docs/kebab_web/error_pages/405.html");
        if (response_body.empty())
            response_body = "<html><body><h1>405 Method Not Allowed</h1><p>POST is not allowed in this ubication.</p></body></html>";
        sendHttpResponse(client_socket, "405 Method Not Allowed", "text/html", response_body);
        close(client_socket);
        return;
    }
    if (request.getCode() == 400){
        std::string response_body = getFileContent("docs/kebab_web/error_pages/400.html");
        if (response_body.empty())
            std::string response_body = "<html><body><h1>400 Bad Request</h1><p>No filename specified.</p></body></html>";
        sendHttpResponse(client_socket, "400 Bad Request", "text/html", response_body);
        return;
    }
    else if (request.getCode() == 401){
        std::string response_body = getFileContent("docs/kebab_web/error_pages/400.html");
        if (response_body.empty())
            std::string response_body = "<html><body><h1>400 Bad Request</h1><p>No header specified.</p></body></html>";
        sendHttpResponse(client_socket, "400 Bad Request", "text/html", response_body);
        return;
    }
    else if (request.getCode() == 415){
        std::string response_body = getFileContent("docs/kebab_web/error_pages/400.html");
        if (response_body.empty())
            std::string response_body = "<html><body><h1>400 Bad Request</h1><p>No boundary specified.</p></body></html>";
        sendHttpResponse(client_socket, "400 Bad Request", "text/html", response_body);
        return;
    }

    std::string full_path = "docs/kebab_web/gallery/" + name;
    std::string base_name = name;
    std::string extension = "";
    // Separar el nombre base y la extensión si tiene.
    size_t dot_pos = name.find_last_of('.');
    if (dot_pos != std::string::npos) {
        base_name = name.substr(0, dot_pos);
        extension = name.substr(dot_pos);
    }
    int file_index = 1;
    while (fileExists(full_path)) {
        std::ostringstream new_filename;
        new_filename << base_name << "(" << file_index << ")" << extension;
        full_path = "docs/kebab_web/gallery/" + new_filename.str();
        file_index++;
    }

    std::ofstream outFile(full_path.c_str());
    if (!outFile) {
        std::string response_body = getFileContent("docs/kebab_web/error_pages/500.html");
        if (response_body.empty())
            std::string response_body = "<html><body><h1>500 Internal Server Error</h1><p>Could not open file for writing.</p></body></html>";
        sendHttpResponse(client_socket, "500 Internal Server Error", "text/html", response_body);
        return; 
    } else{
        outFile << body;
        outFile.close();
        std::string response_body = "<html><body><h1>POST Request Successful</h1><p>File has been uploaded successfully.</p></body></html>";
        sendHttpResponse(client_socket, "200 OK", "text/html", response_body);
    }
    close (client_socket);
}