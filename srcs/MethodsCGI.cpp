#include "./Methods.hpp"

void	Methods::_executeCGI(void)
{
	_createCGIEnv();

}

char	** Methods::_createCGIEnv(void)
{
	char	** envp;

// AUTH_TYPE
	/* _checkAuthenticate already assign the correct value */
// CONTENT_LENGTH
	if (serv->req.headers.find("Content-Length") != serv->req.headers.end())
		_envp["CONTENT_LENGTH"] = serv->req.headers.find("Content-Length")->second;
	else
		_envp["CONTENT_LENGTH"] = "";
// CONTENT_TYPE
	if (serv->req.headers.find("Content-Type") != serv->req.headers.end())
		_envp["CONTENT_TYPE"] = serv->req.headers.find("Content-Type")->second;
	else
		_envp["CONTENT_TYPE"] = "";
// GATEWAY_INTERFACE
	_envp["GATEWAY_INTERFACE"] = "CGI/1.1";
// PATH_INFO
	_envp["PATH_INFO"] = _uriWithoutFirstPart();	// SUFFIXE DE L'URI UNIQUEMENT?
// PATH_TRANSLATED
	_envp["PATH_TRANSLATED"] = _path;
// QUERY_STRING
	// put the search identifier in the uri if any (query-string part of the uri) */
	// _envp["QUERY_STRING"] = ;
// REMOTE_ADDR
	_envp["REMOTE_ADDR"] = "localhost";
// REMOTE_IDENT
	// A VERIFIER
	if (serv->dir.find("server_name") != serv->dir.end())
		_envp["REMOTE_IDENT"] = serv->dir.find("server_name")->second[0];
	else
		_envp["REMOTE_IDENT"] = "";
// REMOTE_USER
	/* _checkAuthenticate already assign the correct value */
// REQUEST_METHOD
	_envp["REQUEST_METHOD"] = serv->req.method;
// REQUEST_URI
	_envp["REQUEST_URI"] = serv->req.uri;
// SCRIPT_NAME
	_envp["SCRIPT_NAME"] = _cgi_path;
// SERVER_NAME
	// A VERIFIER
	if (serv->dir.find("server_name") != serv->dir.end())
		_envp["SERVER_NAME"] = serv->dir.find("server_name")->second[0];
	else
		_envp["SERVER_NAME"] = "";
// SERVER_PORT
	if (serv->dir.find("listen") != serv->dir.end())
		_envp["SERVER_PORT"] = serv->dir.find("listen")->second[0];
	else
		_envp["SERVER_PORT"] = "";
// SERVER_PROTOCOL
	_envp["SERVER_PROTOCOL"] = "HTTP/1.1";
// SERVER_SOFTWARE
	_envp["SERVER_SOFTWARE"] = "Nginx/0.1";

	if (!(envp = (char *)malloc(sizeof(char *) * (_envp.size() + 1))))
		COUT << "CGI EXEC FAILED" << ENDL;

	int		i = 0;
	std::string	tmp;

	for (std::map<std::string, std::string>::iterator it = _envp.begin(); it != _envp.end(); ++it)
	{
		tmp = (*it).first + "=" + (*it).second;
		envp[i] = tmp.c_str();
		++i;
	}

}
