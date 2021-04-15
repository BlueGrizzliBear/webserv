#ifndef METHODS_HPP
# define METHODS_HPP

# include "./webserv.hpp"
# include "./LocationBloc.hpp"
# include "./ConfigParser.hpp"
# include "./Request.hpp"
# include "./Response.hpp"

class ServerBloc;
struct Client;

/* ServerBloc Class Declaration */
class Methods
{
	/* Member Types */
	public:
		typedef std::map<std::string, std::vector<std::string> >	Directives;
		typedef std::map<std::vector<std::string>, LocationBloc>	Locations;

	/* Constructor */
	public:
		/*	default		(1)	*/	Methods(void);
		/*	parent		(2)	*/	Methods(ServerBloc & server, Client & client, std::string const & code = "", std::string const & phrase = "");
		/*	copy		(3)	*/	Methods(Methods const & cpy);

	/* Destructor */
		~Methods();

	/* Operators */
		Methods &	operator=(Methods const & rhs);

	/* Member Functions */
	public:
		void	execute(void);
		void	customError(std::string const & status_code, std::string const & reason_phrase);

	private:
		bool	_ErrorNbInErrorPageList(std::vector<std::string> & list, std::string const & status);
		void	_fillDefaultExceptionBody(std::string const & status, std::string const & reason);

		/* Method functions */
		void	_URIResolutionProcess(void);
		void	_queryResolutionProcess(void);
		void	_applyGet(void);
		void	_applyHead(void);
		void	_applyPost(void);
		void	_applyPut(void);

		/* Execute Get request */
		void	_executeGetReq(void);
		bool	_isDirectory(std::string const & path);
		void	_createIndexHTML(void);
		void	_createHTMLListing(DIR * dir);
		void	_findIndex(std::vector<std::string> & indexes);
		bool	_fileExist(std::string const & path);

		/* Execute Put request */
		void	_executePutReq(void);

		/* Execute Post request */
		void	_executePostReq(void);

		/* Check content type */
		void		_checkContentType(void);
		std::string	_pathExtension(std::string const & path);

		/* Fill Body */
		void		_fillStrFromFile(std::string & body, std::string const & path);
		std::string	_getSizeOfStr(std::string const & str);

	/* MethodsPath.cpp */
		/* Find path (_path) */
			/* (1) find server config */
		void	_findPath(void);

		template< typename T, typename U >
		void	_findAuthenticate(std::map< T, U > & dir);

		template< typename T, typename U >
		void	_findRoot(std::map< T, U > & dir);

		template< typename T, typename U >
		void	_findAutoIndex(std::map< T, U > & dir);

		template< typename T, typename U >
		void	_findCGIPath(std::map< T, U > & dir);

		template< typename T, typename U >
		void	_findClientMaxBodySize(std::map< T, U > & dir, size_t * max_size);

		template< typename T, typename U >
		void	_findVect(std::map< T, U > & dir, std::string to_find, std::vector<std::string> * vect);

		template< typename T, typename U >
		std::string	_findRewrite(std::map< T, U > & dir);

		std::string	_uriWithoutFirstPart();
			/* (2) find location bloc config */
		bool		_matchingLocationDir(std::map<std::vector<std::string>, LocationBloc>::iterator & it, std::map<std::vector<std::string>, LocationBloc>::iterator & tmp);
		bool		_applyLocationBloc(std::map<std::vector<std::string>, LocationBloc>::iterator & it);

		bool		_compareCapturingGroup(std::string uri_path, std::string cap_grp);
		bool		_compareFromEnd(std::string & uri_path, std::vector<std::string> & path_set);
		bool		_compareFromBegin(std::string & uri_path, std::vector<std::string> & path_set);

		std::string	_uriFirstPart(void);

			/* (3) authentication */
		void		_checkRequiredAuthentication(void);
		bool		_checkUserExist(std::string & user, std::string & auth_path);
		std::string	_decodeUser(std::string & user);

			/* (4) allowed method */
		void		_checkAllowedMethods(void);

			/* (5) max_body_size */
		void		_checkMaxBodySize(void);

	/* MethodsHeader.cpp */
		/* Header server response */
		void	_PutHeaderStatusCode(void);
		void	_GetHeaderStatusCode(void);
		void	_lastModifiedHeader(struct tm * timeinfo);
		struct tm	* _getFileTime(void);
		struct tm	* _getHeaderIfUnmodifiedSinceTime(void);
		int			_cmpTimeInfo(struct tm * t1, struct tm * t2);
		std::string	_readFileToStr(void);

	/* MethodsCGI.cpp */


		void		_launchCGI(void);

		void 		_createEnvpMap(void);
		void		_createArgvMap(void);
		char **		_createEnvpArray(void);
		char **		_createArgvArray(void);

		void		_freeArray(char ** array);
		void		_displayArray(char ** array);

		void		_communicateWithCGI(int fd_in, int fd_out, pid_t pid);

		bool		_writeReqtoCGI(int & fd_out);
		bool		_readCGItoResp(int & fd_in);

		/* Utitilies */
		bool		_str_is(std::string str, int func(int));

		void		_parseCGIResponse(void);
		bool		_parseHeaderField(void);

	/* Member Attributes */
	public:
		ServerBloc	* serv;
		Client		* client;

	private:
		/* Method Utilities */
		std::string					_path;

		std::vector<std::string>	_methods;
		size_t						_max_body_size;
		std::string					_query;
		std::string					_cgi_path;

		std::vector<std::string>	_authenticate;
		std::vector<std::string>	_indexes;
		bool						_autoindex;

		std::map<std::string, std::string, ci_less>	_envp;
		std::vector<std::string>					_argv;

		size_t	_writtenBytes;
		std::string _receivedMessage;

};

#endif
