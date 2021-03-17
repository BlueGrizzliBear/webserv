#include "./Request.hpp"

/* Request Class Declaration */
/* Constructor */
/*	default		(1)	*/
Request::Request(void) {}

/*	argument	(2)	*/
Request::Request(const char * request) : _req(request), _pos(0)
{
	_parseRequestLine();
	CME << "HEADER OK" << EME;
	_parseHeaders();
	// _parseBody();

	/* Check if cariage return */
	if (_passStrictOneChar("\r"))
		finished = true;

}

/*	copy		(3)	*/
Request::Request(Request const & cpy)
{
	*this = cpy;
}

/* Destructor */
Request::~Request() {}

/* Operators */
Request &	Request::operator=(Request const & rhs)
{
	method = rhs.method;
	uri = rhs.uri;
	protocol_v = rhs.protocol_v;
	headers = rhs.headers;
	_req = rhs._req;
	_pos = rhs._pos;
	// _parent = rhs._parent;
	return (*this);
}

/* Member Functions */
/* A conditional function which returns a bool if the needle is in the dictionary dic */
bool	Request::_isinDic(char needle, char const * dic)
{
	int i = 0;
	
	while (dic[i] && dic[i] != needle)
		i++;
	if (dic[i] == '\0')
		return (false);
	return (true);
}

/* A function which passes until char is found */
void	Request::_passUntilChar(int c)
{
	while (_req[_pos] && _req[_pos] != c)
		_pos++;
}

/* . . . and an overload with a conditional function (usage with isspace() for example) */
void	Request::_passUntilChar(int func(int))
{
	while (_req[_pos] && !func(_req[_pos]))
		_pos++;
}

/* A function which strictly passes 1 char from the dictionary dic, and if not, throws */
bool	Request::_passStrictOneChar(char const * dic)
{
	if (_isinDic(_req[_pos], dic))
		_pos++;
	else
		return (false);
	return (true);
}

/* . . . and an overload with a conditional function (usage with isspace() for example) */
bool	Request::_passStrictOneChar(int func(int))
{
	if (_req[_pos] && func(_req[_pos]))
		_pos++;
	else
		return (false);
	return (true);
}

/* A function which passes 1 char from the dictionary dic */
void	Request::_passOneChar(char const * dic)
{
	if (_isinDic(_req[_pos], dic))
		_pos++;
}

/* . . . and an overload with a conditional function (usage with isspace() for example) */
void	Request::_passOneChar(int func(int))
{
	if (_req[_pos] && func(_req[_pos]))
		_pos++;
}

/* A function which passes 1 or more Chars from the dictionary dic */
void	Request::_passOptionalChars(char const * dic)
{
	while (_isinDic(_req[_pos], dic))
		_pos++;
}

/* . . . and an overload with a conditional function (usage with isspace() for example) */
void	Request::_passOptionalChars(int func(int))
{
	while (_req[_pos] && func(_req[_pos]))
		_pos++;
}

/* A function which gets the first encountered word until the character delim is found */
std::string	Request::_getWord(char const * delimiter_dic)
{
	std::string word;

	while (_req[_pos] && !_isinDic(_req[_pos], delimiter_dic))
		word += _req[_pos++];
	return (word);
}

/* A function which gets the first encountered word until the function func is true */
std::string	Request::_getWord(int func(int))
{
	std::string word;

	while (_req[_pos] && !func(_req[_pos]))
		word += _req[_pos++];
	return (word);
}

bool	Request::_isLegitPath(std::string const & path)
{
	/* Request-URI = "*" | absoluteURI | abs_path | authority */

	if (path.empty())
		return (false);
	// Option to check maybe ?
	// if (path == "*" /* && method == "OPTION"*/)
		// return (false);
	return (true);
}

int	Request::_parseRequestLine(void) throw(NotImplemented)
{
	/* Request-Line = Method SP Request-URI SP HTTP-Version CRLF */

	/* Check Method */
	method = _getWord(&isspace);
	if (_dic.methodDic.find(method) == _dic.methodDic.end())
		throw NotImplemented(); 								/* Or 405 (Method Not Allowed), if it doesnt have the rights */

	/* Pass 1 Space */
	if (!_passStrictOneChar(" "))
		throw BadRequest();

	/* Check Request-URI */
	uri = _getWord(&isspace);
	if (!_isLegitPath(uri))
		throw BadRequest();

	/* Pass 1 Space */
	if (!_passStrictOneChar(" "))
		throw BadRequest();

// 400 (Bad Request) error or a 301 (Moved Permanently) redirect with
//    the request-target properly encoded. 

	/* Check HTTP-Version */
	protocol_v = _getWord(&isspace);
	if (protocol_v != "HTTP/1.1")
		throw BadRequest();

	/* Check if end of the line (CRLF = \r\n) */
	if (!_passStrictOneChar("\r"))
		throw BadRequest();
	if (!_passStrictOneChar("\n"))
		throw BadRequest();
	
	return (0);
}

int	Request::_parseHeaders(void)
{
	/* Request Header Fields */

	while (_req[_pos] && _req[_pos] != '\n')
	{
		/* Check Header Key */
		std::string header_key = _getWord("(),/:;<=>?@[\\]{}\" \t\r\f\n\v");
		if (!(_dic.headerDic.find(header_key) == _dic.headerDic.end()))
		{
			COUT << "Header key not implemented |" << header_key << "|" << ENDL;
			_passUntilChar("\r");
		}
		else
		{
			/* Check is ':' is present */
			_passStrictOneChar(":");

			/* Pass Optionnal White Spaces */
			_passOptionalChars(&isspace);

			/* Gather Header Values */
			std::string header_val = _getWord("\r");
			CME << "word|" << header_val << "|" << EME;
			if (header_val == "")
			{
				COUT << "ERROR: values needs to be one character long at least\n";
				_pos++;
			}
			else
			{
				headers.insert(std::make_pair(header_key, header_val));
				_pos++;
			}
		}
	}

	return (0);
}

int	Request::_parseBody(void)
{
	while (_req[_pos] && _req[_pos] != '\n')
	{
		COUT << "_req[_pos]|" << _req[_pos] << "|\n";
		_pos++;
	}

	return (0);
}
