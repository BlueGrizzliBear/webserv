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
	/* uri resolution process (treat ../ and ./ if exist */
	_URIResolutionProcess();

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
	// else if (serv->req.method == "PUT")

		// _applyPut();
}

void	Methods::_URIResolutionProcess(void)
{
	size_t pos = 0;
	std::string tmp;
	std::string new_uri;

	while (!serv->req.uri.empty())
	{

		/* 1st */
		if (!serv->req.uri.find(tmp = "../") || !serv->req.uri.find(tmp = "./"))
			serv->req.uri.erase(0, tmp.size());
		/* 3rd */
		else if (!serv->req.uri.find(tmp = "/../") || !serv->req.uri.find(tmp = "/.."))
		{
			serv->req.uri.replace(0, tmp.size(), "/");
			if ((pos = new_uri.rfind("/")) != std::string::npos)
				new_uri.erase(pos, pos - new_uri.size());
		}
		/* 2nd */
		else if (!serv->req.uri.find(tmp = "/./") || !serv->req.uri.find(tmp = "/."))
			serv->req.uri.replace(0, tmp.size(), "/");
		/* 4th */
		else if (!serv->req.uri.find(tmp = ".") || !serv->req.uri.find(tmp = ".."))
			serv->req.uri.erase(0, tmp.size());
		/* 5th */
		else
		{
			size_t i = (serv->req.uri.find("/") == 0) ? 1 : 0;
			if ((pos = serv->req.uri.find("/", i)) == std::string::npos)
				pos = serv->req.uri.size();
			new_uri += serv->req.uri.substr(0, pos);
			serv->req.uri.erase(0, pos);
		}
	}
	serv->req.uri = new_uri;
}

void	Methods::_applyGet(void)
{
	/* Check if file exist on server */
	_findPath();
	/* execute specific to GET request */
	_executeGetReq();
	/* Check if server knows file type */
	_checkContentType();
	/* Fill body with file content */
	_fillBody();

	/* if req header is If-Modified-Since, respond with 200 if modified file after the date or respond 304 with empty body */
	if (serv->req.headers.find("If-Modified-Since") != serv->req.headers.end())
		std::string date = serv->req.headers.find("If-Modified-Since")->second;

	serv->resp.header_fields.insert(std::make_pair("Transfer-Encoding", "identity"));
}

void	Methods::_applyHead(void)
{
	_applyGet();
	serv->resp.body.clear();
}

void	Methods::_applyPost()
{
	_findPath();
}

