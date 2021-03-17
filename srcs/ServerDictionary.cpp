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
								"server"					};
	_createDic(mainDic, main_dir, sizeof(main_dir));

	/* Dictionary for keys in location blocs */
	std::string locations[] = {	"root",
								"autoindex",
								"limit_except",
								"upload_store",
								"expires",
								"proxy_pass",
								"cgi"			};
	_createDic(locationDic, locations, sizeof(locations));

	/* Dictionary for keys in server blocs */
	std::string servers[] = {	"listen",
								"server_name",
								"error_page",
								"location",
								"root",
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

	// case insensitive
	/* Dictionary for implemented Headers in requests */
	std::string headers[] = {	"Accept-Charsets",
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
	std::pair<std::string, std::string>	error_codes[] = {	std::make_pair("400", "Bad Request"),
															std::make_pair("501", "Not Implemented"),
															std::make_pair("501", "Not Implemented")	};
	_createDic(errorDic, error_codes, sizeof(error_codes));
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
void	ServerDictionary::_createDic(Dic & dic, std::string const * tab, size_t size)
{
	for (size_t i = 0; i < (size / sizeof(*tab)); i++)
		dic.insert(std::make_pair(*(tab + i), ""));
}

/* . . . overloaded function for error-codes */
void	ServerDictionary::_createDic(Dic & dic, std::pair<std::string , std::string> const * tab, size_t size)
{
	for (size_t i = 0; i < (size / sizeof(*tab)); i++)
		dic.insert(*(tab + i));
}