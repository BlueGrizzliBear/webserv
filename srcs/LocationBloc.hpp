#ifndef LOCATIONBLOC_HPP
#define LOCATIONBLOC_HPP

#include "./webserv.hpp"

/* LocationBloc Class Declaration */
class LocationBloc
{
	public:
	/* Member Types */
		typedef std::map<std::string, std::vector<std::string> >	Directives;

	/* Constructor */
		/*	default		(1)	*/	LocationBloc(void);
		/*	copy		(2)	*/	LocationBloc(LocationBloc const & cpy);

	/* Destructor */
		~LocationBloc();

	/* Operators */
		LocationBloc &	operator=(LocationBloc const & rhs);
	
	/* Member Functions */
	public:

	/* Member Attributes */
	public:
		Directives		loc_dir;
};

#endif