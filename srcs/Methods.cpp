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
	/* What the RFC ask to do */
	// _applyGet();
	// resp.body.clear();
	/* What the subject tester want us to do */
	resp.header_fields.insert(std::make_pair("Allow", "GET, PUT"));
	throw MethodNotAllowed();
}

void	ServerBloc::_applyPost()
{
	// if (req.body.empty())
	// {
		// resp.header_fields.insert(std::make_pair("Location", req.uri));
		resp.header_fields.insert(std::make_pair("Allow", "GET, PUT"));
		throw MethodNotAllowed();
	// }
}

void	ServerBloc::_findPath(void)
{
	std::string		req_uri = req.uri;

	/* take default root directory in server bloc */
	if (dir.find("root") != dir.end())
		_path = "." + dir.find("root")->second[0];
	for (std::map<std::vector<std::string>, LocationBloc>::iterator it = loc.begin(); it != loc.end(); ++it)
	{
		// COUT << "it->first:" << it->first[0] << ", uriFirstPart:" << _uriFirstPart() << ENDL;
		if (it->first[0] == _uriFirstPart())
		{
			/* take root directory in location block if exist and ingnor the one in server bloc */
			if (it->second.loc_dir.find("root") != it->second.loc_dir.end())
				_path = "." + it->second.loc_dir.find("root")->second[0];
			/* If rewrite replace the location diretory with rewrite directory */
			if (it->second.loc_dir.find("rewrite") != it->second.loc_dir.end())
			{
				req_uri = it->second.loc_dir.find("rewrite")->second[0];
				if (_path.back() == '/')
					req_uri.erase(0, 1);	// remove front '/'
				req_uri += _uriWithoutFirstPart();
			}
		}
	}
	_path += req_uri;
	if (dir.find("autoindex") != dir.end() && dir.find("autoindex")->second[0] == "off")
	{
		// if ((_path.back()) == '/')
			// check if directory exist
			// list directory in html format
			// _path = path to html listing directory
	}
	else
	{
		if ((_path.back()) == '/')
			_findIndex();
		if ((_path.back()) == '/')
			throw Forbidden();
		// std::cout << "path:" << _path << "|" << ENDL;
		if (_fileExist(_path) == false)
			throw NotFound();
	}
}

void		ServerBloc::_checkContentType()
{
	if (_path.back() == '/')
		return ;

	std::string							path_ext = _pathExtension(_path);
	std::map<std::string, std::string>	mime = _parent->getDictionary().mimeDic;
	std::string							contentType = mime.find(path_ext)->second;
	// COUT << "content Type:|" << contentType << "|" << ENDL;

	/* check if content-type exist in the request content-type */
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
	return ("txt");
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

std::string	ServerBloc::_uriWithoutFirstPart()
{
	std::string		uri_path;
	unsigned long	i = 1;

	while (req.uri[i] && req.uri[i] != '/')
		++i;
	++i;
	while (req.uri[i])
	{
		uri_path += req.uri[i];
		++i;
	}
	uri_path += req.uri[i];
	return uri_path;
}
