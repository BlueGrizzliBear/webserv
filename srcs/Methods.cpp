#include "./Methods.hpp"

/* Methods Class Declaration */
/* Constructor */
/*	default		(1)	*/
Methods::Methods(void) : serv(NULL) {}

/*	default		(1)	*/
Methods::Methods(ServerBloc & server) : serv(&server) {}

/*	copy		(2)	*/
Methods::Methods(Methods const & cpy)
{
	*this = cpy;
}

/* Destructor */
Methods::~Methods() {}

/* Operators */
Methods &	Methods::operator=(Methods const & rhs)
{
	serv = rhs.serv;
	return (*this);
}

/* Member Functions */
void	Methods::execute(void)
{
	if (serv->req.method == "GET")
		_applyGet();
	else if (serv->req.method == "HEAD")
		_applyHead();
	else if (serv->req.method == "POST")
	{
		_applyPost();
		// throw BadRequest();
		// throw UnsupportedMediaType();
		// throw Unauthorized();
	}
}

void	Methods::_applyGet()
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
	if (serv->req.headers.find("If-Modified-Since") != serv->req.headers.end())
		std::string date = serv->req.headers.find("If-Modified-Since")->second;

	serv->resp.header_fields.insert(std::make_pair("Transfer-Encoding", "identity"));
}

void	Methods::_applyHead()
{
	_applyGet();
	serv->resp.body.clear();
}

void	Methods::_applyPost()
{
	_findPath();
}

template< typename T, typename U >
std::string	Methods::_findRoot(std::map< T, U > dir)
{
	if (dir.find("root") != dir.end())
		return ("." + dir.find("root")->second[0]);
	return (_path);
}

template< typename T, typename U >
bool		Methods::_findAutoIndex(std::map< T, U > dir, bool autoindex)
{
	if (dir.find("autoindex") != dir.end() && dir.find("autoindex")->second[0] == "off")
		return (false);
	return (autoindex);
}

template< typename T, typename U >
std::vector<std::string>	Methods::_findVect(std::map< T, U > dir, std::string to_find, std::vector<std::string> vect)
{
	if (dir.find(to_find) != dir.end())
		return (dir.find(to_find)->second);
	return vect;
}

bool	Methods::_isRegex(std::string str)
{
	if (str == "=" || str == "^~" || str == "~" || str == "~*" || str == "@")
		return true;
	return false;
}

bool	Methods::_compareFromEnd(std::string uri_path, std::vector<std::string> path_set)
{
	for (std::vector<std::string>::iterator it = path_set.begin(); it != path_set.end(); ++it)
	{
		// COUT << "uri_path:" << uri_path << ", cap_grp:" << *it << ENDL;
		std::string::reverse_iterator it_str = (*it).rbegin();
		std::string::reverse_iterator it_uri = uri_path.rbegin();
		while (it_str != (*it).rend() && it_uri != uri_path.rend() && *it_str == *it_uri)
		{
			++it_str;
			++it_uri;
		}
		if (it_str == (*it).rend())
			return true;
	}
	return false;
}

bool	Methods::_compareFromBegin(std::string uri_path, std::vector<std::string> path_set)
{
	for (std::vector<std::string>::iterator it = path_set.begin(); it != path_set.end(); ++it)
	{
		std::string::iterator it_str = (*it).begin();
		std::string::iterator it_uri = uri_path.begin();
		while (it_str != (*it).end() && it_uri != uri_path.end() && *it_str == *it_uri)
		{
			++it_str;
			++it_uri;
		}
		if (it_str == (*it).end())
			return true;
	}
	return false;
}

