#include "./ServerBloc.hpp"

/* Member Functions */
void	ServerBloc::_applyGet(void)
{
	for (std::map<std::string, std::vector<std::string> >::iterator it = dir.begin(); it != dir.end(); ++it)
	{
		if (it->first == "root")
		{
			COUT << "here\n";
		}
	}
	// iter on location
	for (std::map<std::vector<std::string>, LocationBloc>::iterator it = loc.begin(); it != loc.end(); ++it)
	{
		if (it->first[0] == req.uri)
			// concatener le uri avec le root et verifier si fichier existe
			COUT << "found it" << ENDL;
	}
	// si req.uri pas trouvé dans location, on prend le premier location avec first[1]=="{"
		//et concatener le uri avec le root et verifier si fichier existe
}
