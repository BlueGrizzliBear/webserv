#include "./Response.hpp"

/* Response Class Declaration */
/* Constructor */
/*	default		(1)	*/
Response::Response(void) {}

/*	copy		(2)	*/
Response::Response(Response const & cpy)
{
	*this = cpy;
}

/* Destructor */
Response::~Response() {}

/* Operators */
Response &	Response::operator=(Response const & rhs)
{
	version = rhs.version;
	status = rhs.status;
	status_msg = rhs.status_msg;
	date = rhs.date;
	server = rhs.server;
	content_type = rhs.content_type;
	content_length = rhs.content_length;
	body = rhs.body;
	return (*this);
}

/* Member Functions */
// void	Response::setClientFd(int client_fd)
// {
// 	_client_fd = client_fd;
// }