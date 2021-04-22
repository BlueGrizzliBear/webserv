#include "./Request.hpp"

/* Request Class Declaration */
/* Constructor */
/*	default		(1)	*/
Request::Request(void) : headerComplete(0), headerParsed(0), method(""), uri(""), protocol_v(""), body(""), _req(""), _pos(0) {}

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

size_t &	Request::getPos(void)
{
	return (_pos);
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

int		Request::isValidHost(int c)
{
	std::string dic("/<>@\\{}^`|#\"");

	if (dic.find(static_cast<char>(c)) == std::string::npos)
		return (1);
	return (0);
}

bool	Request::str_is(std::string str, int func(int))
{
	std::string::iterator begin = str.begin();

	while (begin != str.end())
	{
		if (!func(*begin))
			return (false);
		++begin;
	}
	return (true);
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
	if (_req.find(c, _pos) == _pos)
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

	// if ((pos = _req.find_first_of(delimiter_dic, _pos)) != std::string::npos)
	if ((pos = _req.find(delimiter_dic, _pos)) != std::string::npos)
	{
		word = _req.substr(_pos, pos - _pos);
		_pos = pos;
	}
	else
	{
		// _pos = _req.size() - 1;
		return (std::string());
	}
	return (word);
}

std::string	Request::_getURI(const char * delimiter_dic) throw(URITooLong)
{
	std::string word;
	size_t pos;

	if ((pos = _req.find_first_of(delimiter_dic, _pos)) != std::string::npos)
	{
		if ((pos - _pos) > PATH_MAX)
			throw URITooLong();
		word = _req.substr(_pos, pos - _pos);
		_pos = pos;
	}
	else
	{
		// _pos = _req.size() - 1;
		return (std::string());
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

bool	Request::parseRequestLine(void) throw(NotImplemented, BadRequest, URITooLong)
{
	/* Request-Line = Method SP Request-URI SP HTTP-Version CRLF */

	/* Check Method */
	method = _getWord(" ");
	if (_dic.methodDic.find(method) == _dic.methodDic.end())
	{
		COUT << "_req|" << _req << "|\n";
		COUT << "METHOD NON IMPLEMENTEE BORDEL |" << method << "|\n";
		throw NotImplemented();		/* Or 405 (Method Not Allowed), if it doesnt have the rights */
	}

	/* Pass 1 Space */
	if (!_passStrictOneChar(' '))
	{
		COUT << "1\n";
		throw BadRequest();
	}

	/* Check Request-URI */
	// uri = _getWord(" ");
	uri = _getURI(" ");
	if (!_isLegitPath(uri))
	{
		COUT << "1\n";
		throw BadRequest();
	}

	/* Pass 1 Space */
	if (!_passStrictOneChar(' '))
	{
		COUT << "1\n";
		throw BadRequest();
	}

	/* Check HTTP-Version */
	protocol_v = _getWord("\r");
	if (protocol_v != "HTTP/1.1")
	{
		COUT << "1\n";
		throw BadRequest();
	}


	/* Check if end of the line (CRLF = \r\n) */
	if (!_passStrictOneChar('\r'))
	{
		COUT << "1\n";
		throw BadRequest();
	}
	if (!_passStrictOneChar('\n'))
	{
		COUT << "1\n";
		throw BadRequest();
	}

	return (true);
}

bool	Request::parseHeaders(void) throw(BadRequest)
{
	/* Request Header Fields */

	while (1)
	{
		if (_req.find("\r\n", _pos) == _pos)
			break ;
		std::string header_key = _getWord(":");	/* Check Header Key */

		if (header_key == "" && (header_key = _getWord("\r")) == "")	/* Check Header Key */
			break ;
		if (header_key.find_first_of(" \t") != std::string::npos)	// is token A FAIRE
		{
			CERR << "whitespace between header-name and colon\n";
			throw BadRequest();
		}
		else
		{
			if (!_passStrictOneChar(':'))	/* Check is ':' is present */
			{
				CERR << "missing :\n";
				throw BadRequest();
			}

			_passOptionalChars("\t ");
			std::string header_val = _getWord("\r\n");

			if (header_val.size())
			{
				if ((header_val.rfind(" ") == header_val.size() - 1) || (header_val.rfind("\t") == header_val.size() - 1))
					header_val.erase(header_val.size() - 1, 1);
			}

			COUT << "HEADER|" << header_key << ":" << header_val << "|\n";
			if (!headers.insert(std::make_pair(header_key, header_val)).second)
			{
				CERR << "Header already exists :\n";
				throw BadRequest();
			}
		}
		if (!_passStrictOneChar('\r'))
		{
			COUT << "ici3\n";
			throw BadRequest();
		}
		if (!_passStrictOneChar('\n'))
		{
			COUT << "ici4\n";
			throw BadRequest();
		}
	}

	/* Check if new line */
	if (!_passStrictOneChar('\r'))
	{
		COUT << "ici5\n";
		throw BadRequest();
	}
	if (!_passStrictOneChar('\n'))
	{
		COUT << "ici6\n";
		throw BadRequest();
	}
	return (true);
}

bool	Request::_isQuotedString(std::string str)
{
	if (str.find("\"") == 0 && str.rfind("\"") == str.size() - 1 && str.size() > 1)
	{
		str.erase(0, 1);
		str.erase(str.size() - 1, 1);

		std::string::iterator it = str.begin();
		std::string::iterator ite = str.end();
		
		while (it != ite)
		{
			if (isprint(*it))
			{
				if (*it == '\\' && it + 1 == ite)
					return (false);
			}
			else if (*it != '\t')
				return (false);
			it++;
		}
	}
	return (true);
}

bool	Request::_isToken(std::string str)
{
	// TOKEN IS NOT => DQUOTE and "(),/:;<=>?@[\]{}"
	if (str.find_first_of("(),/:;<=>?@[\\]{}\"") == std::string::npos)
		return (true);
	return (false);
}

bool	Request::_chunkedExtensionInvalid(std::string str)
{
	size_t pos;

	while (!str.empty())
	{
		if (str.find(";") != 0)
			return (true);
		str.erase(0, 1);	/* Erase ";" */
		pos = str.find_first_of("=;");
		if (_isToken(str.substr(0, pos)))
		{
			str.erase(0, pos);	/* Erase until next "=" or ";" */
			if (str.find("=") == 0)
			{
				str.erase(0, 1);	/* Erase "=" */
				pos = str.find_first_of(";");
				if (_isToken(str.substr(0, pos)) || _isQuotedString(str.substr(0, pos)))
					str.erase(0, pos);	/* Erase until next ";" */
				else
					return (true);
			}
		}
		else
			return (true);
	}
	return (false);
}

bool	Request::_parseChunkedBody(size_t & size) throw(BadRequest)
{
	size_t pos = 0;
	size_t nb = 0;
	static bool foundEnd = false;

	// COUT << "_req|" << _req << "|\n";

	if ((pos = _req.find("\r\n")) == std::string::npos)
		return (false);

	if ((nb = _req.find_first_not_of("0123456789ABCDEFabcdef", 0)) != 0)
	{
		if (!(size = static_cast<unsigned long>(std::strtol((_req.substr(0, pos)).c_str(), NULL, 16))))
			foundEnd = true;

		// if (nb != pos && _chunkedExtensionInvalid(_req.substr(_req.find(";", 0, pos), pos - _req.find(";", 0, pos))))
		if (nb != pos && _chunkedExtensionInvalid(_req.substr(nb, pos)))
		{
			body.clear();
			_req.clear();
			throw BadRequest();
		}

		if (_req.size() - pos - 2 < size + 2)
		{
			// COUT << "Not enough size\n";
			return (false);
		}

		if (foundEnd)
		{
			if (_req.find("\r\n") != pos + 2)
			{
				// COUT << "_req|" << _req << "|\n";
				if (_req.find("\r\n\r\n") == std::string::npos)
					return (false);

				_req.erase(0, pos + 2);
				// COUT << "ici\n";
				// COUT << "_pos|" << _pos << "|\n";
				parseHeaders();
			}
			_req.clear();
			foundEnd = false;
			// COUT << "FINISHED CHUNKED\n";
			return (true);
		}
		else if (_req.find("\r\n", size + pos + 2) == std::string::npos)
		{
			body.clear();
			_req.clear();
			throw BadRequest();
		}
		body.append(_req, pos + 2, size);

		_req.erase(0, pos + 2 + size + 2);

		// COUT << "Returning here, _req|" << _req << "|\n";
		return (true);
	}
	else
	{
		body.clear();
		_req.clear();
		throw BadRequest();
	}
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
					return (true);
			}
			// COUT << "AFTER CHUNEKD _req|" << _req << "|\n";
			return (false);
		}
		else
			throw BadRequest();
	}
	else if (headers.find("Content-Length") != headers.end())
	{
		// CERR << "Content-length" << ENDL;
		if (!str_is(headers.find("Content-Length")->second, std::isdigit))
			throw BadRequest();
		size_t size = static_cast<size_t>(std::strtol(headers.find("Content-Length")->second.c_str(), NULL, 10));
		if (errno == ERANGE)
		{
			errno = 0;
			throw BadRequest();
		}
		if (size > _req.size() - _pos)
			return (false);
		body.append(_req, _pos, size - _pos);
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
