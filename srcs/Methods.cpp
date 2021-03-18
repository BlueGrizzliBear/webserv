#include "./ServerBloc.hpp"

/* Member Functions */
void	ServerBloc::_applyGet()
{
	_findLocation();
	resp.header_fields.insert(std::make_pair("Transfer-Encoding", "identity"));
}


void	ServerBloc::_applyHead()
{
	_applyGet();
	resp.body.clear();
}


void	ServerBloc::_findLocation()
{
	std::string	path;

	if (dir.find("root") != dir.end())
		path = dir.find("root")->second[0];
	for (std::map<std::vector<std::string>, LocationBloc>::iterator loc_it = loc.begin(); loc_it != loc.end(); ++loc_it)
	{
		if (loc_it->first[0] == _uriFirstPart())
		{
			if (loc_it->second.loc_dir.find("root") != loc_it->second.loc_dir.end())
				path = loc_it->second.loc_dir.find("root")->second[0];
		}
	}
	path += req.uri;
	COUT << "file path:" <<  path << ENDL;
	if (_fileExist(path) == false)
		throw NotFound();
	_fillBody(path);
}

bool	ServerBloc::_fileExist(const std::string& name)
{
	std::ifstream	f(name.c_str());

	return f.good();
}

void	ServerBloc::_fillBody(std::string const &path)
{
	std::ifstream		file(path.c_str());
	std::stringstream	strStream;

	strStream << file.rdbuf();
	resp.body = strStream.str();
}

std::string	ServerBloc::_uriFirstPart()
{
	std::string	uri_path;
	std::string	tmp;

	uri_path += req.uri[0];

	for (unsigned long i = 1; req.uri[i]; ++i)
	{
		if (req.uri[i] == '/')
			return (uri_path + tmp);
		tmp += req.uri[i];
	}
	return uri_path;
}
