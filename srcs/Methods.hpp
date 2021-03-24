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

	private:
		/* Method functions */
		void	_applyGet(void);
		void	_applyHead(void);
		void	_applyPost();
		void	_findPath(void);
		void	_checkContentType(void);
		void	_fillBody(void);

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

	/* Member Attributes */
	public:
		ServerBloc	* serv;

	private:
		/* Method Utilities */
		std::string		_path;
};

#endif
