#ifndef SERVERDICTIONARY_HPP
#define SERVERDICTIONARY_HPP

#include "./webserv.hpp"

class	ServerDictionary
{
	/* Member Types */
	public:
		typedef std::map<std::string, int>	Dic;

	/* Constructor */
		/*	default	(1)	*/	ServerDictionary(void);
		/*	copy	(2)	*/	ServerDictionary(ServerDictionary const & cpy);

	/* Destructor */
		~ServerDictionary();

	/* Operators */
		ServerDictionary &	operator=(ServerDictionary const & rhs);

	/* Member Functions */

	/* Member Attributes */
		Dic	main_dictionary;
		Dic	location_dictionary;
		Dic	server_dictionary;
};

#endif
