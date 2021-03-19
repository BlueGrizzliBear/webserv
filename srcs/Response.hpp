#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "./webserv.hpp"
#include "./ServerDictionary.hpp"

/* Response Class Declaration */
class Response
{
	public:
	/* Member Types */
		typedef std::map<std::string, std::string > Headers;

	/* Constructor */
		/*	default		(1)	*/	Response(void);
		/*	copy		(2)	*/	Response(Response const & cpy);

	/* Destructor */
		~Response();

	/* Operators */
		Response &	operator=(Response const & rhs);
	
	/* Exceptions */

	/* Member Functions */
	public:

	/* Member Attributes */
	public:
		std::string			status_code;	/* Status Line */
		std::string			reason_phrase;	/* Status Line */

		Headers			header_fields;	/* Header Fields */

		std::string		body;			/* Body */
};

#endif