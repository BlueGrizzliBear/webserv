#include "./Request.hpp"

/* Request Class Declaration */
/* Constructor */
/*	default		(1)	*/
Request::Request(void) {}

/*	argument	(2)	*/
Request::Request(const char * request) : _req(request), _pos(0)
{
	// construction with request buffer here


	_parseRequestLine();
	// _parseHeaders();
	// _parseBody();

	/* Check if cariage return */
	if (_req[_pos] == '\r')
		COUT << "found cariage return\n";
	else
		COUT << "A-1 - no cariage return |" << _req[_pos] << "|\n";

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
	return (*this);
}

/* Member Functions */
size_t	Request::_passSpaces(void)
{
	while (_req[_pos] == ' ' || _req[_pos] == '\t')
		_pos++;
	return (_pos);
}

std::string	Request::_getWord(void)
{
	std::string word;

	while (_req[_pos] && !isspace(_req[_pos]))
		word += _req[_pos++];
	return (word);
}

// to implement
bool	Request::_isLegitPath(std::string const & path)
{
	if (path == "/")
		return (true);
	if (path != "*")
		return (false);
	return (true);
}

int	Request::_parseRequestLine(void)
{
	/* Request-Line = Method SP Request-URI SP HTTP-Version CRLF */

	/* Check Method */
	method = _getWord();
	if (_dic.methodDic.find(method) != _dic.methodDic.end())
		COUT << "Found legit method\n";
	else
		COUT << "1 - wrong method|" << method << "|\n";

	_passSpaces();

	/* Check Request-URI */
	// Request-URI = "*" | absoluteURI | abs_path | authority
	uri = _getWord();
	if (_isLegitPath(uri))
		COUT << "Found legit path\n";
	else
		COUT << "2 - wrong path|" << uri << "|\n";

	_passSpaces();

	/* Check HTTP-Version */
	protocol_v = _getWord();
	if (protocol_v == "HTTP/1.1")
		COUT << "Found legit protocol\n";
	else
		COUT << "3 - wrong protocol|" << protocol_v << "|\n";

	/* Check if end of the line */
	if (_req[_pos] == '\n')
		COUT << "found the end of the line\n";
	else
		COUT << "A-1 - no \\n at the end |" << _req[_pos] << "|\n";

	return (0);
}

int	Request::_parseHeaders(void)
{
	/* Request Header Fields */

	/* Check Header Key */
	std::string header_key = _getWord();
	if (_dic.headerDic.find(header_key) != _dic.headerDic.end())
		COUT << "Found legit header_key\n";
	else
		COUT << "1 - wrong Header Field|" << header_key << "|\n";

	if (_req[_pos] != ':')
		COUT << "1 - char is not ':'|" << _req[_pos] << "|\n";
	else
		_pos++;

	_passSpaces();

	while (_req[_pos] && _req[_pos] != '\n')
	{
		/* Check Header Field */
		std::string header = _getWord();
		if (_dic.headerDic.find(header) != _dic.headerDic.end())
			COUT << "Found legit header\n";
		// else if (header == "")
		// 	_pos++;
		else
			COUT << "1 - wrong Header Field|" << header << "|\n";

		

		_passSpaces();

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
