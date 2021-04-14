#include "./Request.hpp"

/* Request Class Declaration */
/* Constructor */
/*	default		(1)	*/
Request::Request(void) : headerComplete(0), headerParsed(0), client(nullptr), _req(""), _pos(0) {}

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
	headerComplete = rhs.headerComplete;
	headerParsed = rhs.headerParsed;

	client = rhs.client;

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
	headerComplete = false;		/* Reseting bool indicator if header is complete or not */
	headerParsed = false;		/* Reseting bool indicator if header is parsed or not */

	method.clear();
	uri.clear();
	protocol_v.clear();
	headers.clear();
	body.clear();
	body.reserve();

	_req.clear();
	_pos = 0;
}

size_t	Request::strFindCaseinsensitive(std::string str, char const * to_find)
{
	std::string	tmp = str;
	std::string	tmp_to_find(to_find);

	for (std::string::iterator it = tmp.begin(); it != tmp.end(); ++it)
		*it = static_cast<char>(tolower(*it));
	for (std::string::iterator it = tmp_to_find.begin(); it != tmp_to_find.end(); ++it)
		*it = static_cast<char>(tolower(*it));
	return (tmp.find(tmp_to_find));
}

/* A function which passes until char is found */
void	Request::_passUntilChar(char c)
{
	size_t i = 0;

	if ((i = _req.find_first_of(c, _pos)) != std::string::npos)
	{
		// COUT << "found r\n";
		_pos = i;
	}
	else
	{
		// COUT << "didnt find r\n";
		_pos = _req.size();
		return;
	}
	return ;
}

/* A function which strictly passes 1 char from the dictionary dic, and if not, throws */
bool	Request::_passStrictOneChar(char c)
{
	if (_req.find_first_of(c, _pos) == _pos)
	{
		_pos++;
		return (true);
	}
	return (false);
}

/* A function which passes 1 or more Chars from the dictionary dic */
void	Request::_passOptionalChars(const char * dic)
{
	size_t i = 0;

	if ((i = _req.find_first_of(dic, _pos)) == _pos)
		_pos++;
}

/* A function which gets the first encountered word until the function func is true */
std::string	Request::_getWord(const char * delimiter_dic)
{
	std::string word;
	size_t pos;

	if ((pos = _req.find_first_of(delimiter_dic, _pos)) != std::string::npos)
	{
		word = _req.substr(_pos, pos - _pos);
		_pos = pos;
	}
	else
	{
		_pos = _req.size();
		return ("");
	}
	return (word);
}

bool	Request::_isLegitPath(std::string const & path)
{
	/* Request-URI = "*" | absoluteURI | abs_path | authority */

	if (path.empty())
		return (false);
	return (true);
}

bool	Request::parseRequestLine(void) throw(NotImplemented, BadRequest)
{
	/* Request-Line = Method SP Request-URI SP HTTP-Version CRLF */

	/* Check Method */
	method = _getWord("\t ");
	if (_dic.methodDic.find(method) == _dic.methodDic.end())
	{
		COUT << "METHOD NON IMPLEMENTEE BORDEL |" << method << "|\n";
		throw NotImplemented();		/* Or 405 (Method Not Allowed), if it doesnt have the rights */
	}

	/* Pass 1 Space */
	if (!_passStrictOneChar(' '))
	{
		// COUT << "1\n";
		throw BadRequest();
	}

	/* Check Request-URI */
	uri = _getWord("\t ");
	if (!_isLegitPath(uri))
	{
		// COUT << "1\n";
		throw BadRequest();
	}

	/* Pass 1 Space */
	if (!_passStrictOneChar(' '))
	{
		// COUT << "1\n";
		throw BadRequest();
	}

	/* Check HTTP-Version */
	protocol_v = _getWord("\r");
	if (protocol_v != "HTTP/1.1")
	{
		// COUT << "1\n";
		throw BadRequest();
	}


	/* Check if end of the line (CRLF = \r\n) */
	if (!_passStrictOneChar('\r'))
	{
		// COUT << "1\n";
		throw BadRequest();
	}
	if (!_passStrictOneChar('\n'))
	{
		// COUT << "1\n";
		throw BadRequest();
	}

	return (true);
}

bool	Request::parseHeaders(void) throw(BadRequest)
{
	/* Request Header Fields */

	while (1)
	{
		// COUT << "BEGIN &_req[_pos]|" << &_req[_pos] << "|\n";

		// std::string header_key = _getWord("(),/:;<=>?@[\\]{}\" \t\r\f\n\v");	/* Check Header Key */
		std::string header_key = _getWord(":\r");	/* Check Header Key */

		if (header_key == "")
			break ;
		else if (_dic.headerDic.find(header_key) != _dic.headerDic.end() || (header_key.find("X-", 0) == 0))
		{
			if (!_passStrictOneChar(':'))	/* Check is ':' is present */
			{
				// COUT << "missing:\n";
				throw BadRequest();
			}

			_passOptionalChars("\t ");

			// COUT << MAGENTA << "New Header\n" << RESET;

			// COUT << "&_req[_pos]|" << &_req[_pos] << "|\n";

			// std::string header_val = _getWord("\t \r");	/* Gather Header Values */
			std::string header_val = _getWord("\r");	/* Gather Header Values */

			// COUT << "&_req[_pos]|" << &_req[_pos] << "|\n";
			headers.insert(std::make_pair(header_key, header_val));

			// COUT << "&_req[_pos]|" << &_req[_pos] << "|\n";
			// _passOptionalChars("\t ");
			// COUT << "&_req[_pos]|" << &_req[_pos] << "|\n";
		}
		else
		{
			_passUntilChar('\r');	/* Header is not implemented: pass until the end of line */
		}


		if (!_passStrictOneChar('\r'))
		{
			// COUT << "ici3\n";
			throw BadRequest();
		}
		if (!_passStrictOneChar('\n'))
		{
			// COUT << "ici4\n";
			throw BadRequest();
		}
	}

	/* Check if new line */
	if (!_passStrictOneChar('\r'))
	{
		// COUT << "ici5\n";
		throw BadRequest();
	}
	if (!_passStrictOneChar('\n'))
	{
		// COUT << "ici6\n";
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
	if (headers.find("Transfer-Encoding") != headers.end())
	{
		// CERR << "Transfert Encoding" << ENDL;

		if (headers.find("Transfer-Encoding")->second == "chunked")
		{
			// CERR << "> chunked" << ENDL;
			static size_t size = 0;

			while (_parseChunkedBody(size))
			{
				if (size == 0)
				{
					return (true);
				}
			}
			return (false);
		}
	}
	else if (headers.find("Content-Length") != headers.end())
	{
		CERR << "Content-length" << ENDL;
		size_t size = static_cast<size_t>(std::atoi(headers.find("Content-Length")->second.c_str()));

		if (size > _req.size() - _pos)
			return (false);
		while (size--)
			body += _req[_pos++];

		// COUT << "Content-Length: BODY IS COMPLETE" << ENDL;
		return (true);
	}
	// CERR << "No Body" << ENDL;
	return (true);	/* No body */
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
