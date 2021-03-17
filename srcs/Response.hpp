#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "./webserv.hpp"
#include "./ServerDictionary.hpp"

/* Response Class Declaration */
class Response
{
	public:
	/* Member Types */
		typedef std::map<std::string, std::vector<std::string> > Headers;

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
		// void	setClientFd(int client_fd);

	/* Member Attributes */
	public:
		/* Header */
		std::string		version;
		std::string		status;
		std::string		status_msg;
		std::string		date;
		std::string		server;
		std::string		content_type;
		std::string		content_length;
		/* Body */
		std::string		body;

	// private:
	// 	int		_client_fd;
};

#endif