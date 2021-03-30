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

		Methods	implementedMethods(*this);
		implementedMethods.customError(it->first, it->second);
	}
}

bool	ServerBloc::readClient(int client_socket)
{
	char	recv_buffer[MAX_HEADER_SIZE];

	ssize_t receivedBytes = recv(client_socket, &recv_buffer, MAX_HEADER_SIZE, 0); /* No Flag, CAREFUL */
	if (receivedBytes < 0)
	{
		std::cerr << "Error in recv(): " << strerror(errno) << ENDL;
		return (false);
	}

	size_t old_pos = req.getData().size() > 4 ? req.getData().size() - 4 : 0;

	req.getData().append(recv_buffer, static_cast<size_t>(receivedBytes));

	if (req.headerComplete)
		return (true);	/* request is already complete ! */
	else if ((req.getData().find("\r\n\r\n", old_pos) == std::string::npos) || (receivedBytes == 0))
	/* Found ending sequence | client closed connection */
		return (false);	/* request is not complete */
	
	/* Request is complete */
	req.headerComplete = 1;
	return (true);
}

bool	ServerBloc::processRequest(void)
{
	static bool	headerParsed = false;

	if (!headerParsed)
	{
		if (req.parseRequestLine())
			CME << "> Parsed Request-line: COMPLETE !" << EME;
		if (req.parseHeaders())
			CME << "> Parsed Headers: COMPLETE !" << EME;
		headerParsed = true;
	}
	if (req.parseBody())
	{
		CME << "> Parsed Body: COMPLETE !" << EME;
		
		/* Cleaning */
		req.getData().clear();			/* Clearing _req buffer */
		headerParsed = false;			/* Reseting bool indicator if header is parsed or not */
		req.headerComplete = false;		/* Reseting bool indicator if header is complete or not */

		/* Execute the parsed request */
		executeRequest();

		/* Clean Request */
		req.clear();

		return (true);
	}

	return (false);
}

void	ServerBloc::executeRequest(void)
{
	Methods	implementedMethods(*this);

	implementedMethods.execute();

	/* Fill Header Fields */
	resp.header_fields.insert(std::make_pair("Connection", "close"));

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
