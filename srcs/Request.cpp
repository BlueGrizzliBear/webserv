#include "./Request.hpp"

/* Request Class Declaration */
/* Constructor */
/*	default		(1)	*/
Request::Request(void) {}

/*	argument	(2)	*/
Request::Request(ServerBloc & server, const char * request) : _req(request), _pos(0), _parent(&server)
{
	_parseRequestLine();
	_parseHeaders();
	// _parseBody();

	/* Check if cariage return */
	_passStrictOneChar("\r");
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
	_parent = rhs._parent;
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

/* A function which strictly passes 1 char from the dictionary dic, and if not, throws */
void	Request::_passStrictOneChar(char const * dic)
{
	if (_isinDic(_req[_pos], dic))
		_pos++;
	else
		COUT << "ERROR: Character not found\n";
}

/* . . . and an overload with a conditional function (usage with isspace() for example) */
void	Request::_passStrictOneChar(int func(int))
{
	if (_req[_pos] && !func(_req[_pos]))
		_pos++;
	else
		COUT << "ERROR: Character not found\n";
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

int	Request::_parseRequestLine(void)
{
	/* Request-Line = Method SP Request-URI SP HTTP-Version CRLF */

	/* Check Method */
	method = _getWord(&isspace);
	if (_dic.methodDic.find(method) == _dic.methodDic.end())
		throw NotImplemented();
	// else
	// 	COUT << "Found legit method\n";

	/* Pass 1 Space */
	_passStrictOneChar(" ");

	/* Check Request-URI */
	uri = _getWord(&isspace);
	if (!_isLegitPath(uri))
		COUT << "2 - wrong path|" << uri << "|\n";

	/* Pass 1 Space */
	_passStrictOneChar(" ");

	/* Check HTTP-Version */
	protocol_v = _getWord(&isspace);
	if (protocol_v != "HTTP/1.1")
		COUT << "3 - wrong protocol|" << protocol_v << "|\n";

	/* Check if end of the line (CRLF = \r\n) */
	_passStrictOneChar("\r");
	_passStrictOneChar("\n");
	
	return (0);
}

int	Request::_parseHeaders(void)
{
	/* Request Header Fields */

	/* Check Header Key */
	std::string header_key = _getWord("(),/:;<=>?@[\\]{}\" \t\r\f\n\v");
	if (_dic.headerDic.find(header_key) == _dic.headerDic.end())
		COUT << "1 - wrong Header Field|" << header_key << "|\n";
	else
		COUT << "Header-key valid\n";

	/* Check is ':' is present */
	_passStrictOneChar(":");

	/* Pass Optionnal White Spaces */
	_passOptionalChars(&isspace);

	/* Check Header Values */
	std::vector<std::string> header_values;
	while (_req[_pos] && _req[_pos] != '\n')
	{
		std::string header_val = _getWord("(),/:;<=>?@[\\]{}\" \t\r\f\n\v");
		CME << "req|" << _req[_pos] << "|" << EME;
		CME << "word|" << header_val << "|" << EME;
		if (header_val != "")
			COUT << "Found legit header\n";
		else
		{
			COUT << "ERROR: values needs to be one character long at least\n";
			_pos++;
		}
		header_values.push_back(header_val);

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
