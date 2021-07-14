#include "./Request.hpp"

/* Request Class Declaration */
/* Constructor */
/*	default		(1)	*/
Request::Request(void) : headerComplete(0), headerParsed(0), method(""), uri(""), protocol_v(""), body(""), _req(""), _pos(0), _foundEnd(false) {}

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
	_foundEnd = rhs._foundEnd;
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
/* Display request for debugging purposes
void	Request::display(void)
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
*/

void	Request::clear(void)
{
	headerComplete = false;
	headerParsed = false;

	method.clear();
	uri.clear();
	protocol_v.clear();
	headers.clear();
	body.clear();
	body.reserve();
	_req.clear();
	_pos = 0;
	_foundEnd = false;
}

size_t	Request::strFindCaseinsensitive(std::string str, char const * to_find)
{
	std::string	tmp = str;
	std::string	tmp_to_find(to_find);

	for (std::string::iterator it = tmp.begin(); it != tmp.end(); ++it)
		*it = static_cast<char>(ft_tolower(*it));
	for (std::string::iterator it = tmp_to_find.begin(); it != tmp_to_find.end(); ++it)
		*it = static_cast<char>(ft_tolower(*it));
	return (tmp.find(tmp_to_find));
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
		_pos = i;
	else
	{
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
	std::string	word;
	size_t 		pos;

	if ((pos = _req.find(delimiter_dic, _pos)) != std::string::npos)
	{
		word = _req.substr(_pos, pos - _pos);
		_pos = pos;
		return (word);
	}
	return (std::string(""));
}

std::string	Request::_getURI(const char * delimiter_dic) throw(URITooLong)
{
	std::string	word;
	size_t 		pos;

	if ((pos = _req.find(delimiter_dic, _pos)) != std::string::npos)
	{
		if ((pos - _pos) > PATH_MAX)
			throw URITooLong();
		word = _req.substr(_pos, pos - _pos);
		_pos = pos;
		return (word);
	}
	return (std::string(""));
}

bool	Request::parseRequestLine(void) throw(NotImplemented, BadRequest, URITooLong)
{
	/* Check Method */
	method = _getWord(" ");
	if (method.empty() || _dic.methodDic.find(method) == _dic.methodDic.end())
		throw NotImplemented();

	/* Pass 1 Space */
	if (!_passStrictOneChar(' '))
		throw BadRequest();

	/* Check Request-URI */
	uri = _getURI(" ");
	if (uri.empty())
		throw BadRequest();

	/* Pass 1 Space */
	if (!_passStrictOneChar(' '))
		throw BadRequest();

	/* Check HTTP-Version */
	protocol_v = _getWord("\r");
	if (protocol_v.empty() || protocol_v != "HTTP/1.1")
		throw BadRequest();

	/* Check if end of the line (CRLF = \r\n) */
	if (!_passStrictOneChar('\r'))
		throw BadRequest();
	if (!_passStrictOneChar('\n'))
		throw BadRequest();

	return (true);
}

bool	Request::parseHeaders(void) throw(BadRequest)
{
	while (1)
	{
		if (_req.find("\r\n", _pos) == _pos)
			break ;
		std::string header_key = _getWord(":");	/* Get Header Key */

		if (header_key.empty())	/* Check Header Key */
			break ;
		if (header_key.find_first_of(" \t") != std::string::npos)
			throw BadRequest();
		else
		{
			if (!_passStrictOneChar(':'))	/* Check is ':' is present */
				throw BadRequest();

			_passOptionalChars("\t ");	/* Pass OWS */

			std::string header_val = _getWord("\r\n");	/* Get Header Value */

			if (!header_val.empty())	/* Check Header Value */
			{
				if ((header_val.rfind(" ") == header_val.size() - 1) || (header_val.rfind("\t") == header_val.size() - 1))
					header_val.erase(header_val.size() - 1, 1);
			}

			if (!headers.insert(std::make_pair(header_key, header_val)).second)	/* Insert Header */
				throw BadRequest();
		}

		if (!_passStrictOneChar('\r'))
			throw BadRequest();
		if (!_passStrictOneChar('\n'))
			throw BadRequest();
	}

	if (!_passStrictOneChar('\r'))
		throw BadRequest();
	if (!_passStrictOneChar('\n'))
		throw BadRequest();

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
			if (ft_isprint(*it))
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

bool	Request::_parseChunkedBody(void) throw(BadRequest)
{
	size_t pos = 0;
	size_t nb = 0;
	size_t size = 0;

	if ((pos = _req.find("\r\n")) == std::string::npos)
		return (false);

	if ((nb = _req.find_first_not_of("0123456789ABCDEFabcdef", 0)) != 0)
	{
		if (!(size = static_cast<unsigned long>(Request::ft_strtol_base((_req.substr(0, nb)), "0123456789abcdef"))))
			_foundEnd = true;

		if (nb != pos && _chunkedExtensionInvalid(_req.substr(nb, pos)))
		{
			body.clear();
			_req.clear();
			throw BadRequest();
		}

		if (_req.size() - pos - 2 < size + 2)
			return (false);

		if (_foundEnd)
		{
			if (_req.find("\r\n") != pos + 2)
			{
				if (_req.find("\r\n\r\n") == std::string::npos)
					return (false);

				_req.erase(0, pos + 2);
				parseHeaders();
			}
			_req.clear();
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
		return (true);
	}
	else
	{
		body.clear();
		_req.clear();
		throw BadRequest();
	}
}

bool	Request::_checkTransferEncoding(std::string & second)
{
	std::string tmp = second;

	while (!tmp.empty())
	{
		size_t pos = 0;
		if ((pos = tmp.find(",")) != std::string::npos)
		{
			std::string tmp2 = tmp.substr(0, pos);
			if (tmp2.find_last_of(" \t") == tmp2.size() - 1)
				tmp2.erase(tmp2.size() - 1, 1);
			if (tmp2 == "identity")
			{
				tmp.erase(0, pos + 1);
				if (tmp.find_first_of(" \t") == 0)
					tmp.erase(0, 1);
			}
			else
				return (false);
		}
		else
		{
			if (tmp != "chunked" && tmp != "identity")
				return (false);
			second = tmp;
			tmp.clear();
		}
	}
	return (true);
}

bool	Request::parseBody(void) throw(NotImplemented, BadRequest)
{
	if (headers.find("Transfer-Encoding") != headers.end())
	{
		if (_checkTransferEncoding(headers.find("Transfer-Encoding")->second) == false)
			throw NotImplemented();

		if (headers.find("Transfer-Encoding")->second == "chunked")
		{
			while (_parseChunkedBody())
			{
				if (_foundEnd == true)
					return (true);
			}
			return (false);
		}
	}
	if (headers.find("Content-Length") != headers.end())
	{
		if (!str_is(headers.find("Content-Length")->second, ft_isdigit))
			throw BadRequest();

		errno = 0;
		size_t size = static_cast<size_t>(Request::ft_strtol_base(headers.find("Content-Length")->second, "0123456789"));
		if (errno == ERANGE) /* Verifying errno for ft_strtol function */
			throw BadRequest();

		if (size > _req.size() - _pos)
			return (false);
		body.append(_req, _pos, size - _pos);
		return (true);
	}
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

int		Request::isValidHost(int c)
{
	std::string dic("/<>@\\{}^`|#\"");

	if (dic.find(static_cast<char>(c)) == std::string::npos)
		return (1);
	return (0);
}

int		Request::ft_isprint(int c)
{
	if (c >= 32 && c <= 126)
		return (1);
	return (0);
}

int		Request::ft_isdigit(int c)
{
	if (c >= '0' && c <= '9')
		return (1);
	return (0);
}

size_t	Request::ft_strlen(const char * s)
{
	size_t i = 0;

	while (s[i])
		i++;
	return (i);
}

size_t	Request::ft_strcmp(const char *s1, const char *s2)
{
	size_t i;

	i = 0;
	while (s1[i] == s2[i] && s2[i] != '\0' && s1[i] != '\0')
		i++;
	return static_cast<unsigned int>(s1[i] - s2[i]);
}

void *	Request::ft_memcpy(void *dst, const void *src, size_t n)
{
	size_t	i;

	i = 0;
	if (dst == NULL && src == NULL)
		return (NULL);
	while (i < n)
	{
		((char*)dst)[i] = ((char*)src)[i];
		i++;
	}
	return (dst);
}

char *	Request::ft_strdup(const char *s1)
{
	char *	c;
	size_t	size;

	size = ft_strlen((char*)s1);
	if ((c = static_cast<char *>(malloc(size * sizeof(char) + 1))) == NULL)
		return (NULL);
	Request::ft_memcpy(c, s1, size);
	c[size] = '\0';
	return (c);
}

int    Request::ft_toupper(int c)
{
    if (c >= 'a' && c <= 'z')
        return (c - 32);
    else
        return (c);
}

std::string	Request::ft_inet_ntoa(in_addr addr)
{
	std::stringstream	s_str;

	for (int i = 0; i < 4; i++)
	{
		s_str << ((ntohl(addr.s_addr) >> (8 * (3 - i))) & 0xFF);
		if (i < 3)
			s_str << ".";
	}
	return (s_str.str());
}

int	Request::ft_atoi(const char *str)
{
	int	i;
	int k;
	int res;

	i = 0;
	k = 0;
	while ((str[i] > 8 && str[i] < 14) || str[i] == 32)
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			k++;
		i++;
	}
	res = 0;
	while (str[i] > 47 && str[i] < 58)
	{
		res = res * 10;
		res = res + str[i] - 48;
		i++;
	}
	if (k % 2 != 0)
		res = res * -1;
	return (res);
}

float	Request::ft_atof(const char *arr)
{
    int i,j,flag;
    float val;
    char c;
    i=0;
    j=0;
    val=0;
    flag=0;
    while ((c = *(arr+i))!='\0'){
        if (c!='.'){
            val =(val*10)+(c-'0');
            if (flag == 1){
                --j;
            }
        }
        if (c=='.'){ if (flag == 1) return 0; flag=1;}
        ++i;
    }
    val = val*static_cast<float>(ft_pow(10,j));
    return val;
}

double	Request::ft_pow(int a, int b)
{
    double aPuissanceB=1;
    for(int i=0;i<b;i++)
    {
		aPuissanceB*=a;
    }
	return aPuissanceB;
}

long long	Request::ft_power(long long nbr, long long power)
{
	long long	newnbr;

	newnbr = nbr;
	if (power == 0)
		return (1);
	while (power > 1)
	{
		newnbr = newnbr * nbr;
		power--;
	}
	return (newnbr);
}

long long	Request::ft_posbase(char c, const char * base)
{
	long i;

	i = 0;
	while (base[i])
	{
		if (c == base[i])
			return (i);
		i++;
	}
	return (0);
}

long	Request::ft_atol_base(const char * str_prim, const char * base)
{
	long long	nbr = 0;
	long long	blen = 0;
	long long	sign = 0;
	long long	slen = 0;
	long long	i = 0;
	char * str;

	i = 0;		
	str = Request::ft_strdup(str_prim);
	if (str == NULL)
		return (nbr);
	while (str[i])
	{
		str[i] = static_cast<char>(ft_tolower(str[i]));
		i++;
	}
	i = 0;
	sign = 1;
	slen = static_cast<long long>(Request::ft_strlen(str));
	if (slen > 19)
	{
		errno = ERANGE;
		free(str);
		return (nbr);
	}	
	blen = static_cast<long long>(Request::ft_strlen(base));
	nbr = 0;
	while ((str[i] >= 9 && str[i] <= 13) || str[i] == 32)
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (i < slen)
	{
		long long tmp = Request::ft_posbase(str[i], base) * Request::ft_power(blen, (slen - i - 1));
		if (Request::ft_posbase(str[i], base) == tmp / Request::ft_power(blen, (slen - i - 1)))
			nbr = nbr + Request::ft_posbase(str[i], base) * Request::ft_power(blen, (slen - i - 1));
		else
			errno = ERANGE;
		i++;
	}
	free(str);
	nbr = nbr * sign;
	return (nbr);
}

long	Request::ft_strtol_base(std::string str, const char * base)
{
	return (Request::ft_atol_base(str.c_str(), base));
}
