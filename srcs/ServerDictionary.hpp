#ifndef SERVERDICTIONARY_HPP
#define SERVERDICTIONARY_HPP

#include "./webserv.hpp"

class	ServerDictionary
{
	/* Constructor */
	public:
		/*	default	(1)	*/	ServerDictionary(void);
		/*	copy	(2)	*/	ServerDictionary(ServerDictionary const & cpy);

	/* Destructor */
		~ServerDictionary();

	/* Operators */
		ServerDictionary &	operator=(ServerDictionary const & rhs);

	/* Member Functions */

	/* Member Attributes */
		std::map<std::string, int>	context_dictionary;
		std::map<std::string, int>	location_dictionary;
		std::map<std::string, int>	server_dictionary;
};

#endif
