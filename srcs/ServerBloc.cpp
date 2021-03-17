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
		resp.reason_phrase = "OK";
	
		/* Fill Body */
		resp.body = it->second;

		/* Fill Header Fields */
		resp.header_fields.insert(std::make_pair("Content-Type", "text/plain"));
		resp.header_fields.insert(std::make_pair("Content-Length", _getSizeOfBody()));
	}
}

void	ServerBloc::parseRequest(const char * request)
{
	/* Displaying Client request */
	COUT << "Received Data from client\n";
	std::cerr << "|" << GREEN << request << RESET << "|" << std::endl;

	/* Constructing request object from request char * */
	try
	{
		Request new_req(request);
		req = new_req;
	}
	catch(const std::exception& e)
	{
		std::cerr << "Exception caught|" << e.what() << "|" << ENDL;
		parseException(e.what());
		throw(e);
	}
}

void	ServerBloc::executeRequest(void)
{
	// Pour Oliv

	COUT << "Executing Request here" << ENDL;

	/* Depending on Execution */
	/* Fill Status Line */
	resp.status_code = "200";
	resp.reason_phrase = "OK";

	/* Fill Body */
	resp.body = "WebServ says < Hi > !";	

	/* Fill Header Fields */
	resp.header_fields.insert(std::make_pair("Content-Type", "text/plain"));
	resp.header_fields.insert(std::make_pair("Content-Length", _getSizeOfBody()));
}

void	ServerBloc::sendResponse(Socket & client)
{
	/* Create corresponding header fields */
	resp.header_fields.insert(std::make_pair("Date", "date"));
	resp.header_fields.insert(std::make_pair("Server", dir.find("server_name")->second[0]));

	/* Fill response msg */
	std::string msg = _concatenateResponse();
	/* Clear Map */
	resp.header_fields.clear();

	CME << "|" << msg << "|" << EME;

	/* Send message to client */
	write(client.fd, msg.c_str(), msg.length());
	COUT << "------------------Hello message sent-------------------" << ENDL;

	COUT << "Sending Response to Client" << ENDL;
}

std::string	ServerBloc::_getSizeOfBody(void)
{
	std::stringstream size;
	size << resp.body.length();
	return (size.str());
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
