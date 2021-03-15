#ifndef SERVERBLOC_HPP
#define SERVERBLOC_HPP

#include "./webserv.hpp"
#include "./LocationBloc.hpp"
#include "./ConfigParser.hpp"

class ConfigParser;

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
	bool			finished;
	int				max;
	fd_set			readfds;
	char			buf[1024];
	fd_set			writefds;
	fd_set			exceptfds;
	struct timeval	timeout;
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

	/* Member Attributes */
	public:
		/* Attributes from parsing */
		Directives	serv_dir;
		Locations	serv_loc;

		/* Attributes from initialization */
		Socket		serv_port;
		Select		serv_select;

		/* Process Attributes */
		pid_t	pid;

	private:
		size_t			_server_no;
		ConfigParser *	_parent;
};

#endif