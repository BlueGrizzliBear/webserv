#include "./Request.hpp"

/* Request Class Declaration */
/* Constructor */
/*	default		(1)	*/
Request::Request(void) : _req(""), _pos(0) {}

/*	copy		(2)	*/
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
	body = rhs.body;

	_req = rhs._req;
	_pos = rhs._pos;
	
	return (*this);
}

/* Gets and Sets */
std::string &	Request::getData(void)
{
	return (_req);
}

/* Member Functions */
void	Request::display(void)	/* Display request for debugging purposes */
{
	COUT << GREEN << "> CLIENT REQUEST\n";
	COUT << method << " " << uri << " " << protocol_v << ENDL;
	
	Headers::iterator begin = headers.begin();
	while (begin != headers.end())
	{
		COUT << begin->first << ": " << begin->second << ENDL;
		++begin;
	}
	// COUT << body << ENDL;
	COUT << RESET;
}

void	Request::clear(void)
{
	method.clear();
	uri.clear();
	protocol_v.clear();
	headers.clear();
	body.clear();
	
	_req.clear();
	_pos = 0;
}

/* A conditional function which returns a bool if the needle is in the dictionary dic */
bool	Request::_isinDic(char needle, char const * dic)
{
	int i = 0;
	
	while (dic[i] && dic[i] != needle)
		i++;
	if (dic[i] == '\0' && needle != '\0')
		return (false);
	return (true);
}

/* A function which passes until char is found */
void	Request::_passUntilChar(int c)
{
	while (_req[_pos] != c)
		_pos++;
}

/* . . . and an overload with a conditional function (usage with isspace() for example) */
void	Request::_passUntilChar(int func(int))
{
	while (!func(_req[_pos]))
		_pos++;
}

/* A function which strictly passes 1 char from the dictionary dic, and if not, throws */
bool	Request::_passStrictOneChar(char const * dic)
{
	if (_isinDic(_req[_pos], dic))
	{
		_pos++;
		return (true);
	}
	return (false);
	// 	_pos++;
	// else if (!_req[_pos])
	// 	return (true);
	// else
	// 	return (false);
	// return (true);
}

/* . . . and an overload with a conditional function (usage with isspace() for example) */
bool	Request::_passStrictOneChar(int func(int))
{
	if (func(_req[_pos]))
	{
		_pos++;
		return (true);
	}
	return (false);
	// else if (!_req[_pos])
	// 	return (true);
	// else
	// 	return (false);
	// return (true);
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
	if (func(_req[_pos]))
		_pos++;
}

/* A function which passes 1 or more Chars from the dictionary dic */
void	Request::_passOptionalChars(char const * dic)
{
	size_t ret = 0;
	if ((ret = _req.find_first_of(dic)) != std::string::npos)
		_pos += ret;
}

/* . . . and an overload with a conditional function (usage with isspace() for example) */
void	Request::_passOptionalChars(int func(int))
{
	while (func(_req[_pos]))
		_pos++;
}

/* A function which gets the first encountered word until the character delim is found */
std::string	Request::_getWord(char const * delimiter_dic)
{
	std::string word;

	while (!_isinDic(_req[_pos], delimiter_dic))
		word += _req[_pos++];
	return (word);
}

/* A function which gets the first encountered word until the function func is true */
std::string	Request::_getWord(int func(int))
{
	std::string word;

	while (!func(_req[_pos]))
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

bool	Request::parseRequestLine(void) throw(NotImplemented, BadRequest)
{
	/* Request-Line = Method SP Request-URI SP HTTP-Version CRLF */

	/* Check Method */
	method = _getWord(&isspace);
	// COUT << "here\n";
	if (_dic.methodDic.find(method) == _dic.methodDic.end())
		throw NotImplemented();		/* Or 405 (Method Not Allowed), if it doesnt have the rights */

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

	/* Check HTTP-Version */
	protocol_v = _getWord(&isspace);
	if (protocol_v != "HTTP/1.1")
		throw BadRequest();

	/* Check if end of the line (CRLF = \r\n) */
	if (!_passStrictOneChar("\r"))
		throw BadRequest();
	if (!_passStrictOneChar("\n"))
		throw BadRequest();

	return (true);
}

bool	Request::parseHeaders(void) throw(BadRequest)
{
	/* Request Header Fields */

	while (_req[_pos] && _req[_pos] != '\r')
	{
		/* Check Header Key */
		std::string header_key = _getWord("(),/:;<=>?@[\\]{}\" \t\r\f\n\v");
		if (header_key == "")
			break ;
		else if (_dic.headerDic.find(header_key) != _dic.headerDic.end() || (header_key.substr(0, 2).find("X-") != std::string::npos))
		{
			/* Header is implemented: get values and insert into Headers */
			// COUT << "Header IS implemented |" << header_key << "|" << ENDL;

			/* Check is ':' is present */
			if (!_passStrictOneChar(":"))
				throw BadRequest();

			_passOptionalChars(&isspace);

			/* Gather Header Values */
			std::string header_val = _getWord("\r");
			// CME << "Values|" << header_val << "|" << EME;
			headers.insert(std::make_pair(header_key, header_val));

		}
		else
		{
			/* Header is not implemented: pass until the end of line */
			_passUntilChar('\r');
		}
		_passOneChar("\r");
		_passOneChar("\n");
	}

	/* Check if new line */
	if (_req.find("\r\n", _pos) == std::string::npos || !(_pos += 2))
	{
		COUT << "ICI \n";
		throw BadRequest();
	}
	return (true);
}

bool	Request::_parseChunkedBody(size_t & size) throw(BadRequest)
{
	size_t pos = 0;
	if ((pos = _req.find("\r\n")) == std::string::npos)
		return (false);

	if (_req.find_first_not_of("0123456789ABCDEFabcdef", 0) != pos)
	{
		body.clear();
		_req.clear();
		throw BadRequest();
	}

	size = static_cast<unsigned long>(std::strtol((_req.substr(0, pos)).c_str(), NULL, 16));

	if (_req.size() - pos - 2 < size + 2)
		return (false);

	size_t body_pos = 0;
	if ((body_pos = _req.find("\r\n", size + pos + 2, 2)) == std::string::npos)
	{
		body.clear();
		_req.clear();
		throw BadRequest();
	}

	body.append(_req, pos + 2, size);

	_req.erase(0, pos + 2 + size + 2);
	
	return (true);
}

bool	Request::parseBody(void) throw(BadRequest)
{
	if (headers.find("Transfer-Encoding") != headers.end() && headers.find("Transfer-Encoding")->second == "chunked")
	{
		// COUT << "Transfert Encoding" << ENDL;
		static size_t size = 0;

		while (_parseChunkedBody(size))
		{
			if (size == 0)
				return (true);
		}
		return (false);
	}
	else if (headers.find("Content-Length") != headers.end())
	{
		size_t size = static_cast<size_t>(std::atoi(headers.find("Content-Length")->second.c_str()));
		
		if (size > _req.size() - _pos)
			return (false);
		while (size--)
			body += _req[_pos++];
		
		// COUT << "Content-Length: BODY IS COMPLETE" << ENDL;
		return (true);
	}
	else
	{
		// COUT << "NO body" << ENDL;
		return (true);	/* No body */
	}
	return (false);
}

/* Static Functions */
int		Request::tounderscore(int c)
{
	if (c == '-')
		return ('_');
	return (c);
}

std::string Request::transform(std::string str, int func(int))
{
	std::string::iterator begin = str.begin();
	std::string result;

	while (begin != str.end())
	{
		result.append(1, func(*begin));
		++begin;
	}
	return (result);
}
