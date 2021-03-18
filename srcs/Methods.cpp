#include "./ServerBloc.hpp"

/* Member Functions */
void	ServerBloc::_applyGet()
{
	_findLocation();
	// resp.header_fields.insert(std::make_pair("Transfer-Encoding", "identity"));
}


void	ServerBloc::_applyHead()
{
	_applyGet();
	resp.body.clear();
}


void	ServerBloc::_findLocation(void) throw (NotFound)
{
	std::string					path;

	if (dir.find("root") != dir.end())
		path = "." + dir.find("root")->second[0];
	for (std::map<std::vector<std::string>, LocationBloc>::iterator it = loc.begin(); it != loc.end(); ++it)
	{
		if (it->first[0] == _uriFirstPart())
		{
			if (it->second.loc_dir.find("root") != it->second.loc_dir.end())
				path = "." + it->second.loc_dir.find("root")->second[0];
		}
	}
	path += req.uri;
	if ((path.back()) == '/')
		path = _findIndex(path);
	// COUT << "file path:" << path << ENDL;
	if (_fileExist(path) == false)
		throw NotFound();
	resp.header_fields.insert(std::make_pair("Content-Type", _mimeType(path)));
	_fillBody(path);
}

std::string	ServerBloc::_mimeType(const std::string& )
{
	return ("text/html");
}

std::string	ServerBloc::_findIndex(const std::string& path)
{
	std::vector<std::string>	index;
	// std::string					new_path;

	if (dir.find("index") != dir.end())
	{
		index = dir.find("index")->second;
		for (std::vector<std::string>::iterator it = index.begin(); it != index.end(); ++it)
		{
			// new_path = path + *it;
			if (_fileExist(path + *it) == true)
				return (path + *it);
		}
	}
	return path;
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
