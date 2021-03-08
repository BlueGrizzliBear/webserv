#ifndef LOCATIONBLOC_HPP
#define LOCATIONBLOC_HPP

#include "./webserv.hpp"

/* LocationBloc Class Declaration */
class LocationBloc
{
	/* Member Types */
	typedef std::map<std::vector<std::string>, std::vector<std::string> >	Directives;

	/* Constructor */
	public:
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
		Directives	loc_dir;
};

#endif