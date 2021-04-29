#include "./LocationBloc.hpp"

/* LocationBloc Class Declaration */
/* Constructor */
/*	default		(1)	*/
LocationBloc::LocationBloc(void) {}

/*	copy		(2)	*/
LocationBloc::LocationBloc(LocationBloc const & cpy)
{
	*this = cpy;
}

/* Destructor */
LocationBloc::~LocationBloc() {}

/* Operators */
LocationBloc &	LocationBloc::operator=(LocationBloc const & rhs)
{
	loc_dir = rhs.loc_dir;
	return (*this);
}
