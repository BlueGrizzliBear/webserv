#ifndef SERVERBLOC_HPP
# define SERVERBLOC_HPP

# include <sys/time.h>
# include "./webserv.hpp"
# include "./LocationBloc.hpp"
# include "./ConfigParser.hpp"
# include "./Request.hpp"
# include "./Response.hpp"

class ConfigParser;
// class Request;

struct Socket
{
	int					fd;
	unsigned short		port_no;
	bool				is_default;
	struct sockaddr_in	address;
	int					addrlen;
};

struct Select
{
	int					fd_max;
	char				buf[MAX_HEADER_SIZE];

	ssize_t			n;

	fd_set			readfds;
	fd_set			writefds;
	fd_set			exceptfds;

	struct timeval	timeout;

	bool			incomplete;
};

/* ServerBloc Class Declaration */
class ServerBloc
{
	/* Member Types */
	public:
		typedef std::map<std::string, std::vector<std::string> >	Directives;
		typedef std::map<std::vector<std::string>, LocationBloc>	Locations;

	/* Constructor */
	public:
		/*	default		(1)	*/	ServerBloc(void);
		/*	by parent	(2)	*/	ServerBloc(ConfigParser * parent);
		/*	copy		(2)	*/	ServerBloc(ServerBloc const & cpy);

	/* Destructor */
		~ServerBloc();

	/* Operators */
		ServerBloc &	operator=(ServerBloc const & rhs);

	/* Member Functions */
	public:
		/* Gets and Sets */
		size_t &		getNo(void);
		ConfigParser *	getParent(void);

		void	readClient(int client_socket);
		void	parseException(const char * code);

		void	processRequest(void);
		void	executeRequest(void);
		void	sendResponse(Socket & client);

	private:
		/* Method functions */
		void		_applyGet(void);

		/* Response functions */
		std::string	_getSizeOfBody(void);
		std::string	_getDate(void);
		std::string	_concatenateResponse(void);

	/* Member Attributes */
	public:
		/* Attributes from parsing */
		Directives	dir;
		Locations	loc;

		/* Attributes from initialization */
		Socket		serv_port;
		Select		serv_select;

		/* Attributes from RequestParsing */
		Request 	req;
		Response 	resp;

		/* Process Attributes */
		pid_t	pid;

	private:
		size_t			_server_no;
		ConfigParser *	_parent;
};

#endif
