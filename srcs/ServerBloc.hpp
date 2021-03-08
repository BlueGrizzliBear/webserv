#ifndef SERVERBLOC_HPP
#define SERVERBLOC_HPP

#include "./webserv.hpp"
#include "./LocationBloc.hpp"
#include "./ConfigParser.hpp"

class ConfigParser;

/* ServerBloc Class Declaration */
class ServerBloc
{
	/* Member Types */
	typedef std::map<std::vector<std::string>, std::vector<std::string> >	Directives;
	typedef std::map<std::vector<std::string>, LocationBloc>				Locations;

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
		unsigned short	getPort(void);
		size_t &		getNo(void);
		ConfigParser *	getParent(void);

	/* Member Attributes */
	public:
		/* Attributes from parsing */
		Directives	serv_dir;
		Locations	serv_loc;

		/* Socket Attributes */
		int					server_fd;
		struct sockaddr_in	address;
		int					addrlen;

		/* Process Attributes */
		pid_t	pid;

	private:
		size_t			_server_no;
		ConfigParser *	_parent;
};

#endif