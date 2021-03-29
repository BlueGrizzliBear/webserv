#ifndef METHODS_HPP
# define METHODS_HPP

# include "./webserv.hpp"
# include "./LocationBloc.hpp"
# include "./ConfigParser.hpp"
# include "./Request.hpp"
# include "./Response.hpp"

class ServerBloc;

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
		/*	by parent	(2)	*/	Methods(ServerBloc & server);
		/*	copy		(2)	*/	Methods(Methods const & cpy);

	/* Destructor */
		~Methods();

	/* Operators */
		Methods &	operator=(Methods const & rhs);

	/* Member Functions */
	public:
		void	execute(void);
		void	customError(std::string status_code, std::string reason_phrase);

	private:
		bool	_ErrorNbInErrorPageList(std::vector<std::string> list, std::string status);
		void	_fillDefaultExceptionBody(std::string status, std::string reason);

		/* Method functions */
		void	_URIResolutionProcess(void);
		void	_applyGet(void);
		void	_applyHead(void);
		void	_applyPost(void);
		void	_applyPut(void);
		/* Execute Get request */
		void	_executeGetReq(void);
		bool	_isDirectory(std::string const & path);
		void	_createIndexHTML(void);
		void	_createHTMLListing(DIR * dir);
		void	_findIndex(std::vector<std::string> indexes);
		bool	_fileExist(std::string const & path);

		/* Execute Put request */
		void	_executePutReq(void);

		/* Check content type */
		void		_checkContentType(void);
		std::string	_pathExtension(std::string const & path);

		/* Fill Body */
		void	_fillBody(void);
		std::string	_getSizeOfStr(std::string const & str);

	/* MethodsPath.cpp */
		/* Find path (_path) */
			/* (1) find server config */
		void	_findPath(void);

		template< typename T, typename U >
		void	_findRoot(std::map< T, U > root);

		template< typename T, typename U >
		void	_findAutoIndex(std::map< T, U > dir);

		template< typename T, typename U >
		std::vector<std::string>	_findVect(std::map< T, U > dir, std::string to_find, std::vector<std::string> vect);

		template< typename T, typename U >
		std::string	_findRewrite(std::map< T, U > dir);
		std::string	_uriWithoutFirstPart();
			/* (2) find location bloc config */
		bool		_matchingLocationDir(std::map<std::vector<std::string>, LocationBloc>::iterator it, std::map<std::string, std::vector<std::string> > * location_dir);
		bool		_isRegex(std::string str);
		bool		_compareCapturingGroup(std::string uri_path, std::string capturing_group);
		bool		_compareFromEnd(std::string uri_path, std::vector<std::string> path_set);
		bool		_compareFromBegin(std::string uri_path, std::vector<std::string> path_set);
		std::string	_toLowerStr(std::string const & str);
		std::string	_uriFirstPart();
			/* (3) allowed method */
		void		_checkAllowedMethods(std::vector<std::string> methods);


	/* MethodsHeader.cpp */
		/* Header server response */
		void	_PutHeaderStatusCode(void);
		void	_GetHeaderStatusCode(void);
		void	_lastModifiedHeader(struct tm *timeinfo);
		struct tm	*_getFileTime(void);
		struct tm	*_getHeaderIfUnmodifiedSinceTime(void);
		int			_cmpTimeInfo(struct tm *t1, struct tm *t2);
		std::string	_readFileToStr(void);


	/* Member Attributes */
	public:
		ServerBloc	* serv;

	private:
		/* Method Utilities */
		std::string					_path;
		std::vector<std::string>	_indexes;
		bool						_autoindex;
};

#endif
