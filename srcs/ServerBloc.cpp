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

/* Getters and Setters */
size_t &	ServerBloc::getNo(void)
{
	return (_server_no);
}

ConfigParser *	ServerBloc::getParent(void)
{
	return (_parent);
}

/* Member Functions */
void	ServerBloc::parseException(const char * code)
{
	std::string str(code);
	std::map<std::string, std::string>::iterator it = _parent->getDictionary().errorDic.find(str);

	if (it != _parent->getDictionary().errorDic.end())
	{
		/* Fill Status Line */
		resp.status_code = it->first;
		resp.reason_phrase = it->second;

		Methods	implementedMethods(*this, resp.status_code, resp.reason_phrase);
	}
}

bool	ServerBloc::readClient(int client_socket)
{
	char	recv_buffer[MAX_HEADER_SIZE];

	ssize_t receivedBytes = read(client_socket, &recv_buffer, MAX_HEADER_SIZE);

	if (receivedBytes < 0)
	{
		std::cerr << "Error in read(): " << strerror(errno) << ENDL;
		COUT << MAGENTA << "WTF" << RESET << ENDL;
		return (false);
	}

	size_t old_pos = req.getData().size() > 4 ? req.getData().size() - 4 : 0;

	req.getData().append(recv_buffer, static_cast<size_t>(receivedBytes));

	// if (req.headerComplete || receivedBytes == 0)	/* Headers seems complete || client connection closed or EOF ! */
	if (req.headerComplete)	/* Headers seems complete */
		return (true);
	else if (receivedBytes == 0)	/* client connection closed or EOF ! */
	{
		COUT << MAGENTA << "Client connection closed or EOF" << RESET << ENDL;
		return (true);
	}
	else if ((req.getData().find("\r\n\r\n", old_pos) == std::string::npos))
	{
		// COUT << MAGENTA << "Not Found ending sequence" << RESET << ENDL;
		return (false); /* Not found ending sequence */
	}

	/* Request is complete */
	req.headerComplete = 1;
	return (true);
}

bool	ServerBloc::processRequest(Socket & client)
{
	static bool	headerParsed = false;

	if (!headerParsed)
	{
		req.client = &client;

		if (req.parseRequestLine())
			// CME << "> Parsed Request-line: COMPLETE !" << EME;
		if (req.parseHeaders())
			// CME << "> Parsed Headers: COMPLETE !" << EME;
		headerParsed = true;
		req.getData().erase(0, req.getData().find("\r\n\r\n") + 4);
	}
	if (req.parseBody())
	{
		// CME << "> Parsed Body: COMPLETE !" << EME;

		/* Cleaning */
		req.getData().clear();			/* Clearing _req buffer */
		req.headerComplete = false;		/* Reseting bool indicator if header is complete or not */
		headerParsed = false;			/* Reseting bool indicator if header is parsed or not */

		// COUT << MAGENTA << "Avant Exec" << RESET << ENDL;

		/* Execute the parsed request */
		Methods	implementedMethods(*this);
		implementedMethods.execute();

		// COUT << MAGENTA << "AFter Exec" << RESET << ENDL;

		/* Clean Request */
		req.clear();

		return (true);
	}
	return (false);
}

void	ServerBloc::_addHeaderFields(void)
{
	/* Create corresponding header fields */
	resp.header_fields.insert(std::make_pair("Date", _getDate()));
	resp.header_fields.insert(std::make_pair("Server", dir.find("server_name")->second[0]));

	/* Cache indications */
	resp.header_fields.insert(std::make_pair("Cache-Control", "no-store"));
	// resp.header_fields.insert(std::make_pair("Cache-Control", "max-age=10"));

	resp.header_fields.insert(std::make_pair("Connection", "close"));
}

bool	ServerBloc::sendResponse(Socket & client)
{
	if (!resp.isComplete)
	{
		_addHeaderFields();	/* Add the right header fields */
		resp.concatenateResponse();	/* Fill response msg */
	}
	if (resp.sendMsg(client.fd, resp.msg) == true)
	{
		// COUT << "Cleaning now\n";
		resp.cleanResponse();
		return (true);
	}
	return (false);
}

std::string	ServerBloc::_getDate(void)
{
	struct timeval	time_val;
	struct tm		*time;
	char			buffer[30];

	gettimeofday(&time_val, nullptr);
	time = gmtime(&time_val.tv_sec); // to check (time_t to struct tm)
	strftime(buffer, 30, "%a, %d %b %Y %H:%M:%S GMT", time);
	std::string	str(buffer);
	return (str);
}
