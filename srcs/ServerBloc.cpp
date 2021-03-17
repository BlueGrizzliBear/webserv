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
		resp.status = it->first;
		resp.body = it->second;
		resp.status_msg = "OK";
		resp.content_type = "text/plain";
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
	}
}

void	ServerBloc::executeRequest(void)
{
	// Pour Oliv

	COUT << "Executing Request here" << ENDL;

	/* En attendant */
	resp.status = "200";
	resp.status_msg = "OK";
	resp.content_type = "text/plain";
	resp.body = "WebServ says \"Hi\" !";
}

void	ServerBloc::sendResponse(Socket & client)
{
	/* Finish initializing responsonse */
	resp.version = "HTTP/1.1";
	resp.date = "date";
	resp.server = dir.find("server_name")->second[0];
	resp.content_length = sizeof(resp.body);

	/* Fill response msg */
	std::string msg;
	msg = resp.version + resp.status + resp.status_msg
		+ resp.date
		+ resp.server
		+ resp.content_type
		+ resp.content_length
		+ "\n"
		+ resp.body;

	/* Answering to the Client tmp */
	// char hello[3000] = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";

	/* Send message to client */
	// write(client.fd, hello, strlen(hello));
	write(client.fd, msg.c_str(), msg.length());
	COUT << "------------------Hello message sent-------------------" << ENDL;

	COUT << "Sending Response to Client" << ENDL;
}
