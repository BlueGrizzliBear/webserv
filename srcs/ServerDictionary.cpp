#include "./ServerDictionary.hpp"

/* Constructor */
/*	default	(1)	*/
ServerDictionary::ServerDictionary()
{
	/* Dictionary for keys in main context */
	std::string main_dir[] = {	"user",
								"worker_processes",
								"pid",
								"worker_connections",
								"client_max_body_size",
								"cgi",
								"server"					};
	_createDic(mainDic, main_dir, sizeof(main_dir));

	/* Dictionary for keys in location blocs */
	std::string locations[] = {	"root",
								"index",
								"rewrite",
								"allowed_methods",
								"autoindex",
								"client_max_body_size",
								"limit_except",
								"upload_store",
								"auth_basic",
								"auth_basic_user_file",
								"expires",
								"proxy_pass",
								"cgi"			};
	_createDic(locationDic, locations, sizeof(locations));

	/* Dictionary for keys in server blocs */
	std::string servers[] = {	"listen",
								"server_name",
								"error_page",
								"location",
								"auth_basic",
								"auth_basic_user_file",
								"root",
								"index",
								"autoindex"		};
	_createDic(serverDic, servers, sizeof(servers));

	/* Dictionary for methods in requests */
	std::string methods[] = {	"GET",
								"HEAD",
								"POST",
								"PUT",
								"DELETE",
								"CONNECT",
								"OPTIONS",
								"TRACE",
								"WHAT",
								"PATCH"		};
	_createDic(methodDic, methods, sizeof(methods));

	/* Dictionary for implemented Headers in requests */
	std::string headers[] = {	"Accept-Charset",
								"Accept-Language",
								"Allow",
								"Authorization",
								"Content-Language",
								"Content-Length",
								"Content-Location",
								"Content-Type",
								"Date",
								"Host",
								"Last-Modified",
								"Location",
								"Referer",
								"Retry-After",
								"Server",
								"Transfer-Encoding",
								"User-Agent",
								"WWW-Authenticate"		};
	_createDic(headerDic, headers, sizeof(headers));

	/* Dictionary for error-codes */
	std::pair<std::string, std::string>	error_codes[] = {	std::make_pair("100", "Continue"),
															std::make_pair("101", "Switching Protocols"),
															std::make_pair("102", "Processing"),
															std::make_pair("103", "Early Hints"),
															std::make_pair("200", "OK"),
															std::make_pair("201", "Created"),
															std::make_pair("202", "Accepted"),
															std::make_pair("203", "Non-Authoritative Information"),
															std::make_pair("204", "No Content"),
															std::make_pair("205", "Reset Content"),
															std::make_pair("206", "Partial Content"),
															std::make_pair("207", "Multi-Status"),
															std::make_pair("208", "Already Reported"),
															std::make_pair("226", "IM Used"),
															std::make_pair("300", "Multiple Choices"),
															std::make_pair("301", "Moved Permanently"),
															std::make_pair("302", "Found"),
															std::make_pair("303", "See Other"),
															std::make_pair("304", "Not Modified"),
															std::make_pair("305", "Use Proxy"),
															std::make_pair("306", "(Unused)"),
															std::make_pair("307", "Temporary Redirect"),
															std::make_pair("308", "Permanent Redirect"),
															std::make_pair("400", "Bad Request"),
															std::make_pair("401", "Unauthorized"),
															std::make_pair("402", "Payment Required"),
															std::make_pair("403", "Forbidden"),
															std::make_pair("404", "Not Found"),
															std::make_pair("405", "Method Not Allowed"),
															std::make_pair("406", "Not Acceptable"),
															std::make_pair("407", "Proxy Authentication Required"),
															std::make_pair("408", "Request Timeout"),
															std::make_pair("409", "Conflict"),
															std::make_pair("410", "Gone"),
															std::make_pair("411", "Length Required"),
															std::make_pair("412", "Precondition Failed"),
															std::make_pair("413", "Payload Too Large"),
															std::make_pair("414", "URI Too Long"),
															std::make_pair("415", "Unsupported Media Type"),
															std::make_pair("416", "Range Not Satisfiable"),
															std::make_pair("417", "Expectation Failed"),
															std::make_pair("421", "Misdirected Request"),
															std::make_pair("422", "Unprocessable Entity"),
															std::make_pair("423", "Locked"),
															std::make_pair("424", "Failed Dependency"),
															std::make_pair("425", "Too Early"),
															std::make_pair("426", "Upgrade Required"),
															std::make_pair("428", "Precondition Required"),
															std::make_pair("429", "Too Many Requests"),
															std::make_pair("431", "Request Header Fields Too Large"),
															std::make_pair("451", "Unavailable For Legal Reasons"),
															std::make_pair("500", "Internal Server Error"),
															std::make_pair("501", "Not Implemented"),
															std::make_pair("502", "Bad Gateway"),
															std::make_pair("503", "Service Unavailable"),
															std::make_pair("504", "Gateway Timeout"),
															std::make_pair("505", "HTTP Version Not Supported"),
															std::make_pair("506", "Variant Also Negotiates"),
															std::make_pair("507", "Insufficient Storage"),
															std::make_pair("508", "Loop Detected"),
															std::make_pair("510", "Not Extended"),
															std::make_pair("511", "Network Authentication Required") };
	_createDic(errorDic, error_codes, sizeof(error_codes));

	_parseMimeTypes();
}

/*	copy	(2)	*/
ServerDictionary::ServerDictionary(ServerDictionary const & cpy)
{
	*this = cpy;
}

/* Destructor */
ServerDictionary::~ServerDictionary() {}

/* Operators */
ServerDictionary &	ServerDictionary::operator=( ServerDictionary const & rhs )
{
	(void)rhs;
	return (*this);
}

/* Member Functions */
template < class Compare >
void	ServerDictionary::_createDic(std::map< std::string, std::string, Compare > & dic, std::string const * tab, size_t size)
{
	for (size_t i = 0; i < (size / sizeof(*tab)); i++)
		dic.insert(std::make_pair(*(tab + i), ""));
}

/* . . . overloaded function for error-codes */
template < class Compare >
void	ServerDictionary::_createDic(std::map< std::string, std::string, Compare > & dic, std::pair<std::string , std::string> const * tab, size_t size)
{
	for (size_t i = 0; i < (size / sizeof(*tab)); i++)
		dic.insert(*(tab + i));
}

void	ServerDictionary::_parseMimeTypes(void)
{
	std::string		mime_value;
	std::string		mime_key;
	std::string		line;

	std::ifstream	file("./configuration/mime.types");
	if (file.good())
	{
		while (getline(file, line))
		{
			std::string::iterator	it = line.begin();
			for ( ; it != line.end() && *it != ' ' && *it != '\t'; ++it)
				mime_value += *it;
			while (it != line.end() && *it == ' ' && *it == '\t')
				++it;
			for ( ; it != line.end(); ++it)
			{
				for ( ; it != line.end() && *it != ' ' && *it != '\t' && *it != ';'; ++it)
					mime_key += *it;
				if (mime_value.empty() == false && mime_key.empty() == false)
					mimeDic.insert(std::make_pair(mime_key, mime_value));
				mime_key.clear();
			}
			mime_key.clear();
			mime_value.clear();
		}
	}
	file.close();
}