bool			Methods::_compareCapturingGroup(std::string uri_path, std::string cap_grp)
{
	std::string::iterator		it_cap = cap_grp.begin();
	std::vector<std::string>	path_set;
	std::string					tmp;
	unsigned int				i = 0;
	std::string					first_part;

	path_set.push_back(tmp);
	while (it_cap != cap_grp.end() && *it_cap != '(')
	{
		for (std::vector<std::string>::iterator it = path_set.begin(); it != path_set.end(); ++it)
			*it += *it_cap;
		++it_cap;
	}
	first_part = path_set[0];
	if (it_cap != cap_grp.end())
		++it_cap;
	while (it_cap != cap_grp.end() && *it_cap != ')')
	{
		while (it_cap != cap_grp.end() && *it_cap != '|' && *it_cap != ')')
		{
			tmp += *it_cap;
			++it_cap;
		}
		if (i == 0)
			path_set[i] = first_part + tmp;
		else
			path_set.push_back(first_part + tmp);
		++i;
		tmp.clear();
		if (*it_cap == '|')
			++it_cap;
	}
	while (it_cap != cap_grp.end() && *it_cap != ')')
	{
		for (std::vector<std::string>::iterator it = path_set.begin(); it != path_set.end(); ++it)
			*it += *it_cap;
		++it_cap;
	}
	if (cap_grp.back() == '$' && _compareFromEnd(uri_path, path_set))
		return true;
	else if (_compareFromBegin(uri_path, path_set))
		return true;
	return false;
}

std::string		Methods::_toLowerStr(std::string const &str)
{
	std::string	ret;

	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
		ret.append(1, tolower(*it));
	return ret;
}

void	Methods::_matchingLocationDir(std::map<std::vector<std::string>, LocationBloc>::iterator it, bool *break_loc, std::map<std::string, std::vector<std::string> > *location_dir)
{
	if (_isRegex(it->first[0]))
	{
		*break_loc = true;
		if (it->first[0] == "=" && (serv->req.uri == it->first[1]))
			*location_dir = it->second.loc_dir;
		else if (it->first[0] == "^~" && _compareCapturingGroup(serv->req.uri, it->first[1]))
			*location_dir = it->second.loc_dir;
		else if (it->first[0] == "~" && _compareCapturingGroup(serv->req.uri, it->first[1]))
			*location_dir = it->second.loc_dir;
		else if (it->first[0] == "~*" && _compareCapturingGroup(_toLowerStr(serv->req.uri), _toLowerStr(it->first[1])))
			*location_dir = it->second.loc_dir;
		else
			*break_loc = false;
	}
	else
	{
		if (it->first[0] == _uriFirstPart())
		{
			// COUT << "match found:" << it->first[0] << ENDL;
			*location_dir = it->second.loc_dir;
		}
	}
	*break_loc = false;
}

template< typename T, typename U >
std::string	Methods::_findRewrite(std::map< T, U > dir)
{
	std::string		req_uri = serv->req.uri;

	if (dir.find("rewrite") != dir.end())
	{
		req_uri = dir.find("rewrite")->second[0];
		if (_path.back() == '/')
			req_uri.erase(0, 1);	// remove front '/'
		req_uri += _uriWithoutFirstPart();
	}
	return req_uri;
}

void	Methods::_checkAllowedMethods(std::vector<std::string> methods)
{
	std::string	cat_meth;

	if (!methods.empty()) /* if not empty */
	{
		// COUT << "methods not empty false" << ENDL;
		for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); ++it)
		{
			if (serv->req.method == *it)
				return ;
			if (*it != methods.back())
				cat_meth += *it + ", ";
			else
				cat_meth += *it;
		}
		serv->resp.header_fields.insert(std::make_pair("Allow", cat_meth));
		COUT << "here\n";
		throw ServerBloc::MethodNotAllowed();
	}
}

