#ifndef SERVERBLOC_HPP
#define SERVERBLOC_HPP

#include "./webserv.hpp"
#include "./LocationBloc.hpp"
#include "./ConfigParser.hpp"
#include "./Request.hpp"
#include "./Response.hpp"

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
	int				max;
	char			buf[1024];
	fd_set			readfds;
	fd_set			writefds;
	fd_set			exceptfds;
	struct timeval	timeout;
	bool			finished;
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
		size_t &		getNo(void);
		ConfigParser *	getParent(void);

		void	parseRequest(const char * request);
		void	executeRequest(void);
		void	sendResponse(Socket & client);

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