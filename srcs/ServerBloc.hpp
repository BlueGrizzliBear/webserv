#ifndef SERVERBLOC_HPP
# define SERVERBLOC_HPP

# include "./webserv.hpp"
# include "./LocationBloc.hpp"
# include "./ConfigParser.hpp"
# include "./Request.hpp"
# include "./Response.hpp"
# include "./Methods.hpp"

class ConfigParser;

struct Select
{
	int		fd_max;

	fd_set	readfds;
	fd_set	writefds;
	fd_set	exceptfds;

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

	/* Exceptions */
		class BadRequest : public std::exception {
			public:
				virtual const char *	what() const throw() { return ("400"); }
		};

		class Unauthorized : public std::exception {
			public:
				virtual const char *	what() const throw() { return ("401"); }
		};

		class Forbidden : public std::exception {
			public:
				virtual const char *	what() const throw() { return ("403"); }
		};

		class NotFound : public std::exception {
			public:
				virtual const char *	what() const throw() { return ("404"); }
		};

		class MethodNotAllowed : public std::exception {
			public:
				virtual const char *	what() const throw() { return ("405"); }
		};

		class PreconditionFailed : public std::exception {
			public:
				virtual const char *	what() const throw() { return ("412"); }
		};

		class PayloadTooLarge : public std::exception {
			public:
				virtual const char *	what() const throw() { return ("413"); }
		};

		class UnsupportedMediaType : public std::exception {
			public:
				virtual const char *	what() const throw() { return ("415"); }
		};

	/* Member Functions */
	public:
		/* Gets and Sets */
		size_t &		getNo(void);
		ConfigParser *	getParent(void);

		bool	readClient(Socket & client);
		void	parseException(const char * code);

		bool	processRequest(Socket & client);
		bool	sendResponse(Socket & client);

	private:
		/* Response functions */
		std::string	_getDate(void);
		void		_addHeaderFields(void);

	/* Member Attributes */
	public:
		/* Attributes from parsing */
		Directives	dir;
		Locations	loc;

		/* Attributes from initialization */
		unsigned short	port_no;
		bool			is_default;
		Socket			serv_port;
		Select			serv_select;

		std::list<Socket> clientList;

		/* Attributes from RequestParsing */
		Request		req;
		Response 	resp;

		/* Process Attributes */
		pid_t	pid;

	private:
		/* ConfigFile Parsing Utilities */
		size_t			_server_no;
		ConfigParser *	_parent;
};

#endif
