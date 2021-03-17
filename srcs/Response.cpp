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