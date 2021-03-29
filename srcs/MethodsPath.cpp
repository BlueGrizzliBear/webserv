#include "./Methods.hpp"

/* Find path (_path) */
	/* (1) find server config */
void	Methods::_findPath(void)
{
	std::vector<std::string>	methods;
	// std::vector<std::string>	error_pages;
	std::map<std::string, std::vector<std::string> >	locationDir;
	std::string					req_uri = serv->req.uri;

	_autoindex = true;
	/* finding all default server conf */
	_findRoot(serv->dir);
	_findAutoIndex(serv->dir);
	methods = _findVect(serv->dir, "allowed_methods", methods);
	_indexes = _findVect(serv->dir, "index", _indexes);
	// error_pages = _findVect(serv->dir, "error_pages", error_pages);

	/* iterating location bloc */
	for (std::map<std::vector<std::string>, LocationBloc>::iterator it = serv->loc.begin(); it != serv->loc.end(); ++it)
	{
		if (_matchingLocationDir(it, &locationDir))
			break;
	}
	/* if location bloc found applying corresponding config */
	if (locationDir.empty() == false)
	{
		_findRoot(locationDir);
		_findAutoIndex(locationDir);
		methods = _findVect(locationDir, "allowed_methods", methods);
		_indexes = _findVect(locationDir, "index", _indexes);
		// error_pages = _findVect(locationDir, "error_pages", error_pages);
		req_uri = _findRewrite(locationDir);
	}
	/* return exeption if method not allowed */
	_checkAllowedMethods(methods);
	_path += req_uri;
	// COUT << "_path:" << _path << "|" << ENDL;
}

template< typename T, typename U >
void		Methods::_findRoot(std::map< T, U > dir)
{
	if (dir.find("root") != dir.end())
		_path = ("." + dir.find("root")->second[0]);
}

template< typename T, typename U >
void		Methods::_findAutoIndex(std::map< T, U > dir)
{
	if (dir.find("autoindex") != dir.end() && dir.find("autoindex")->second[0] == "off")
		_autoindex = false;
}

template< typename T, typename U >
std::vector<std::string>	Methods::_findVect(std::map< T, U > dir, std::string to_find, std::vector<std::string> vect)
{
	if (dir.find(to_find) != dir.end())
		return (dir.find(to_find)->second);
	return vect;
}


template< typename T, typename U >
std::string	Methods::_findRewrite(std::map< T, U > dir)
{
	std::string		req_uri = serv->req.uri;

	if (dir.find("rewrite") != dir.end())
	{
		req_uri = dir.find("rewrite")->second[0];
		if (!_path.empty() && *(_path.rbegin()) == '/')
			req_uri.erase(0, 1);	// remove front '/'
		req_uri += _uriWithoutFirstPart();
	}
	return req_uri;
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

	/* (2) find location bloc config */
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
	}
	else
	{
		if (it->first[0] == _uriFirstPart())
		{
			*location_dir = it->second.loc_dir;
			return false;
		}
	}
	return false;
}

bool	Methods::_isRegex(std::string str)
{
	if (str == "=" || str == "^~" || str == "~" || str == "~*" || str == "@")
		return true;
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
	if (!cap_grp.empty() && *(cap_grp.rbegin()) == '$' && _compareFromEnd(uri_path, path_set))
		return true;
	else if (_compareFromBegin(uri_path, path_set))
		return true;
	return false;
}

bool	Methods::_compareFromEnd(std::string uri_path, std::vector<std::string> path_set)
{
	for (std::vector<std::string>::iterator it = path_set.begin(); it != path_set.end(); ++it)
	{
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

std::string		Methods::_toLowerStr(std::string const &str)
{
	std::string	ret;

	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
		ret.append(1, tolower(*it));
	return ret;
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
	if (!tmp.empty() && *(tmp.rbegin()) != '/')
		return  (uri_path + tmp + '/');
	return (uri_path);
}

	/* (3) allowed method */
void	Methods::_checkAllowedMethods(std::vector<std::string> methods)
{
	std::string	cat_meth;

	if (!methods.empty()) /* if not empty */
	{
		for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); ++it)
		{
			if (serv->req.method == *it)
				return ;
			if (*it != *(methods.rbegin()))
				cat_meth += *it + ", ";
			else
				cat_meth += *it;
		}
		serv->resp.header_fields.insert(std::make_pair("Allow", cat_meth));
		throw ServerBloc::MethodNotAllowed();
	}
}
