#####################
#      SETUP        #
#####################

NAME = webserv
CC = c++
RM = rm -f
CFLAGS = -Wall -Wextra -Werror -std=c++98 -g3 -fsanitize=address

#####################
#      COLORS       #
#####################

NOC = \033[0m
YELLOW = \033[1;33m
GREEN = \033[1;32m
RED = \033[1;31m

#####################
#      FILES        #
#####################

SRCS = webserv.cpp Server.cpp Client.cpp methods/method.cpp methods/Request.cpp methods/getResponse.cpp methods/deleteResponse.cpp methods/postResponse.cpp config/ConfigParser.cpp config/LocationConfig.cpp config/ServerConfig.cpp Utils.cpp

#####################
#	RULES       #
#####################

%.o: %.cpp
	@echo "$(GREEN)[OK]$(NOC)" "\c"
	${CC} ${CFLAGS} -c $< -o $@

OBJS = ${SRCS:.cpp=.o}

all: ${NAME}

${NAME}: ${OBJS} ${INCLUDE}
	@${CC} ${CFLAGS} ${OBJS} -o ${NAME}
	@echo "$(GREEN)Exercise compiled ✓$(NOC)"

clean:
	@${RM} ${OBJS}
	@echo "$(RED)Objects deleted ✓$(NOC)"

fclean:
	@${RM} ${OBJS}
	@${RM} ${NAME}
	@echo "$(RED)Executable deleted ✓$(NOC)"

re: fclean all

.PHONY: all clean fclean re