void	Methods::_applyPut()
{
// If the target resource does not have a current representation and the
//    PUT successfully creates one, then the origin server MUST inform the
//    user agent by sending a 201 (Created) response.  If the target
//    resource does have a current representation and that representation
//    is successfully modified in accordance with the state of the enclosed
//    representation, then the origin server MUST send either a 200 (OK) or
//    a 204 (No Content) response to indicate successful completion of the
//    request.

// An origin server that allows PUT on a given target resource MUST send
//    a 400 (Bad Request) response to a PUT request that contains a
//    Content-Range header field

// An origin server MUST NOT send a validator header field
//    (Section 7.2), such as an ETag or Last-Modified field, in a
//    successful response to PUT unless the request's representation data
//    was saved without any transformation applied to the body (i.e., the
//    resource's new representation data is identical to the representation
//    data received in the PUT request) and the validator field value
//    reflects the new representation.

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

bool	Methods::_matchingLocationDir(std::map<std::vector<std::string>, LocationBloc>::iterator it, std::map<std::string, std::vector<std::string> > *location_dir)
{
	if (_isRegex(it->first[0]))
	{
		if (it->first[0] == "=" && (serv->req.uri == it->first[1]))
		{
			*location_dir = it->second.loc_dir;
			return true;
		}
		else if (it->first[0] == "^~" && _compareCapturingGroup(serv->req.uri, it->first[1]))
		{
			*location_dir = it->second.loc_dir;
			return true;
		}
		else if (it->first[0] == "~" && _compareCapturingGroup(serv->req.uri, it->first[1]))
		{
			*location_dir = it->second.loc_dir;
			return true;
		}
		else if (it->first[0] == "~*" && _compareCapturingGroup(_toLowerStr(serv->req.uri), _toLowerStr(it->first[1])))
		{
			*location_dir = it->second.loc_dir;
			return true;
		}
		// else
		// 	return false;
	}
	else
	{
		if (it->first[0] == _uriFirstPart())
		{
			// COUT << "match found:" << it->first[0] << ENDL;
			*location_dir = it->second.loc_dir;
			return false;
		}
	}
	return false;
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
	std::vector<std::string>	methods;
	// std::vector<std::string>	indexes;
	std::vector<std::string>	error_pages;
	std::map<std::string, std::vector<std::string> >	locationDir;
	std::string					req_uri = serv->req.uri;

	_autoindex = true;
	/* finding all default server conf */
	_path = _findRoot(serv->dir);
	_autoindex = _findAutoIndex(serv->dir, _autoindex);
	methods = _findVect(serv->dir, "allowed_methods", methods);
	_indexes = _findVect(serv->dir, "index", _indexes);
	error_pages = _findVect(serv->dir, "error_pages", error_pages);

	/* iterating location bloc */
	for (std::map<std::vector<std::string>, LocationBloc>::iterator it = serv->loc.begin(); it != serv->loc.end(); ++it)
	{
		if (_matchingLocationDir(it, &locationDir))
			break;
	}
	// COUT << "_path before locationDir is empty:" << _path << "|" << ENDL;
	if (locationDir.empty() == false)
	{
		_path = _findRoot(locationDir);
		_autoindex = _findAutoIndex(locationDir, _autoindex);
		methods = _findVect(locationDir, "allowed_methods", methods);
		_indexes = _findVect(locationDir, "index", _indexes);
		error_pages = _findVect(locationDir, "error_pages", error_pages);
		req_uri = _findRewrite(locationDir);
	}
	_checkAllowedMethods(methods);
	_path += req_uri;
	COUT << "_path:" << _path << "|" << ENDL;
}

void	Methods::_executeGetReq(void)
{
	if (_path.back() != '/' && _isDirectory(_path) == true)
		_path.append("/");
	/* create html list directory if autoindex off and copy to body */
	if (_autoindex == false)
	{
		if ((_path.back()) == '/')
			// COUT << "List directory" << ENDL;
			_createIndexHTML();
	}
	else	/* copy asked file to body if exist */
	{
		if ((_path.back()) == '/')
			_findIndex(_indexes);
		// COUT << "after index path:" << _path << "|" << ENDL;
		if ((_path.back()) == '/')
			throw ServerBloc::Forbidden();
		if (_fileExist(_path) == false)
			throw ServerBloc::NotFound();
	}
}

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

void		Methods::_createIndexHTML()
{
	DIR				*dir;

	/* check if directory exist */
	if ((dir = opendir(_path.c_str())))
	{
		/* list directory in html format*/
		_createHTMLListing(dir);
		_path = "./dir_listing.html";
	}
	else
		throw ServerBloc::NotFound();
}

void	Methods::_createHTMLListing(DIR * dir)
{
	struct dirent	*dp;
	struct stat		info;
	struct tm		*timeinfo;
	char			date[30];
	std::string		file_path;

	std::ofstream	dir_list("./dir_listing.html");

	dir_list << "<html>\n<head><title>Index of /</title></head>\n<body bgcolor=\"white\">\n<h1>Index of " << serv->req.uri << "</h1><hr><pre>\n";
	while ((dp = readdir(dir)) != NULL)
	{
		if (strcmp(dp->d_name, "."))
		{
			std::string		points(30 - strlen(dp->d_name), '.');

			file_path = _path + dp->d_name;
			if (!lstat(file_path.c_str(), &info))
			{
				time(&info.st_mtime);
  				timeinfo = localtime (&info.st_mtime);
				strftime(date, 30, "%d-%b-%Y %H:%M", timeinfo);
				dir_list << "<a href=\"" << serv->req.uri;
				if (serv->req.uri.back() != '/')
					dir_list << '/';
				dir_list << dp->d_name << "\">" << dp->d_name;
				if (dp->d_type == DT_DIR)
					dir_list << '/';
				dir_list << "</a>";
				if (strcmp(dp->d_name, ".."))
					dir_list << points << date << "................" << info.st_size << '\n';
				else
					dir_list << '\n';
			}
			else
			{
				dir_list << "<a href=\"" << serv->req.uri;
				if (serv->req.uri.back() != '/')
					dir_list << '/';
				dir_list << dp->d_name << "\">" << dp->d_name;
				if (dp->d_type == DT_DIR)
					dir_list << '/';
				dir_list << "</a>";
				if (strcmp(dp->d_name, ".."))
					dir_list << points << "unauthorized.." << "................" << "unauthorized" << '\n';
				else
					dir_list << '\n';
			}
		}
	}
	dir_list << "</pre><hr></body>\n</html>";
	dir_list.close();
	closedir(dir);
}

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
	if (_fileExist("./dir_listing.html"))
		std::remove("./dir_listing.html");
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
