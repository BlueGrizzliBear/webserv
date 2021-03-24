#ifndef SERVERBLOC_HPP
# define SERVERBLOC_HPP

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
		class SeeOther : public std::exception {
			public:
				virtual const char *	what() const throw() { return ("303"); }
		};

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

		class UnsupportedMediaType : public std::exception {
			public:
				virtual const char *	what() const throw() { return ("415"); }
		};

	/* Member Functions */
	public:
		/* Gets and Sets */
		size_t &		getNo(void);
		ConfigParser *	getParent(void);

		bool	readClient(int client_socket);
		void	parseException(const char * code);

		bool	processRequest(void);
		void	executeRequest(void);
		bool	sendResponse(Socket & client);

	private:
		/* Method functions */
		void		_applyGet(void);
		void		_applyHead(void);
		void		_applyPost();
		void		_findPath(void);
		void		_checkContentType(void);
		void		_fillBody(void);

		/* usefull method function */
		template< typename T, typename U >
		std::string	_findRoot(std::map< T, U > root);

		template< typename T, typename U >
		bool		_findAutoIndex(std::map< T, U > dir, bool autoindex);

		template< typename T, typename U >
		std::vector<std::string>	_findVect(std::map< T, U > dir, std::string to_find, std::vector<std::string> vect);

		template< typename T, typename U >
		std::string		_findRewrite(std::map< T, U > dir);

		bool			_isRegex(std::string str);
		void			_matchingLocationDir(std::map<std::vector<std::string>, LocationBloc>::iterator it, bool * break_loc, std::map<std::string, std::vector<std::string> > * location_dir);
		bool			_compareCapturingGroup(std::string uri_path, std::string capturing_group);
		std::string		_toLowerStr(std::string const & str);
		bool			_compareFromEnd(std::string uri_path, std::vector<std::string> path_set);
		bool			_compareFromBegin(std::string uri_path, std::vector<std::string> path_set);
		void			_checkAllowedMethods(std::vector<std::string> methods);

		bool			_fileExist(std::string const & path);
		bool			_isDirectory(std::string const & path);
		void			_findIndex(std::vector<std::string> indexes);
		std::string		_uriFirstPart();
		std::string		_uriWithoutFirstPart();
		std::string		_pathExtension(std::string const & path);
		// void			_createIndexHTML(void);

		/* Response functions */
		std::string	_getSizeOfStr(std::string const & str);
		std::string	_getDate(void);
		void		_addHeaderFields(void);

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
		/* Method Utilities */
		std::string		_path;

		/* ConfigFile Parsing Utilities */
		size_t			_server_no;
		ConfigParser *	_parent;

};


#endif
