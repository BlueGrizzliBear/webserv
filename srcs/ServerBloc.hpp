#ifndef SERVERBLOC_HPP
# define SERVERBLOC_HPP

# include "./webserv.hpp"
# include "./LocationBloc.hpp"
# include "./ConfigParser.hpp"
# include "./Request.hpp"
# include "./Response.hpp"
# include "./Methods.hpp"

class ConfigParser;

struct Client
{
	Socket		socket;
	Request		req;
	Response 	resp;

	ServerBloc * serv;

	bool	finishedReading;
	bool	clientClosed;
};

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

		class InternalServerError : public std::exception {
			public:
				virtual const char *	what() const throw() { return ("500"); }
		};

	public:
	/* Gets and Sets */
		size_t &		getNo(void);
		ConfigParser *	getParent(void);

	/* Member Functions */
		bool	readClient(Client & client);
		void	parseException(Client & client, const char * code);

		bool	processRequest(Client & client);
		bool	sendResponse(Client & client);

	private:
		std::string	_createDate(void);
		void		_addHeaderFields(Client & client);

	public:
	/* Member Attributes */
		/* Attributes from parsing */
		Directives	dir;
		Locations	loc;
		unsigned short	port_no;
		bool			is_default;
		bool			is_unique;

		/* Socket attributes of the server */
		Socket			serv_port;

		/* Utilities for IO operations */
		Select			serv_select;

		std::list<Client>	clientList;
		static int totalClients;

	private:
		size_t			_server_no;
		ConfigParser *	_parent;
};

#endif
