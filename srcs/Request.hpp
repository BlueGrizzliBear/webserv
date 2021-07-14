#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "./webserv.hpp"
# include "./ServerDictionary.hpp"

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
		size_t &		getPos(void);

	/* Member Functions */
		bool	parseRequestLine(void) throw(NotImplemented, BadRequest, URITooLong);
		bool	parseHeaders(void) throw(BadRequest);
		bool	parseBody(void) throw(NotImplemented, BadRequest);

		void	clear(void);

		/* void	display(void); */

		size_t	strFindCaseinsensitive(std::string str, char const * to_find);
		bool	str_is(std::string str, int func(int));

	private:
		bool	_isLegitPath(std::string const & path);
		bool	_isQuotedString(std::string str);
		bool	_isToken(std::string str);

		void	_passUntilChar(char c);
		bool	_passStrictOneChar(char c);
		void	_passOptionalChars(const char * dic);

		std::string	_getWord(const char * delimiter_dic);
		std::string	_getURI(const char * delimiter_dic) throw(URITooLong);

		bool	_chunkedExtensionInvalid(std::string str);
		bool	_parseChunkedBody(void) throw(BadRequest);
		bool	_checkTransferEncoding(std::string & second);


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
		bool				_foundEnd;

	/* Static functions */
	public:
		static int			tounderscore(int c);
		static std::string	transform(std::string str, int func(int));
		static int			isValidHost(int c);
		static int			ft_isprint(int c);
		static int			ft_isdigit(int c);
		static size_t		ft_strlen(const char * s);
		static size_t		ft_strcmp(const char *s1, const char *s2);
		static char *		ft_strdup(const char *s1);
		static void *		ft_memcpy(void *dst, const void *src, size_t n);
		static int			ft_atoi(const char *str);
		static float		ft_atof(const char *arr);
		static double		ft_pow(int a, int b);
		static long long	ft_power(long long nbr, long long power);
		static long long	ft_posbase(char c, const char * base);
		static long			ft_atol_base(const char * str, const char * base);
		static long			ft_strtol_base(std::string str, const char * base);
		static std::string	ft_inet_ntoa(in_addr addr);
		static int			ft_toupper(int c);
};

#endif
