#include "method.hpp"

bool fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

int postResponse(Request request, int client_socket, const ServerConfig& serverConfig){

    std::string body = request.getBody();
    std::string name = request.getFileName();

    std::string newUrl = serverConfig.host;
    newUrl = request.getUrl();
    if (access(request.getUrl().c_str(), F_OK) == -1)
        newUrl = urlDecode(request.getUrl());
   if (!serverConfig.isMethodAllowed(newUrl, 'P') || urlRecoil(request.getUrl())) {
        std::string response_body = getFileContent(getErrorPage(405, serverConfig));
        if (response_body.empty())
            response_body = "<html><body><h1>405 Method Not Allowed</h1><p>POST is not allowed in this ubication.</p></body></html>";
        sendHttpResponse(client_socket, "405 Method Not Allowed", "text/html", response_body);
        request.setCode(405);
        close(client_socket);
        return (405);
    }
    if (request.getCode() == 400){
        std::string response_body = getFileContent(getErrorPage(400, serverConfig));
        if (response_body.empty())
            std::string response_body = "<html><body><h1>400 Bad Request</h1><p>No filename specified.</p></body></html>";
        sendHttpResponse(client_socket, "400 Bad Request", "text/html", response_body);
        request.setCode(400);
        close(client_socket);
        return (400);
    }
    else if (request.getCode() == 401){
        std::string response_body = getFileContent(getErrorPage(401, serverConfig));
        if (response_body.empty())
            std::string response_body = "<html><body><h1>401 Bad Request</h1><p>No header specified.</p></body></html>";
        sendHttpResponse(client_socket, "401 Bad Request", "text/html", response_body);
        request.setCode(401);
        close(client_socket);
        return (401);
    }
    else if (request.getCode() == 415){
        std::string response_body = getFileContent(getErrorPage(415, serverConfig));
        if (response_body.empty())
            std::string response_body = "<html><body><h1>415 Bad Request</h1><p>No boundary specified.</p></body></html>";
        sendHttpResponse(client_socket, "415 Bad Request", "text/html", response_body);
        request.setCode(415);
        close(client_socket);
        return (415);
    }

    std::string full_path = serverConfig.root + "fake-gallery/" + name;
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
        full_path = serverConfig.root + "fake-gallery/" + new_filename.str();
        file_index++;
    }

    std::ofstream outFile(full_path.c_str());
    if (!outFile) {
        std::string response_body = getFileContent(getErrorPage(500, serverConfig));
        if (response_body.empty())
            std::string response_body = "<html><body><h1>500 Internal Server Error</h1><p>Could not open file for writing.</p></body></html>";
        sendHttpResponse(client_socket, "500 Internal Server Error", "text/html", response_body);
        request.setCode(500);
    } else{
        outFile << body;
        outFile.close();

        // Ejecutar el script CGI y capturar su salida
        std::string script_path = serverConfig.root + "/upload/upload_response.php";
        std::string command = "php " + script_path + " " + base_name + extension;
        std::string response_body;

        FILE* pipe = popen(command.c_str(), "r");
        if (pipe) {
            char buffer[128];
            char *fg = fgets(buffer, sizeof(buffer), pipe);
            if (fg != NULL){
                while (fg != NULL) {
                    response_body += buffer;
                    fg = fgets(buffer, sizeof(buffer), pipe);
                }
            }
            else{
                std::string response_body = getFileContent(getErrorPage(500, serverConfig));
                if (response_body.empty())
                    std::string response_body = "<html><body><h1>500 Internal Server Error</h1><p>Could not open file for writing.</p></body></html>";
                sendHttpResponse(client_socket, "500 Internal Server Error", "text/html", response_body);
                request.setCode(500);
                close (client_socket);
                return (500);
            }
            pclose(pipe);
            sendHttpResponse(client_socket, "200 OK", "text/html", response_body);
            request.setCode(200);
        } else {
            std::string response_body = getFileContent(getErrorPage(500, serverConfig));
            if (response_body.empty())
                std::string response_body = "<html><body><h1>500 Internal Server Error</h1><p>Could not open file for writing.</p></body></html>";
            sendHttpResponse(client_socket, "500 Internal Server Error", "text/html", response_body);
            request.setCode(500);
        }
    }
    close (client_socket);
    return (request.getCode());
}