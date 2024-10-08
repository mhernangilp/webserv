/*
#include "method.hpp"
#include <cstring>
#include <unistd.h> // Para el uso de close
#include <errno.h>   // Para el uso de errno
#include <cstdlib>   // Para std::stoi y std::string

// Función para crear un directorio
bool createDirectory(const std::string& path) {
    return (mkdir(path.c_str(), 0755) == 0 || errno == EEXIST);
}

// Función para guardar un archivo en el sistema de archivos
bool saveFile(const std::string& filename, const std::string& content) {
    std::ofstream outfile(filename.c_str(), std::ios::binary);
    if (!outfile) {
        std::cerr << "Error al abrir el archivo: " << filename << std::endl;
        return false;
    }
    outfile.write(content.c_str(), content.size());
    outfile.close();
    return true;
}

// Función para enviar la respuesta HTTP al cliente
void HttpResponse(int client_socket, const std::string& statusCode, const std::string& contentType, const std::string& body) {
    std::string response =
        "HTTP/1.1 " + statusCode + "\r\n"
        "Content-Type: " + contentType + "\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + body;

    send(client_socket, response.c_str(), response.size(), 0);
}

// Función para parsear el body de la solicitud POST
void postResponse(const std::string& request_body, const std::string& boundary, int client_socket) {
    std::stringstream ss(request_body);
    std::string line;
    std::map<std::string, std::string> form_data;

    std::string part;
    while (std::getline(ss, line)) {
        if (line == "--" + boundary) {
            if (!part.empty()) {
                size_t pos = part.find("\r\n\r\n");
                if (pos != std::string::npos) {
                    std::string headers = part.substr(0, pos);
                    std::string body = part.substr(pos + 4); // +4 para saltar los CRLF

                    // Procesar encabezados
                    std::string content_disposition;
                    std::istringstream header_stream(headers);
                    while (std::getline(header_stream, line)) {
                        if (line.find("Content-Disposition:") != std::string::npos) {
                            content_disposition = line;
                        }
                    }

                    // Extraer el nombre del campo y el nombre del archivo
                    std::string name, filename;
                    size_t name_start = content_disposition.find("name=\"") + 6;
                    size_t name_end = content_disposition.find("\"", name_start);
                    name = content_disposition.substr(name_start, name_end - name_start);

                    if (content_disposition.find("filename=") != std::string::npos) {
                        size_t filename_start = content_disposition.find("filename=\"") + 10;
                        size_t filename_end = content_disposition.find("\"", filename_start);
                        filename = content_disposition.substr(filename_start, filename_end - filename_start);

                        // Crear directorios si no existen
                        std::string base_path = "docs/kebab_web/gallery";
                        createDirectory(base_path); // Crear el directorio base

                        // Guardar el archivo
                        if (saveFile(base_path + "/" + filename, body)) {
                            std::cout << "Archivo subido: " << filename << std::endl;
                        }
                    } else {
                        // Si no es un archivo, guardar en form_data
                        form_data[name] = body;
                    }
                }
                part.clear();
            }
        } else if (!line.empty()) {
            part += line + "\n"; // Construir la parte
        }
    }

    // Enviar una respuesta al cliente
    HttpResponse(client_socket, "204 No Content", "text/html", "");
}

// Función para recibir el cuerpo de la solicitud
std::string receiveBody(int client_socket, int content_length) {
    std::string body;
    body.resize(content_length);
    int bytes_received = 0;
    
    // Recibir datos en partes
    while (bytes_received < content_length) {
        int result = recv(client_socket, &body[bytes_received], content_length - bytes_received, 0);
        if (result <= 0) {
            std::cerr << "Error al recibir datos" << std::endl;
            return "";
        }
        bytes_received += result;
    }
    return body;
}

// Función principal para manejar la solicitud POST
void handle_post_request(int client_socket) {
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];

    // Recibir la solicitud
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        std::cerr << "Error al recibir la solicitud" << std::endl;
        return;
    }

    // Convertir el buffer a string usando assign
    std::string request;
    request.assign(buffer, bytes_received);

    // Buscar el header Content-Length para determinar la longitud del body
    size_t content_length_pos = request.find("Content-Length: ");
    if (content_length_pos != std::string::npos) {
        size_t start = content_length_pos + 16; // 16 es la longitud de "Content-Length: "
        size_t end = request.find("\r\n", start);
        
        // Validar que el substring sea un número válido
        std::string content_length_str = request.substr(start, end - start);
        if (content_length_str.empty() || !std::all_of(content_length_str.begin(), content_length_str.end(), ::isdigit)) {
            std::cerr << "Content-Length no es un número válido" << std::endl;
            return;
        }
        
        // Convertir a entero
        int content_length = std::stoi(content_length_str);
        
        // Recibir el body
        std::string body = receiveBody(client_socket, content_length);
        if (!body.empty()) {
            // Extraer el boundary del header Content-Type
            std::string boundary;
            if (request.find("Content-Type: multipart/form-data;") != std::string::npos) {
                size_t boundary_pos = request.find("boundary=") + 9; // 9 para saltar "boundary="
                boundary = request.substr(boundary_pos);
                boundary.erase(0, boundary.find_first_not_of(" \r\n")); // Quitar espacios y nuevos renglones
                boundary.erase(boundary.find_last_not_of(" \r\n") + 1); // Limpiar también el final
            }
            std::cout << "Boundary: " << boundary << std::endl;
            std::cout << "Body: " << body << std::endl;
            postResponse(body, boundary, client_socket);
        }
    } else {
        std::cerr << "No se encontró Content-Length en la solicitud" << std::endl;
    }

    close(client_socket);
}
*/