void	Methods::_findPath(void)
{
	bool						autoindex = true;
	bool						break_loc = false;
	std::vector<std::string>	methods;
	std::vector<std::string>	indexes;
	std::vector<std::string>	error_pages;
	std::map<std::string, std::vector<std::string> >	locationDir;
	std::string					req_uri = serv->req.uri;

	/* finding all default server conf */
	_path = _findRoot(serv->dir);
	// COUT << "find _path:" << _path << ENDL;
	autoindex = _findAutoIndex(serv->dir, autoindex);
	// COUT << "find autoindex:" << autoindex << ENDL;
	methods = _findVect(serv->dir, "allowed_methods", methods);
	// COUT << "find methods ok:" << ENDL;
	indexes = _findVect(serv->dir, "index", indexes);
	// COUT << "find indexes ok:" << ENDL;
	error_pages = _findVect(serv->dir, "error_pages", error_pages);
	// COUT << "find error_pages ok:" << ENDL;

	/* iterating location bloc */
	for (std::map<std::vector<std::string>, LocationBloc>::iterator it = serv->loc.begin(); it != serv->loc.end(); ++it)
	{
		// COUT << "loc dir:" << (*it).first[0] << ENDL;
		_matchingLocationDir(it, &break_loc, &locationDir);
		if (break_loc)
			break;
	}
	// COUT << "_path before locationDir is empty:" << _path << "|" << ENDL;
	if (locationDir.empty() == false)
	{
		_path = _findRoot(locationDir);
		autoindex = _findAutoIndex(locationDir, autoindex);
		methods = _findVect(locationDir, "allowed_methods", methods);
		indexes = _findVect(locationDir, "index", indexes);
		error_pages = _findVect(locationDir, "error_pages", error_pages);
		req_uri = _findRewrite(locationDir);
	}
	// COUT << "_path before check allowed method:" << _path << "|" << ENDL;
	_checkAllowedMethods(methods);
	_path += req_uri;
	// COUT << "_path:" << _path << "|" << ENDL;
	if (_path.back() != '/' && _isDirectory(_path) == true)
		_path.append("/");
	if (autoindex == false)
	{
		if ((_path.back()) == '/')
			COUT << "List directory" << ENDL;
	}
	else
	{
		if ((_path.back()) == '/')
			_findIndex(indexes);
		// COUT << "after index path:" << _path << "|" << ENDL;
		if ((_path.back()) == '/')
			throw ServerBloc::Forbidden();
		if (_fileExist(_path) == false)
			throw ServerBloc::NotFound();
	}
}

// void	Methods::_findPath(void)
// {
// 	std::string		req_uri = req.uri;

// 	/* take default root directory in server bloc */
// 	if (dir.find("root") != dir.end())
// 		_path = "." + dir.find("root")->second[0];
// 	/* Iter on location bloc from first to last one */
// 	for (std::map<std::vector<std::string>, LocationBloc>::iterator it = loc.begin(); it != loc.end(); ++it)
// 	{
// 		COUT << "it->first:" << it->first[0] << ", uriFirstPart:" << _uriFirstPart() << ENDL;
// 		/* If a location match the uri */
// 		if (it->first[0] == _uriFirstPart())
// 		{
// 			/* If allowed methods condition one location bloc */
// 			if (it->second.loc_dir.find("allowed_methods") != it->second.loc_dir.end())
// 			{
// 				if (req.method != it->second.loc_dir.find("allowed_methods")->second[0])
// 				{
// 					resp.header_fields.insert(std::make_pair("Allow", it->second.loc_dir.find("allowed_methods")->second[0]));
// 					throw MethodNotAllowed();
// 				}
// 			}
// 			// COUT << "it->first[0] == _uriFirstPart()" << ENDL;
// 			/* take root directory in location block if exist and ingnor the one in server bloc */
// 			if (it->second.loc_dir.find("root") != it->second.loc_dir.end())
// 				_path = "." + it->second.loc_dir.find("root")->second[0];
// 			COUT << "_path:|" << _path << "|" << ENDL;
// 			/* If rewrite replace the location diretory with rewrite directory */
// 			if (it->second.loc_dir.find("rewrite") != it->second.loc_dir.end())
// 			{
// 				// COUT << "req_uri:|" << req_uri << "|" << ENDL;
// 				req_uri = it->second.loc_dir.find("rewrite")->second[0];
// 				if (_path.back() == '/')
// 					req_uri.erase(0, 1);	// remove front '/'
// 				COUT << "req_uri:|" << req_uri << "|" << ENDL;
// 				req_uri += _uriWithoutFirstPart();
// 				COUT << "req_uri after without first part:|" << req_uri << "|" << ENDL;
// 			}
// 		}
// 	}
// 	COUT << "before _path:" << _path << "|" << ENDL;
// 	_path += req_uri;
// 	COUT << "after _path:" << _path << "|" << ENDL;
// 	if (_path.back() != '/' && _isDirectory(_path) == true)
// 		_path.append("/");
// 	if (dir.find("autoindex") != dir.end() && dir.find("autoindex")->second[0] == "off")
// 	{
// 		if ((_path.back()) == '/')
// 			COUT << "List directory" << ENDL;
// 			// _createIndexHTML();
// 			// check if directory exist
// 			// list directory in html format
// 			// _path = path to html listing directory
// 	}
// 	else
// 	{
// 		if ((_path.back()) == '/')
// 			_findIndex();
// 		COUT << "after index path:" << _path << "|" << ENDL;
// 		if ((_path.back()) == '/')
// 			throw Forbidden();
// 		if (_fileExist(_path) == false)
// 			throw NotFound();
// 	}
// }

