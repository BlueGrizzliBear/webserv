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

		/* Fill Body */
		// resp.body = 

		/* Fill Header Fields */
		resp.header_fields.insert(std::make_pair("Content-Type", "text/plain"));
		resp.header_fields.insert(std::make_pair("Content-Length", _getSizeOfBody()));
	}
}

void	ServerBloc::readClient(int client_socket)
{
	if (req.finished && !serv_select.incomplete)
	{
		req.ss.str("");
		req.finished = 0;
		serv_select.incomplete = 1;
	}
	serv_select.n = 0;
	if ((serv_select.n = recv(client_socket, serv_select.buf, (MAX_HEADER_SIZE - 1), 0)) < 0)
	{
		std::cerr << "Error in recv(): " << strerror(errno) << ENDL;
		serv_select.n = 0;
		return ;
	}
	serv_select.buf[serv_select.n] = '\0';
	req.ss << serv_select.buf;

	if (req.ss.str().find("\r\n\r\n") == std::string::npos)
	{
		// COUT << "Incomplete request|" << req.ss.str() << "|" << ENDL;
		serv_select.incomplete = 1;
	}
	else
		/* Found ending sequence */
		serv_select.incomplete = 0;

	/* If client closed the connection or if we arrived at eof */
	if (serv_select.n == 0)
		serv_select.incomplete = 0;

	return ;
}

void	ServerBloc::processRequest(void)
{
	/* Displaying Client request */
	COUT << "Received Data from client\n";
	std::cerr << "|" << GREEN << req.ss.str() << RESET << "|" << std::endl;

	try
	{
		/* Parse request */
		Request new_req(req.ss);
		req = new_req;

		/* Execute the parsed request */
		executeRequest();
	}
	catch(const std::exception& e)
	{
		std::cerr << "Exception caught|" << e.what() << "|" << ENDL;
		parseException(e.what());
		throw(e);
	}
	/* Depending on Execution */
	/* Fill Status Line */
	resp.status_code = "200";
	resp.reason_phrase = "OK";

	/* Fill Body */
	resp.body = "WebServ says < Hi > !";

	/* Fill Header Fields */
	resp.header_fields.insert(std::make_pair("Connection", "close"));
	resp.header_fields.insert(std::make_pair("Content-Type", "text/plain"));
	resp.header_fields.insert(std::make_pair("Content-Length", _getSizeOfBody()));		
}

void	ServerBloc::executeRequest(void)
{
	COUT << "Executing Request here" << ENDL;
	if (req.method == "GET")
		_applyGet();
	else if (req.method == "HEAD")
		_applyHead();
	// else if (req.method == "POST")
	// {}
	// else
	// {}
}

void	ServerBloc::sendResponse(Socket & client)
{
	/* Create corresponding header fields */
	resp.header_fields.insert(std::make_pair("Date", _getDate()));
	resp.header_fields.insert(std::make_pair("Server", dir.find("server_name")->second[0]));

	/* specific case header fields */
		// MAKE SPECIFIC FUNCTION
	/* If status code 405 Method not allowed, repond with the server allowed method */
	if (resp.status_code == "405")
		resp.header_fields.insert(std::make_pair("Allow", "GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, WHAT, PATCH"));
	/* If status code 401 Unauthorized, respond with the WWW-authenticate to specify needed format */
	if (resp.status_code == "401")
		resp.header_fields.insert(std::make_pair("WWW-Authenticate", "Basic"));


	/* Fill response msg */
	std::string msg = _concatenateResponse();
	/* Clear Map */
	resp.header_fields.clear();

	CME << "|" << msg << "|" << EME;

	/* Send message to client */
	write(client.fd, msg.c_str(), msg.length());
	COUT << "------------------Hello message sent-------------------" << ENDL;
}

std::string	ServerBloc::_getSizeOfBody(void)
{
	std::stringstream size;
	size << resp.body.length();
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

std::string	ServerBloc::_concatenateResponse(void)
{
	std::string msg;

	/* Status Line */
	msg = "HTTP/1.1 " + resp.status_code + " " + resp.reason_phrase + "\n";

	/* Header Fields */
	std::map<std::string, std::string>::iterator begin = resp.header_fields.begin();
	while (begin != resp.header_fields.end())
	{
		msg += begin->first + ": " + begin->second + "\n";
		begin++;
	}

	/* New line */
	msg += "\n";

	/* Body */
	if (resp.body.size())
		msg += resp.body;

	return (msg);
}
