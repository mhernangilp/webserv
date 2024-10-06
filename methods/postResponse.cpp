#include "method.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm> // Para std::replace

void HttpResponse(int client_socket, const std::string& statusCode, const std::string& contentType, const std::string& body) {
    // Construir la respuesta HTTP
    std::string response = 
        "HTTP/1.1 " + statusCode + "\r\n" +
        "Content-Type: " + contentType + "\r\n" +
        "Content-Length: " + std::to_string(body.size()) + "\r\n" +
        "Connection: close\r\n" + // Cerrar conexión después de la respuesta
        "\r\n" + // Línea en blanco que separa headers del cuerpo
        body; // Cuerpo de la respuesta
    
    // Enviar la respuesta al socket del cliente
    ssize_t sent_bytes = send(client_socket, response.c_str(), response.size(), 0);
    if (sent_bytes < 0) {
        std::cerr << "Error sending response: " << strerror(errno) << std::endl; // Manejo de errores en el envío
    }
}

void postResponse(int client_socket, Request request) {
    std::string content_type = request.getHeaders()["Content-Type"];
    
    if (content_type.find("multipart/form-data") != std::string::npos) {
        // Extraer el boundary
        std::string boundary = "--" + content_type.substr(content_type.find("boundary=") + 9);
        boundary = boundary.substr(0, boundary.find(";")); // Extrae el límite si tiene ";" al final

        // Maneja el contenido multipart
        std::string body = request.getBody();
        std::size_t pos = 0;

        // Asegurarse de que la carpeta existe
        std::string gallery_path = "docs/kebab_web/gallery";
        mkdir(gallery_path.c_str(), 0777); // Crear la carpeta si no existe

        // Iterar sobre cada parte del multipart
        while ((pos = body.find(boundary, pos)) != std::string::npos) {
            pos += boundary.length();
            std::size_t end_pos = body.find(boundary, pos);
            if (end_pos == std::string::npos) break;
            std::string part = body.substr(pos, end_pos - pos);
            pos = end_pos; // Moverse a la siguiente parte

            // Busca el campo "Content-Disposition"
            std::size_t content_disposition_pos = part.find("Content-Disposition: ");
            if (content_disposition_pos != std::string::npos) {
                std::size_t filename_pos = part.find("filename=\"");
                if (filename_pos != std::string::npos) {
                    // Extraer el nombre del archivo
                    filename_pos += 10; // Longitud de 'filename="'
                    std::size_t filename_end_pos = part.find("\"", filename_pos);
                    std::string filename = part.substr(filename_pos, filename_end_pos - filename_pos);

                    // Limpiar el nombre del archivo
                    std::replace(filename.begin(), filename.end(), '/', '_'); // Reemplaza '/' por '_'
                    std::replace(filename.begin(), filename.end(), '\\', '_'); // Reemplaza '\' por '_'

                    // Encontrar el contenido del archivo
                    std::size_t file_content_pos = part.find("\r\n\r\n") + 4; // Salto después de los headers
                    std::size_t file_content_end = part.rfind("\r\n");
                    std::string file_content = part.substr(file_content_pos, file_content_end - file_content_pos);

                    // Guardar el archivo en binario
                    std::string file_path = gallery_path + "/" + filename; // Define tu ruta
                    std::ofstream file(file_path, std::ios::binary);
                    if (file) {
                        file.write(file_content.c_str(), file_content.length());
                        file.close();
                        std::cout << "Archivo guardado: " << file_path << std::endl;
                    } else {
                        std::cerr << "No se pudo abrir el archivo para escribir: " << file_path << std::endl;
                    }
                }
            }
        }

        // Enviar respuesta de éxito
        std::string bodyResponse = "File uploaded successfully!";
        HttpResponse(client_socket, "200 OK", "text/plain", bodyResponse);
    } else {
        // Manejar otros tipos de contenido, por ejemplo, application/x-www-form-urlencoded
        std::string bodyResponse = "Unsupported content type!";
        HttpResponse(client_socket, "400 Bad Request", "text/plain", bodyResponse);
    }
}