bool	Methods::_isDirectory(std::string const & path)
{
	DIR	*	dir = NULL;

	if ((dir = opendir(path.c_str())))
	{
		closedir(dir);
		return true;
	}
	else
		return false;
}

// void		Methods::_createIndexHTML()
// {
// 	DIR				*dir;
// 	struct dirent	*list;

// 	if ((dir = opendir(_path.c_str())))
// 	{
// 		list = readdir(dir);

// 	}
// 	else
// 		throw NotFound();
// }

void	Methods::_checkContentType(void)
{
	/* If directory do nothing */
	if (_path.back() == '/')
		return ;
	/* Find correct content-type matching file extension */
	std::string	contentType;
	std::string	pathExt = _pathExtension(_path);

	if (serv->getParent()->getDictionary().mimeDic.find(pathExt) != serv->getParent()->getDictionary().mimeDic.end())
		contentType = serv->getParent()->getDictionary().mimeDic.find(pathExt)->second;
	else
		contentType = serv->getParent()->getDictionary().mimeDic.find("txt")->second;
	/* check if content-type exist if the request content-type */
	if (serv->req.headers.find("Content-Type") != serv->req.headers.end())
	{
		if (serv->req.headers.find("Content-Type")->second != contentType)
			throw ServerBloc::UnsupportedMediaType();
	}
	serv->resp.header_fields.insert(std::make_pair("Content-Type", contentType));
}

std::string	Methods::_pathExtension(const std::string & path)
{
	std::string	ext;
	std::string::const_reverse_iterator	it = path.rbegin();

	while (it != path.rend())
	{
		if (*it == '.')
		{
			while(--it != path.rbegin())
				ext += *it;
			ext += *it;
			return (ext);
		}
		++it;
	}
	return ("txt");
}

void	Methods::_findIndex(std::vector<std::string> indexes)
{
	for (std::vector<std::string>::iterator it = indexes.begin(); it != indexes.end(); ++it)
	{
		if (_fileExist(_path + *it) == true)
		{
			_path += *it;
			return ;
		}
	}
	_path += *(indexes.begin());
}

bool	Methods::_fileExist(const std::string & name)
{
	int	fd;

	if ((fd = open(name.c_str(), O_WRONLY)) < 1)
		return false;
	else
	{
		close(fd);
		return true;
	}
}

void	Methods::_fillBody()
{
	std::ifstream		file(_path.c_str());
	std::stringstream	strStream;

	strStream << file.rdbuf();
	serv->resp.body = strStream.str();
	file.close();
}

std::string	Methods::_uriFirstPart()
{
	std::string	uri_path;
	std::string	tmp;

	uri_path = serv->req.uri[0];

	for (unsigned long i = 1; serv->req.uri[i]; ++i)
	{
		if (serv->req.uri[i] == '/')
		{
			tmp += serv->req.uri[i];
			return (uri_path + tmp);
		}
		tmp += serv->req.uri[i];
	}
	if (tmp.empty() == false && tmp.back() != '/')
		return  (uri_path + tmp + '/');
	return (uri_path);
}

std::string	Methods::_uriWithoutFirstPart()
{
	std::string		uri_path;
	unsigned long	i = 1;

	while (serv->req.uri[i] && serv->req.uri[i] != '/')
		++i;
	++i;
	while (serv->req.uri[i])
	{
		uri_path += serv->req.uri[i];
		++i;
	}
	return (uri_path);
}
