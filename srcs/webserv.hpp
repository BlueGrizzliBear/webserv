#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>

#include <limits>
#include <iostream>
#include <string>
#include <fstream>
#include <utility>
#include <csignal>

#include <map>
#include <vector>

/* Default Config */
#define PORT 8080
#define BACK_LOG 128

#define COUT std::cout
#define CIN std::cin
#define CERR std::cerr
#define ENDL std::endl

/* Debug for me */
#define CME std::cerr << CYAN
// #define CMEG std::cerr << GREEN
#define EME RESET << std::endl

# define RESET		"\033[0m"
# define RED		"\033[0;31m"
# define GREEN		"\033[0;32m"
# define YELLOW		"\033[0;33m"
# define BLUE		"\033[0;34m"
# define MAGENTA	"\033[0;35m"
# define CYAN		"\033[0;36m"
# define WHITE		"\033[0;37m"

#endif
