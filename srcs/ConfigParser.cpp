#include "./ConfigParser.hpp"

/* Constructor */
/*	default		(1)	*/
ConfigParser::ConfigParser() {}

/*	argument	(2)	*/
ConfigParser::ConfigParser(char const * path)
{
	std::string config(path);

	if (config.length() != 0)
		throw ErrorConfig();
}

/*	copy		(3)	*/
ConfigParser::ConfigParser(ConfigParser const & cpy)
{
	*this = cpy;
}

/* Destructor */
ConfigParser::~ConfigParser() {}

/* Operators */
ConfigParser &ConfigParser::operator=( ConfigParser const & rhs )
{
	(void)rhs;
	return (*this);
}

/* Member Functions */
