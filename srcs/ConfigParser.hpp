#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include "./webserv.hpp"
# include "./ServerDictionary.hpp"
# include "./LocationBloc.hpp"
# include "./ServerBloc.hpp"

class ServerBloc;

/* ConfigParser Class Declaration */
class ConfigParser
{
	public:
	/* Member Types */
		typedef ServerDictionary::Dic		Dic;
		typedef std::vector<ServerBloc>	Servers;
		typedef std::map<std::vector<std::string>, LocationBloc>	Locations;
		typedef std::map<std::string, std::vector<std::string> >	Directives;

	/* Constructor */
		/*	default		(1)	*/	ConfigParser(void);
		/*	argument	(2)	*/	ConfigParser(const char * path, char const ** envp);
		/*	copy		(3)	*/	ConfigParser(ConfigParser const & cpy);

	/* Destructor */
		~ConfigParser();

	/* Operators */
		ConfigParser &	operator=(ConfigParser const & rhs);

	/* Exceptions */
		class FileAccess : public std::exception
		{
			public:
				virtual const char *	what() const throw() { return (strerror(errno)); }
		};

		class ItemNotFile : public std::exception
		{
			public:
				virtual const char *	what() const throw() { return ("Error: Invalid file."); }
		};

		class FileOpen : public std::exception
		{
			public:
				virtual const char *	what() const throw() { return ("Fatal error: Could not open file."); }
		};

		class InvalidKey : public std::exception
		{
			public:
				virtual const char *	what() const throw() { return ("Error: Invalid key in Configuration File"); }
		};

		class MissingKey : public std::exception
		{
			public:
				virtual const char *	what() const throw() { return ("Error: Missing key in Configuration File"); }
		};

		class RedundantDirKey : public std::exception
		{
			public:
				virtual const char *	what() const throw() { return ("Error: Directive Key should be unique."); }
		};

		class UnexpectedToken : public std::exception
		{
			public:
				virtual const char *	what() const throw() { return ("Error: Unexpected Token in Configuration File"); }
		};

		class MissingArgument : public std::exception
		{
			public:
				virtual const char *	what() const throw() { return ("Error: Missing argument in Configuration File"); }
		};

		class ExitProgram : public std::exception
		{
			public:
				virtual const char *	what() const throw() { return ("Exiting program."); }
		};

	public:
	/* Gets and Sets */
		ServerDictionary &	getDictionary(void);
		Servers &			getServers(void);
		Directives &		getMainDirs(void);
		const char **		getEnvp(void);

	/* Member Functions */
		void	closeServerSockets(const char * main_err, const char * err);
		/* void	display_config(void); */

	private:
		bool	_is_in_dictionnary(Dic dic, std::string word);

		void	_display_parsing_error(size_t new_count);

		void	_try_open_file(const char * path);
		void	_parse_main_context(std::fstream & file, Dic dic, Directives & dir, Servers & serv, Locations & loc);
		void	_parse_server(std::string & key, std::fstream & file, Servers & serv);
		void	_parse_location(std::string & key, std::fstream & file, Servers & serv, Locations & loc);
		void	_parse_directive(std::string & key, Directives & dir);
		bool	_str_is_digit(std::string const & str);
		int		_check_directive(std::string & key, std::vector<std::string> & values);

		void	_verify_uniqueness(ServerBloc & serv, std::string str);
		void	_verify_serverbloc(ServerBloc & serv);

		void	_setPortNo(void);
		void	_initDefaultServer(ServerBloc & serv);
		void	_initPort(ServerBloc & serv);
		void	_setNonDefaultServers(void);
		void	_initServers(void);

	/* Member Attributes */
	private:
		ServerDictionary	_dic;

		std::string			_path;
		std::string			_line;
		size_t				_line_no;
		size_t				_count;
		size_t				_bracket;

		const char **	_envp;

		Directives	_main_dir;
		Servers		_servers;

	/* Static Functions for Debug */
		static void	_display_string(std::string const & str);
		static void	_display_arguments(std::string const & str);
		static void	_display_dir(std::pair<const std::string, std::vector<std::string> > & pair);
		static void	_display_location_bloc(std::pair<const std::vector<std::string>, LocationBloc> & pair);
		static void	_display_server_bloc(ServerBloc & serv);
};

#endif
