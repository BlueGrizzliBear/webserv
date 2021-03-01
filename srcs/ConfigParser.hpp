#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <iostream>

#include "./webserv.hpp"

class ConfigParser
{
	/* Constructor */
	private:
		/*	default		(1)	*/	ConfigParser(void);

	public:
		/*	argument	(2)	*/	ConfigParser(char const * path);
		/*	copy		(3)	*/	ConfigParser(ConfigParser const & cpy);

	/* Destructor */
		~ConfigParser();

	/* Operators */
		ConfigParser &	operator=(ConfigParser const & rhs);
		
	/* Exceptions */
		class ErrorConfig : public std::exception
		{
			public:
				virtual const char *	what() const throw() { return ("Configuration File is corrupted."); }
		};

		class LocationError : public std::exception
		{
			public:
				virtual const char *	what() const throw() { return ("Configuration File is missing at location."); }
		};

	/* Member Functions */

		
	/* Member Attributes */
	private:

};

#endif
