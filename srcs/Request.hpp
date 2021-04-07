#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "./webserv.hpp"
# include "./ServerDictionary.hpp"
// #include "./ServerBloc.hpp"

class ServerBloc;

/* Request Class Declaration */
class Request
{
	public:
	/* Member Types */
		typedef std::map<std::string, std::string > Headers;

	/* Constructor */
		/*	default		(1)	*/	Request(void);
		/*	copy		(2)	*/	Request(Request const & cpy);

	/* Destructor */
		~Request();

	/* Operators */
		Request &	operator=(Request const & rhs);

	/* Exceptions */
		class BadRequest : public std::exception {
			public:
				virtual const char *	what() const throw() { return ("400"); }
		};

		class NotImplemented : public std::exception {
			public:
				virtual const char *	what() const throw() { return ("501"); }
		};

	public:

	/* Gets and Sets */
		std::string &	getData(void);

	/* Member Functions */
		bool	parseRequestLine(void) throw(NotImplemented, BadRequest);
		bool	parseHeaders(void) throw(BadRequest);
		bool	parseBody(void) throw(BadRequest);

		void	clear(void);

		void	display(void);

	private:
		size_t	_passSpaces(void);

		void	_passUntilChar(int c);
		void	_passUntilChar(int func(int));

		bool	_isLegitPath(std::string const & path);
		bool	_isinDic(char needle, char const * dic);

		bool	_passStrictOneChar(char const * dic);
		bool	_passStrictOneChar(int func(int));

		void	_passOneChar(char const * dic);
		void	_passOneChar(int func(int));

		void	_passOptionalChars(char const * dic);
		void	_passOptionalChars(int func(int));

		std::string	_getWord(char const * delimiter_dic);
		std::string	_getWord(int func(int));

		void	_parseChunkedBody(ssize_t & size) throw(BadRequest);
		bool	_checkEndOfChunkedEncoding(ssize_t & size);

	/* Member Attributes */
	public:
		/* Parsing Attributes */
		bool	headerComplete;

		/* Request Attributes */
		std::string		method;
		std::string		uri;
		std::string		protocol_v;
		Headers			headers;
		std::string		body;

	private:
		ServerDictionary	_dic;
		std::string 		_req;
		size_t				_pos;

	/* Static functions */
	public:
		static int			tounderscore(int c);
		static std::string	transform(std::string str, int func(int));
};

#endif
