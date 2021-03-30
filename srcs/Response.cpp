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
	/* Status Line */
	status_code = rhs.status_code;
	reason_phrase = rhs.reason_phrase;
	
	/* Header Fields */
	header_fields = rhs.header_fields;

	/* Body */
	body = rhs.body;
	
	return (*this);
}

/* Member Functions */
void	Response::concatenateResponse(void)
{
	/* Status Line */
	msg = "HTTP/1.1 " + status_code + " " + reason_phrase + "\r\n";

	/* Header Fields */
	std::map<std::string, std::string>::iterator begin = header_fields.begin();
	while (begin != header_fields.end())
	{
		msg += begin->first + ": " + begin->second + "\r\n";
		begin++;
	}

	/* New line */
	msg += "\r\n";

	/* Display Temporary msg */
	CME << "|" << msg << "|" << EME;

	/* Body */
	if (body.size())
		msg += body;

	/* Initialisation de count */
	isComplete = 1;
}

bool	Response::sendMsg(int client_socket)
{
	// et en profiter pour ne pas couper la connection si le read lit \0

	static size_t	count = 0;
	std::string		tmp(msg.substr(count, msg.length() - count));

	/* try to send */
	ssize_t writtenBytes = write(client_socket, tmp.data(), tmp.length());		
	if (writtenBytes < 0)
	{
		std::cerr << "Error in write(): " << strerror(errno) << ENDL;
		return (false);
	}
	count += static_cast<size_t>(writtenBytes);
	
	if (count == msg.length())
	{
		cleanResponse();
		count = 0;
		COUT << "------------------Complete message sent-------------------" << ENDL;
		return (true);
	}
	COUT << "Message not send, only sent|" << count << "/" << msg.length() << "|bytes" << ENDL;
	return (false);
}

void	Response::cleanResponse(void)
{
	/* Cleaning Response */
	status_code.clear();	/* Status Line */
	reason_phrase.clear();	/* Status Line */
	header_fields.clear();	/* Header Fields */
	body.clear();			/* Body */

	/* Cleaning Msg */
	msg.clear();	/* Msg */
	isComplete = 0;	/* Msg status */
}
