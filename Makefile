SRC_PATH		=		./srcs/

SRCS_NAME		=		webserv.cpp	\
						ConfigParser.cpp \
						ServerDictionary.cpp \
						LocationBloc.cpp \
						Request.cpp \
						Response.cpp \
						ServerBloc.cpp \
						Methods.cpp \
						MethodsPath.cpp \
						MethodsHeader.cpp \
						MethodsCGI.cpp

OBJS			= 		${SRCS:.cpp=.o}

NAME			=		./webserv

COMPIL			=		clang++
# FLAGS			+= 		trying something out again
# FLAGS			+=		-Wall -Wextra -Werror -Wconversion -std=c++98 -fsanitize=address -g3
FLAGS			+=		-Wall -Wextra -Werror -Wconversion -std=c++98 -g3

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

						rm -f ./YoupiBanane/post_body

						rm -f ./data/youpi_put_test/file_should_exist_after
						rm -f ./data/youpi_put_test/multiple_same

						rm -rf ./ultimate_tester/__pycache__
						rm -rf ./ultimate_tester/testcase/__pycache__

						rm -rf ./ulti_tester/long.txt
						rm -rf ./ulti_tester/tmp

re				:      	fclean all

.PHONY			:		all clean fclean re
