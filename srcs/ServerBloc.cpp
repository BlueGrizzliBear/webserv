#include "./ServerBloc.hpp"

/* ServerBloc Class Declaration */
/* Constructor */
/*	default		(1)	*/
ServerBloc::ServerBloc(void) : pid(0), _server_no(0), _parent(NULL) {}

/*	default		(1)	*/
ServerBloc::ServerBloc(ConfigParser * parent) : pid(0), _server_no(0), _parent(parent) {}

/*	copy		(2)	*/
ServerBloc::ServerBloc(ServerBloc const & cpy)
{
	*this = cpy;
}

/* Destructor */
ServerBloc::~ServerBloc() {}

/* Operators */
ServerBloc &	ServerBloc::operator=(ServerBloc const & rhs)
{
	dir = rhs.dir;
	loc = rhs.loc;
	serv_port = rhs.serv_port;
	serv_select = rhs.serv_select;
	pid = rhs.pid;
	_server_no = rhs._server_no;
	_parent = rhs._parent;
	return (*this);
}

/* Member Functions */
size_t &	ServerBloc::getNo(void)
{
	return (_server_no);
}

ConfigParser *	ServerBloc::getParent(void)
{
	return (_parent);
}

void	ServerBloc::parseException(const char * code)
{
	std::string str(code);
	std::map<std::string, std::string>::iterator it = _parent->getDictionary().errorDic.find(str);

	if (it != _parent->getDictionary().errorDic.end())
	{
		/* Fill Status Line */
		resp.status_code = it->first;
		resp.reason_phrase = it->second;

		/* Fill Header Fields */
		resp.header_fields.insert(std::make_pair("Content-Type", "text/plain"));
	}
}

bool	ServerBloc::readClient(int client_socket)
{
	char	recv_buffer[MAX_HEADER_SIZE];

	ssize_t receivedBytes = recv(client_socket, &recv_buffer, (MAX_HEADER_SIZE - 1), 0); /* No Flag, CAREFUL */
	if (receivedBytes < 0)
	{
		std::cerr << "Error in recv(): " << strerror(errno) << ENDL;
		return (false);
	}		

	recv_buffer[static_cast<size_t>(receivedBytes)] = '\0';
	req.receivedData << recv_buffer;

	if ((req.receivedData.str().find("\r\n\r\n") == std::string::npos) || (receivedBytes == 0))
	/* Found ending sequence | client closed connection */
		return (false);	/* request is not complete */
	else
	/* Found ending sequence */
		return (true);	/* request is complete ! */
}

bool	ServerBloc::processRequest(void)
{
	/* Create request with parser */
	Request new_req(req.receivedData.str());
	req = new_req;

	/* Execute the parsed request */
	if (req.isComplete())
	{
		CME << "> Parsed Body: COMPLETE !" << EME;
		req.receivedData.str(""); /* Reseting string stream */
		executeRequest();
		return (true);
	}
	return (false);
}

void	ServerBloc::executeRequest(void)
{
	if (req.method == "GET")
		_applyGet();
	else if (req.method == "HEAD")
		_applyHead();
	else if (req.method == "POST")
	{
		_applyPost();
		// throw BadRequest();
		// throw UnsupportedMediaType();
		// throw Unauthorized();
	}

	/* Depending on Execution */
	/* Fill Status Line */
	resp.status_code = "200";
	resp.reason_phrase = "OK";

	/* Fill Header Fields */
	resp.header_fields.insert(std::make_pair("Connection", "close"));
	resp.header_fields.insert(std::make_pair("Content-Length", _getSizeOfStr(resp.body)));

	CME << "> EXECUTION DONE !" << EME;
}

void	ServerBloc::_addHeaderFields(void)
{
	/* Create corresponding header fields */
	resp.header_fields.insert(std::make_pair("Date", _getDate()));
	resp.header_fields.insert(std::make_pair("Server", dir.find("server_name")->second[0]));

	/* Cache indications */
	resp.header_fields.insert(std::make_pair("Cache-Control", "no-store"));
	// resp.header_fields.insert(std::make_pair("Cache-Control", "max-age=10"));

	/* specific case header fields */
		// MAKE SPECIFIC FUNCTION
	/* If status code 405 Method not allowed, repond with the server allowed method */
	if (resp.status_code == "405")
		resp.header_fields.insert(std::make_pair("Allow", "GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, WHAT, PATCH"));
	/* If status code 401 Unauthorized, respond with the WWW-authenticate to specify needed format */
	if (resp.status_code == "401")
		resp.header_fields.insert(std::make_pair("WWW-Authenticate", "Basic"));
}

bool	ServerBloc::sendResponse(Socket & client)
{
	if (!resp.isComplete)
	{
		_addHeaderFields();	/* Add the right header fields */	
		resp.concatenateResponse();	/* Fill response msg */
	}
	return (resp.sendMsg(client.fd));
}

std::string	ServerBloc::_getSizeOfStr(std::string const & str)
{
	std::stringstream size;
	size << str.length();
	return (size.str());
}

std::string	ServerBloc::_getDate(void)
{
	struct timeval	time_val;
	struct tm		*time;
	char			buffer[30];

	gettimeofday(&time_val, nullptr);
	time = gmtime(&time_val.tv_sec);
	strftime(buffer, 30, "%a, %d %b %Y %H:%M:%S GMT", time);
	std::string	str(buffer);
	return (str);
}
