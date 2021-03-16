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

/* A function which passes 1 char from the dictionary dic */
void	Request::_passOneChar(char const * dic)
{
	if (_isinDic(_req[_pos], dic))
		_pos++;
}

/* . . . and an overload with a conditional function (usage with isspace() for example) */
void	Request::_passOneChar(int func(int))
{
	if (_req[_pos] && !func(_req[_pos]))
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
	while (_req[_pos] && !func(_req[_pos]))
		_pos++;
}

/* A function gets the first encountered word */
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

	/* Pass 1 Space */
	_passOneChar(" ");

	/* Check Request-URI */
	// Request-URI = "*" | absoluteURI | abs_path | authority
	uri = _getWord();
	if (_isLegitPath(uri))
		COUT << "Found legit path\n";
	else
		COUT << "2 - wrong path|" << uri << "|\n";

	/* Pass 1 Space */
	_passOneChar(" ");

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

	/* Check is ':' is present */
	if (_req[_pos] != ':')
		COUT << "1 - char is not ':'|" << _req[_pos] << "|\n";
	else
		_pos++;

	/* Pass Optionnal White Spaces */
	_passOptionalChars(&isspace);

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

		

		/* Pass Optionnal White Spaces */
		_passOptionalChars(&isspace);

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
