SRC_PATH		=		./srcs/

SRCS_NAME		=		webserv.cpp	\
						ConfigParser.cpp \
						ServerDictionary.cpp \
						ServerBloc.cpp \
						LocationBloc.cpp

OBJS			= 		${SRCS:.cpp=.o}

NAME			=		./webserv

COMPIL			=		clang++

FLAGS			+=		-Wall -Wextra -Werror -Wconversion -std=c++98

SRCS			=		$(addprefix $(SRC_PATH),$(SRCS_NAME))

all				:		./webserv $(NAME)

%.o				:		%.cpp $(HEADER)
						$(COMPIL) $(FLAGS) -c $< -o $(<:.cpp=.o)

$(NAME)			:		$(OBJS)
						$(COMPIL) $(FLAGS) $(OBJS) -o $(NAME)

clean			:
						rm -f $(OBJS)

fclean			:		clean
						rm -f $(NAME)

re				:      	fclean all

.PHONY			:		all clean fclean re
