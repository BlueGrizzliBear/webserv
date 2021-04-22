#ifndef WEBSERV_HPP
# define WEBSERV_HPP

/* C libraries */
# include <stdio.h>
# include <sys/socket.h>
# include <unistd.h>
# include <stdlib.h>
# include <netinet/in.h>
# include <string.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <sys/time.h>
# include <sys/wait.h>
# include <dirent.h>
# include <arpa/inet.h>
# include <errno.h>

/* C++ libraries */
# include <cstdlib>
# include <limits>
# include <iostream>
# include <string>
# include <fstream>
# include <utility>
# include <csignal>
# include <sstream>
# include <cctype>

/* STL libraries */
# include <algorithm>
# include <map>
# include <vector>
# include <list>

/* Default Config */
# define PORT 8080
# define MAX_CLIENTS 1024
# define MAX_HEADER_SIZE 65535
// # define MAX_HEADER_SIZE 819

/* Debug defines */
# define COUT std::cout
# define CIN std::cin
# define CERR std::cerr
# define ENDL std::endl

/* For debug puroposes */
# define CME std::cerr << CYAN
# define CMEY std::cerr << YELLOW
# define CMER std::cerr << RED
// #define CMEG std::cerr << GREEN
# define EME RESET << std::endl

/* Terminal Colors */
# define RESET		"\033[0m"
# define RED		"\033[0;31m"
# define GREEN		"\033[0;32m"
# define YELLOW		"\033[0;33m"
# define BLUE		"\033[0;34m"
# define MAGENTA	"\033[0;35m"
# define CYAN		"\033[0;36m"
# define WHITE		"\033[0;37m"

struct Socket
{
	int					fd;
	struct sockaddr_in	address;
	int					addrlen;
};

struct ci_less : std::binary_function < std::string, std::string, bool >
{
	struct nocase_compare : public std::binary_function< unsigned char, unsigned char, bool>
	{
		bool operator() (const char & x, const char & y) const
		{
			return tolower(x) < tolower(y);
		}
	};
	bool operator() (const std::string & x, const std::string & y) const
	{
		return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end(), nocase_compare());
	}
};

#endif
