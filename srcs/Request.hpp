#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "./webserv.hpp"
#include "./ServerDictionary.hpp"

/* Request Class Declaration */
class Request
{
	public:
	/* Member Types */
		typedef std::map<std::string, std::vector<std::string> > Headers;

	/* Constructor */
		/*	default		(1)	*/	Request(void);
		/*	argument	(2)	*/	Request(const char * request);
		/*	copy		(3)	*/	Request(Request const & cpy);

	/* Destructor */
		~Request();

	/* Operators */
		Request &	operator=(Request const & rhs);
	
	/* Member Functions */
	public:

	private:
		std::string	_getWord(void);
		size_t		_passSpaces(void);

		bool	_isLegitPath(std::string const & path);
		bool	_isinDic(char needle, char const * dic);
		void	_passOneChar(char const * dic);
		void	_passOneChar(int func(int));
		void	_passOptionalChars(char const * dic);
		void	_passOptionalChars(int func(int));

		int		_parseRequestLine(void);
		int		_parseHeaders(void);
		int		_parseBody(void);

	/* Member Attributes */
	public:
		std::string		method;
		std::string		uri;
		std::string		protocol_v;
		Headers			headers;

	private:
		ServerDictionary	_dic;
		std::string const 	_req;
		size_t				_pos;


};

#endif