#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <stdexcept>
#include "method.hpp"

// Función para manejar la respuesta POST
void handle_post_request(int client_socket, std::string raw_request) {
    std::map<std::string, std::string> headers; // Deberías llenar esto con tus encabezados

    // Extraer el cuerpo de la solicitud
    std::string body = raw_request;

    // Verifica el header Content-Type
    if (headers.find("Content-Type") == headers.end()) {
        sendHttpResponse(client_socket, "400 Bad Request", "text/html", "<h1>400 Bad Request</h1><p>No Content-Type specified.</p>");
        return;
    }

    std::string content_type = headers["Content-Type"];
    if (content_type.find("multipart/form-data") != std::string::npos) {
        std::string filename; // Para almacenar el nombre del archivo

        // Extraer el nombre del archivo del header
        if (headers.find("filename") != headers.end()) {
            filename = headers["filename"]; // Asegúrate de que estás almacenando el nombre correctamente
        } else {
            sendHttpResponse(client_socket, "400 Bad Request", "text/html", "<h1>400 Bad Request</h1><p>No filename specified.</p>");
            return;
        }

        // Construir la ruta completa donde se guardará el archivo
        std::string filePath = "docs/kebab_web/gallery/" + filename;

        // Crear el archivo donde se va a guardar el cuerpo del POST
        std::ofstream outFile(filePath.c_str(), std::ios::out | std::ios::trunc);
        if (!outFile.is_open()) {
            // Enviar una respuesta de error HTTP 500 si no se puede abrir el archivo
            std::string body = "<html><body><h1>500 Internal Server Error</h1><p>Could not open file for writing.</p></body></html>";
            sendHttpResponse(client_socket, "500 Internal Server Error", "text/html", body);
            return;
        }

        // Guardar el contenido del cuerpo en el archivo
        outFile << body; // Asegúrate de que `body` contiene el contenido real del archivo
        outFile.close();

        // Generar una respuesta HTML simple al cliente
        std::string response_body = "<html><body><h1>POST Request Successful</h1><p>File has been uploaded successfully.</p></body></html>";
        sendHttpResponse(client_socket, "200 OK", "text/html", response_body);
    } else {
        // Manejo de otros tipos de contenido, si es necesario
        sendHttpResponse(client_socket, "415 Unsupported Media Type", "text/html", "<h1>415 Unsupported Media Type</h1><p>Only multipart/form-data is supported.</p>");
    }
}


void postResponse(std::string name, std::string body, int client_socket){
    std::ofstream outFile("docs/kebab_web/gallery/" + name);

    // Verificar si el archivo se abrió correctamente
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