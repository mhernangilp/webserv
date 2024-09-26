#include "Request.hpp"


void method(Request request, int socket){
	std::cout << "----------------------------------------------" << std::endl;
	std::cout << request.getMethod() << std::endl;
	std::cout << request.getUrl() << std::endl;
	std::cout << request.getHttpVersion() << std::endl;
	std::cout << request.getHost() << std::endl;
	std::cout << request.getHeaders()["User-Agent"] << std::endl;
	std::cout << request.getBody() << std::endl;
	std::cout << "----------------------------------------------" << std::endl;
						
	if (request.getMethod() == "GET")
		getResponse(request.getUrl(), socket);
	if (request.getMethod() == "DELETE")
		deleteResponse(request.getUrl(), socket);
}
