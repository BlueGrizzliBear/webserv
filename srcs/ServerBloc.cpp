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

void	ServerBloc::parseRequest(const char * request)
{
	/* Displaying Client request */
	COUT << "Received Data from client\n";
	std::cerr << "|" << GREEN << request << RESET << "|" << std::endl;

	/* Constructing request object from request char * */
	Request new_req(*this, request);
	req = new_req;
}

void	ServerBloc::executeRequest(void)
{
	// Pour Oliv
	COUT << "Executing Request here" << ENDL;
}

void	ServerBloc::sendResponse(Socket & client)
{
	/* Answering to the Client tmp */
	char hello[3000] = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";

	write(client.fd, hello, strlen(hello));

	COUT << "------------------Hello message sent-------------------" << ENDL;

	COUT << "Sending Response to Client" << ENDL;
}
