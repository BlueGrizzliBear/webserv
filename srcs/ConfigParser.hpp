#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "./webserv.hpp"
#include "./ServerDictionary.hpp"

struct	server_bloc
{
	unsigned int				listen_port;
	std::vector<std::string>	server_names;
	std::map<std::string, std::vector<std::string> > _directives;
	// std::vector<std::string>	directives;
	std::map<std::vector<std::string>, std::multimap<std::string, std::string> >	locations;
};

class ConfigParser
{
	/* Constructor */
	private:
		/*	default		(1)	*/	ConfigParser(void);
		/*	copy		(3)	*/	ConfigParser(ConfigParser const & cpy);

	public:
		/*	argument	(2)	*/	ConfigParser(const char * path);

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

	/* Member Functions */
	private:
		bool	_is_in_dictionnary(std::map<std::string, int> dic, std::string word);

		void	_display_error(void);

		void	_parse_main_context(std::fstream & file, std::map<std::string, int> dic);

		void	_parse_context(std::string & key, std::fstream & file);
		void	_parse_directive(std::string & key);

	/* Member Attributes */
	private:
		ServerDictionary	_dic;
		
		std::string	_path;
		std::string	_line;
		size_t		_line_no;
		size_t		_count;
		bool		_bracket;

		std::map<std::string, std::vector<std::string> >	_directives;
		std::vector<server_bloc>	_servers;
};

#endif
