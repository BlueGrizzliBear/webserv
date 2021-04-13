#include "./Response.hpp"

/* Response Class Declaration */
/* Constructor */
/*	default		(1)	*/
Response::Response(void) : status_code(""), reason_phrase(""), header_fields(), body(""), msg(""), isComplete(0) {}

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

	msg = rhs.msg;

	isComplete = rhs.isComplete;

	return (*this);
}

/* Member Functions */
void	Response::concatenateResponse(void)
{
	/* Status Line */
	msg = "HTTP/1.1 " + status_code + " " + reason_phrase + "\r\n";

	/* Header Fields */
	std::map<std::string, std::string, ci_less>::iterator begin = header_fields.begin();
	while (begin != header_fields.end())
	{
		msg += begin->first + ": " + begin->second + "\r\n";
		begin++;
	}

	/* Display Temporary msg */
	// CME << "> RESPONSE\n" << msg << EME;

	/* New line */
	msg += "\r\n";

	/* Body */
	if (body.size())
	{
		msg += body;
		// CME << "-- with Body --\n" << EME;
	}

	/* Initialisation de count */
	isComplete = 1;

}

bool	Response::sendMsg(int client_socket, std::string & message)
{
	static size_t	count = 0;

	ssize_t writtenBytes = send(client_socket, &message.data()[count], message.length() - count, MSG_DONTWAIT);

	if (writtenBytes < 0)
	{
		CERR << "Error in write(): " << strerror(errno) << ENDL;
		return (true);
	}
	count += static_cast<size_t>(writtenBytes);

	if (count == message.length())
	{
		count = 0;
		// COUT << "------------------Complete message sent-------------------" << ENDL;

		return (true);
	}
	// COUT << "Sent|" << count << "/" << message.length() << "|bytes" << ENDL;
	return (false);
}

bool	Response::sendMsgCGI(int client_socket, std::string & message)
{
	static size_t	count = 0;

	ssize_t writtenBytes = write(client_socket, &message.data()[count], message.length() - count);

	if (writtenBytes < 0)
	{
		CERR << "Error in write(): " << strerror(errno) << ENDL;
		return (true);
	}
	count += static_cast<size_t>(writtenBytes);

	if (count == message.length())
	{
		count = 0;
		// COUT << "------------------Complete message sent-------------------" << ENDL;

		return (true);
	}
	// COUT << "Sent|" << count << "/" << message.length() << "|bytes" << ENDL;
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
