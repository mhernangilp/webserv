#include "method.hpp"

int cgi_function(Request request, const ServerConfig& serverConfig, int client_socket, std::string name){
    std::string script_path = serverConfig.root + "/cgi-bin/checker.php";
        std::string response_body;

        std::string exists = "NO";
        if (fileExists(serverConfig.root + "/fake-gallery/" + name))
            exists = "YES";
        std::string command = "php " + script_path + " " + "\"" + name + "\"" + " " + exists;
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
        close (client_socket);
        return (request.getCode());
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

    if (request.getUrl() == "/cgi-bin/checker.php")
        return (cgi_function(request, serverConfig, client_socket, name));

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

        std::string response_body = getFileContent(serverConfig.root + "upload/upload_done.html");
        if (response_body.empty())
            std::string response_body = "<html><body><h1>POST Request Successful</h1><p>File has been uploaded successfully.</p></body></html>";
        sendHttpResponse(client_socket, "200 OK", "text/html", response_body);
        request.setCode(200);
    }
    close (client_socket);
    return (request.getCode());
}