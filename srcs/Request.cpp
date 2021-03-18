#include "./Request.hpp"

/* Request Class Declaration */
/* Constructor */
/*	default		(1)	*/
Request::Request(void) {}

/*	argument	(2)	*/
Request::Request(std::stringstream & ss) : _req(ss.str()), _pos(0)
{
	_parseRequestLine();
	CME << "REQUEST LINE OK" << EME;
	_parseHeaders();
	CME << "HEADERS OK" << EME;
	_parseBody();
	CME << "BODY DONE" << EME;
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
	finished = rhs.finished;

	method = rhs.method;
	uri = rhs.uri;
	protocol_v = rhs.protocol_v;
	headers = rhs.headers;
	body = rhs.body;

	_req = rhs._req;
	_pos = rhs._pos;
	
	return (*this);
}

/* Member Functions */
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
	else if (!_req[_pos])
		return (true);
	else
		return (false);
	return (true);
}

/* . . . and an overload with a conditional function (usage with isspace() for example) */
bool	Request::_passStrictOneChar(int func(int))
{
	if (_req[_pos] && func(_req[_pos]))
		_pos++;
	else if (!_req[_pos])
		return (true);
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

int	Request::_parseRequestLine(void) throw(NotImplemented, BadRequest)
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

int	Request::_parseHeaders(void) throw(BadRequest)
{
	/* Request Header Fields */

	while (_req[_pos] && _req[_pos] != '\r')
	{
		/* Check Header Key */
		std::string header_key = _getWord("(),/:;<=>?@[\\]{}\" \t\r\f\n\v");
		if (header_key == "")
			break ;
		else if (_dic.headerDic.find(header_key) == _dic.headerDic.end())
		{
			/* Header is not implemented: pass until the end of line */
			_passUntilChar('\r');
		}
		else
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
		_passOneChar("\r");
		_passOneChar("\n");
	}
	return (0);
}

int	Request::_parseChunkedBody(ssize_t & size) throw(BadRequest)
{
	std::string tmp;

	while (_req[_pos] && std::isxdigit(_req[_pos]))
	{
		tmp += _req[_pos];
		body += _req[_pos++];
	}
	_passStrictOneChar("\r");
	_passStrictOneChar("\n");
	size = std::atoi(tmp.c_str());
	while (_req[_pos] && size-- > 0)
		body += _req[_pos++];
	return (0);
}

bool	Request::_checkEndOfChunkedEncoding(ssize_t & size)
{
	if (_req[_pos] == '0' && _req[_pos + 1] == '\r' && _req[_pos + 2] == '\n' && _req[_pos + 3] == '\r' && _req[_pos + 4] == '\n')
	{
		if (size == 0)
		{
			body += _req[_pos] + _req[_pos + 1] + _req[_pos + 2] + _req[_pos + 3] + _req[_pos + 4];;
			_pos += 5;
			return (true);
		}
	}
	return (false);
}

int	Request::_parseBody(void) throw(BadRequest)
{
	/* Check if new line */
	if (!(_passStrictOneChar("\r") && _passStrictOneChar("\n")))
	{
		/* in the mean time */
		throw BadRequest();
	}

	/* Parse Body */
	if (headers.find("Content-Length") != headers.end())
	{
		int size = std::atoi(headers.find("Content-Length")->second.c_str());
		while (_req[_pos] && size--)
			_pos++;
		finished = true;
		COUT << "Content-Length: BODY IS COMPLETE" << ENDL;
	}
	/* Can only be "Transfert-Encoding: chunked" */
	else if (headers.find("Transfer-Encoding") != headers.end())
	{
		static ssize_t size = 0;

		while (_req[_pos])
		{
			if ((finished = _checkEndOfChunkedEncoding(size)))
			{
				COUT << "Transfert-Encoding: complete" << ENDL;
				return (0);
			}
			else
			{
				_parseChunkedBody(size);
				COUT << "Transfert-Encoding: INCOMPLETE BODY" << ENDL;
			}
			// _pos++;
		}
		COUT << "Transfert-Encoding: INCOMPLETE BODY" << ENDL;
	}
	else
	{
		/* No body */
		finished = true;
		COUT << "NO BODY" << ENDL;
	}
	return (0);
}
