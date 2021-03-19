#include "./ServerBloc.hpp"

/* Member Functions */
void	ServerBloc::_applyGet()
{
	/* Check if file exist on server */
	_findPath();
	/* Check access rights to access */
		// A FAIRE
	/* Check if server knows file type */
	_checkContentType();
	/* Fill body with file content */
	_fillBody();

	/* if req header is If-Modified-Since, respond with 200 if modified file after the date or respond 304 with empty body */
	if (req.headers.find("If-Modified-Since") != req.headers.end())
		std::string date = req.headers.find("If-Modified-Since")->second;

	// resp.header_fields.insert(std::make_pair("Transfer-Encoding", "identity"));
}

void	ServerBloc::_applyHead()
{
	_applyGet();
	resp.body.clear();
	// POUR PASSER LE TESTEUR
	// resp.header_fields.insert(std::make_pair("Allow", "GET, HEAD"));
	// throw MethodNotAllowed();
}

void	ServerBloc::_applyPost()
{
	// if (req.body.empty())
	// {
		// resp.header_fields.insert(std::make_pair("Location", req.uri));
		resp.header_fields.insert(std::make_pair("Allow", "GET, HEAD"));
		throw MethodNotAllowed();
	// }
}

void	ServerBloc::_findPath(void)
{
	if (dir.find("root") != dir.end())
		_path = "." + dir.find("root")->second[0];
	for (std::map<std::vector<std::string>, LocationBloc>::iterator it = loc.begin(); it != loc.end(); ++it)
	{
		// COUT << "it->first:" << it->first[0] << ", uriFirstPart:" << _uriFirstPart() << ENDL;
		if (it->first[0] == _uriFirstPart())
		{
			if (it->second.loc_dir.find("root") != it->second.loc_dir.end())
				_path = "." + it->second.loc_dir.find("root")->second[0];
			// REDIRECTION A FAIRE pour location /directory/ pointant sur un autre dossier
			// if (it->second.loc_dir.find("root") != it->second.loc_dir.end())
		}
	}
	_path += req.uri;
	if ((_path.back()) == '/')
		_findIndex();
	if (_fileExist(_path) == false)
		throw NotFound();
}

void		ServerBloc::_checkContentType()
{
	std::string	contentType = _parent->getDictionary().mimeDic.find(_pathExtension(_path))->second;

	/* check if content-type exist */
	if (req.headers.find("Content-Type") != req.headers.end())
	{
		if (req.headers.find("Content-Type")->second != contentType)
			throw UnsupportedMediaType();
	}
	resp.header_fields.insert(std::make_pair("Content-Type", contentType));
}

std::string	ServerBloc::_pathExtension(const std::string& path)
{
	std::string							ext;
	std::string::const_reverse_iterator	it = path.rbegin();

	while (it != path.rend())
	{
		if (*it == '.')
		{
			while(--it != path.rbegin())
				ext += *it;
			ext += *it;
			return ext;
		}
		++it;
	}
	return ("plain/text");
}

void	ServerBloc::_findIndex()
{
	std::vector<std::string>	index;

	if (dir.find("index") != dir.end())
	{
		index = dir.find("index")->second;
		for (std::vector<std::string>::iterator it = index.begin(); it != index.end(); ++it)
		{
			if (_fileExist(_path + *it) == true)
			{
				_path += *it;
				return ;
			}
		}
	}
}

bool	ServerBloc::_fileExist(const std::string& name)
{
	std::ifstream	f(name.c_str());

	if (f.good())
	{
		f.close();
		return true;
	}
	f.close();
	return false;
}

void	ServerBloc::_fillBody()
{
	std::ifstream		file(_path.c_str());
	std::stringstream	strStream;

	strStream << file.rdbuf();
	resp.body = strStream.str();
	file.close();
}

std::string	ServerBloc::_uriFirstPart()
{
	std::string	uri_path;
	std::string	tmp;

	uri_path += req.uri[0];

	for (unsigned long i = 1; req.uri[i]; ++i)
	{
		if (req.uri[i] == '/')
		{
			tmp += req.uri[i];
			return (uri_path + tmp);
		}
		tmp += req.uri[i];
	}
	return uri_path;
}
