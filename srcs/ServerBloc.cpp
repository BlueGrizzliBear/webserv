#include "./ServerBloc.hpp"

/* ServerBloc Class Declaration */
/* Constructor */
/*	default		(1)	*/
ServerBloc::ServerBloc(void) : server_fd(), address(), addrlen(sizeof(address)), pid(0), _server_no(0), _parent(NULL) {}

/*	default		(1)	*/
ServerBloc::ServerBloc(ConfigParser * parent) : server_fd(), address(), addrlen(sizeof(address)), pid(0), _server_no(0), _parent(parent) {}

/*	copy		(2)	*/
ServerBloc::ServerBloc(ServerBloc const & cpy) : server_fd(), address(), addrlen(sizeof(address)) 
{
	*this = cpy;
}

/* Destructor */
ServerBloc::~ServerBloc() {}

/* Operators */
ServerBloc &	ServerBloc::operator=(ServerBloc const & rhs)
{
	serv_dir = rhs.serv_dir;
	serv_loc = rhs.serv_loc;
	server_fd = rhs.server_fd;
	address = rhs.address;
	addrlen = rhs.addrlen;
	pid = rhs.pid;
	_server_no = rhs._server_no;
	_parent = rhs._parent;
	return (*this);
}

/* Member Functions */
unsigned short	ServerBloc::getPort(void)
{
	std::vector<std::string> search_vector;
	search_vector.push_back("listen");
	search_vector.push_back("");

	if (serv_dir.lower_bound(search_vector) == serv_dir.end())
		return (PORT); /* default value for port if missing a value */
	
	std::string tmp(serv_dir.lower_bound(search_vector)->first[1]);

	if (tmp.find_first_not_of("0123456789") != std::string::npos)
		throw std::exception();
	int port = atoi(tmp.c_str());
	if (port > USHRT_MAX || port < 0)
		throw std::exception();
	return (static_cast<unsigned short>(port));
}

size_t &	ServerBloc::getNo(void)
{
	return (_server_no);
}

ConfigParser *	ServerBloc::getParent(void)
{
	return (_parent);
}