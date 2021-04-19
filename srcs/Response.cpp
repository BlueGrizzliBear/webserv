# include "./Response.hpp"
# include "./ServerBloc.hpp"

/* Response Class Declaration */
/* Constructor */
/*	default		(1)	*/
Response::Response(void) : status_code(""), reason_phrase(""), header_fields(), body(""), msg(""), writtenBytes(0), isComplete(0) {}

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

	writtenBytes = rhs.writtenBytes;
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

	/* New line */
	msg += "\r\n";

	CME << "> RESPONSE\n" << msg << EME;

	/* Body */
	if (!body.empty())
	{
		msg += body;
		// CME << "-- with Body --\n" << EME;
	}

	/* Initialisation de count */
	isComplete = 1;
}

bool	Response::sendResptoClient(Client & client)
{
	ssize_t sentBytes = send(client.socket.fd, &client.resp.msg.data()[writtenBytes], client.resp.msg.length() - writtenBytes, MSG_DONTWAIT);

	if (sentBytes < 0)
	{
		if (sentBytes < 0)
			CERR << "Error in send(): " << strerror(errno) << ENDL;
		// else
		// 	CERR << "Error in send(): client closed connection" << ENDL;
		// client.clientClosed = true;
		return (true);
	}

	if ((writtenBytes += static_cast<size_t>(sentBytes)) == client.resp.msg.length())
	{
		// COUT << "------------------Complete message sent-------------------" << ENDL;
		return (true);
	}
	// COUT << "Sent|" << writtenBytes << "/" << client.resp.msg.length() << "|bytes" << ENDL;
	return (false);
}

void	Response::cleanResponse(void)
{
	/* Cleaning Response */
	status_code.clear();	/* Status Line */
	reason_phrase.clear();	/* Status Line */
	header_fields.clear();	/* Header Fields */
	body.clear();			/* Body */
	body.reserve();								/* Reserver */

	/* Cleaning Msg */
	msg.clear();	/* Msg */
	msg.reserve();								/* Reserver */
	isComplete = 0;	/* Msg status */
}
