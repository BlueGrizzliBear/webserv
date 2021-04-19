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
		typedef std::map<std::string, std::string, ci_less> Headers;

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

		class URITooLong : public std::exception {
			public:
				virtual const char *	what() const throw() { return ("414"); }
		};

		class NotImplemented : public std::exception {
			public:
				virtual const char *	what() const throw() { return ("501"); }
		};

	public:

	/* Gets and Sets */
		std::string &	getData(void);

	/* Member Functions */
		bool	parseRequestLine(void) throw(NotImplemented, BadRequest, URITooLong);
		bool	parseHeaders(void) throw(BadRequest);
		bool	parseBody(void) throw(BadRequest);

		void	clear(void);

		void	display(void);

		size_t	strFindCaseinsensitive(std::string str, char const * to_find);
		bool	str_is(std::string str, int func(int));

	private:
		void	_passUntilChar(char c);

		bool	_isLegitPath(std::string const & path);
		bool	_isinDic(char needle, const char * dic);

		bool	_passStrictOneChar(char c);

		void	_passOptionalChars(const char * dic);

		std::string	_getWord(const char * delimiter_dic);
		std::string	_getURI(const char * delimiter_dic) throw(URITooLong);

		bool	_parseChunkedBody(size_t & size) throw(BadRequest);

	/* Member Attributes */
	public:
		/* Parsing Attributes */
		bool		headerComplete;
		bool		headerParsed;

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
