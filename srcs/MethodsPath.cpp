#include "./Methods.hpp"

/* Find path (_path) */
	/* (1) find server config */
void	Methods::_findPath(void)
{
	// std::vector<std::string>	methods;
	std::string	req_uri = serv->req.uri;
	// size_t		max_body_size = 1000000; /* 1 GB by default */

	_autoindex = true;
	/* finding all default server conf */
	_findAuthenticate(serv->dir);
	_findRoot(serv->dir);
	_findAutoIndex(serv->dir);
	_findCGIPath(serv->dir);
	_findVect(serv->dir, "allowed_methods", &_methods);
	_findClientMaxBodySize(serv->getParent()->getMainDirs(), &_max_body_size);
	_findVect(serv->dir, "index", &_indexes);

	/* iterating location bloc */
	std::map<std::vector<std::string>, LocationBloc>::iterator tmp = serv->loc.end();

	for (std::map<std::vector<std::string>, LocationBloc>::iterator it = serv->loc.begin(); it != serv->loc.end(); ++it)
	{
		if (_matchingLocationDir(it, tmp))
			break;
	}
	/* if location bloc found applying corresponding config */
	if (tmp != serv->loc.end())
	{
		_findAuthenticate(tmp->second.loc_dir);
		_findRoot(tmp->second.loc_dir);
		_findAutoIndex(tmp->second.loc_dir);
		_findCGIPath(tmp->second.loc_dir);
		_findVect(tmp->second.loc_dir, "allowed_methods", &_methods);
		_findClientMaxBodySize(tmp->second.loc_dir, &_max_body_size);
		_findVect(tmp->second.loc_dir, "index", &_indexes);
		req_uri = _findRewrite(tmp->second.loc_dir);
	}

	_path += req_uri;
	// COUT << "_path:|" << _path << "|" << ENDL;
}

template< typename T, typename U >
void		Methods::_findAuthenticate(std::map< T, U > & dir)
{
	if (dir.find("auth_basic") != dir.end())
	{
		std::vector<std::string>	realm_vect = dir.find("auth_basic")->second;
		std::string					realm;
		std::vector<std::string>::iterator it = realm_vect.begin();

		realm = *it;
		it++;
		for(; it != realm_vect.end(); ++it)
			realm += " " + *it;
		_authenticate.push_back(realm);
		if (dir.find("auth_basic_user_file") != dir.end())
			_authenticate.push_back(("." + dir.find("auth_basic_user_file")->second[0]));
		else
			_authenticate.push_back("./no_file_exist");
	}
}

template< typename T, typename U >
void		Methods::_findRoot(std::map< T, U > & dir)
{
	if (dir.find("root") != dir.end())
		_path = ("." + dir.find("root")->second[0]);
}

template< typename T, typename U >
void		Methods::_findAutoIndex(std::map< T, U > & dir)
{
	if (dir.find("autoindex") != dir.end() && dir.find("autoindex")->second[0] == "off")
		_autoindex = false;
}

template< typename T, typename U >
void		Methods::_findCGIPath(std::map< T, U > & dir)
{
	if (dir.find("cgi") != dir.end())
	{
		_cgi_path = dir.find("cgi")->second[0];
		_cgi_is_php = false;
	}
	if (dir.find("php-cgi") != dir.end())
	{
		_cgi_path = dir.find("php-cgi")->second[0];
		_cgi_is_php = true;
	}
}

template< typename T, typename U >
void	Methods::_findVect(std::map< T, U > & dir, std::string to_find, std::vector<std::string> * vect)
{
	if (dir.find(to_find) != dir.end())
		*vect = dir.find(to_find)->second;
}

template< typename T, typename U >
void	Methods::_findClientMaxBodySize(std::map< T, U > & dir, size_t * max_size)
{
	std::string to_find;

	if (dir.find("client_max_body_size") != dir.end())
	{
		to_find = dir.find("client_max_body_size")->second[0];

		if (!to_find.empty())
		{
			*max_size = static_cast<size_t>(std::atoi(to_find.c_str()));
			if (to_find[to_find.size() - 1] == 'K')
				*max_size *= 1000;
			else if (to_find[to_find.size() - 1] == 'M')
				*max_size *= 1000000;
			else if (to_find[to_find.size() - 1] == 'G')
				*max_size *= 1000000000;
		}
	}
}

template< typename T, typename U >
std::string	Methods::_findRewrite(std::map< T, U > & dir)
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

std::string	Methods::_uriWithoutFirstPart(void)
{
	std::string	uri_path;
	size_t		i = 0;

	if ((i = serv->req.uri.find("/", 1)) == std::string::npos)
		return ("");
	i++;
	uri_path = serv->req.uri.substr(i, serv->req.uri.size() - i);
	return (uri_path);
}

	/* (2) find location bloc config */
bool	Methods::_matchingLocationDir(std::map<std::vector<std::string>, LocationBloc>::iterator & it, std::map<std::vector<std::string>, LocationBloc>::iterator & tmp)
{
	if (it->first[0] == "=" && ((_uriFirstPart() == it->first[1]) || (serv->req.uri == it->first[1])))
	{
		tmp = it;
		return true;
	}
	else if (it->first[0] == "^~" && _compareCapturingGroup(serv->req.uri, it->first[1]))
	{
		tmp = it;
		return true;
	}
	else if (it->first[0] == "~" && _compareCapturingGroup(serv->req.uri, it->first[1]))
	{
		tmp = it;
		return true;
	}
	else if (it->first[0] == "~*" && _compareCapturingGroup(serv->req.transform(serv->req.uri, tolower), serv->req.transform(it->first[1], tolower)))
	{
		tmp = it;
		return true;
	}
	else if (it->first[0] == _uriFirstPart())
		tmp = it;
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

bool	Methods::_compareFromEnd(std::string & uri_path, std::vector<std::string> & path_set)
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

bool	Methods::_compareFromBegin(std::string & uri_path, std::vector<std::string> & path_set)
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

std::string	Methods::_uriFirstPart(void)
{
	std::string	uri_path;
	size_t		i = 0;

	if ((i = serv->req.uri.find("/", 1)) == std::string::npos)
		uri_path = serv->req.uri;
	else
		uri_path = serv->req.uri.substr(0, i);
	if (!uri_path.empty() && *(uri_path.rbegin()) != '/')
		return (uri_path + '/');
	return (uri_path);
}

	/* (3) authentication */
void	Methods::_checkRequiredAuthentication(void)
{
	_envp["AUTH_TYPE"] = "";
	_envp["REMOTE_USER"] = "";
	if (!_authenticate.empty())
	{
		if (serv->req.headers.find("Authorization") != serv->req.headers.end())
		{
			if (_checkUserExist(serv->req.headers.find("Authorization")->second, _authenticate[1]))
				return ;
		}
		serv->resp.header_fields.insert(std::make_pair("WWW-Authenticate", "Basic realm=" + _authenticate[0] + ", charset=\"UTF-8\""));
		_envp["AUTH_TYPE"] = "Basic";
		throw ServerBloc::Unauthorized();
	}
}

bool	Methods::_checkUserExist(std::string & user, std::string & auth_path)
{
	std::vector<std::string>	users;
	std::string					line;
	std::fstream				user_file(auth_path);

	_envp["AUTH_TYPE"] = user.substr(0, user.find(' ') - 1);
	if (serv->req.strFindCaseinsensitive(user, "Basic") != std::string::npos && user_file.good())
	{
		user.erase(serv->req.strFindCaseinsensitive(user, "Basic"), 6);
		user = _decodeUser(user);
		while (std::getline(user_file, line))
			users.push_back(line);
		for (std::vector<std::string>::iterator it = users.begin(); it != users.end(); ++it)
		{
			if (*it == user)
			{
				_envp["REMOTE_USER"] = user.substr(0, user.find(':') - 1);
				return true;
			}
		}
	}
	return false;
}

std::string	Methods::_decodeUser(std::string & user)
{
	unsigned char	from_base64[] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
									 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
									 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62,  255, 62,  255, 63,
									 52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  255, 255, 255, 255, 255, 255,
									 255, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,   11,  12,  13,  14,
									 15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255,  255, 255, 255, 63,
									 255, 26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
									 41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255,  255, 255, 255, 255};
	// Make sure string length is a multiple of 4
	while ((user.size() % 4) != 0)
		user.push_back('=');

	size_t	encoded_size = user.size();
	std::vector<unsigned char>	ret;

	ret.reserve(3 * encoded_size / 4);
	for (size_t i = 0; i < encoded_size; i += 4)
	{
		// Get values for each group of four base 64 characters
		unsigned char	b4[4];
		b4[0] = (user[i + 0] <= 'z') ? from_base64[static_cast<unsigned char>(user[i + 0])] : 0xff;
		b4[1] = (user[i + 1] <= 'z') ? from_base64[static_cast<unsigned char>(user[i + 1])] : 0xff;
		b4[2] = (user[i + 2] <= 'z') ? from_base64[static_cast<unsigned char>(user[i + 2])] : 0xff;
		b4[3] = (user[i + 3] <= 'z') ? from_base64[static_cast<unsigned char>(user[i + 3])] : 0xff;
		// Transform into a group of three bytes
		unsigned char	b3[3];
		b3[0] = static_cast<unsigned char>(((b4[0] & 0x3f) << 2) + ((b4[1] & 0x30) >> 4));
		b3[1] = static_cast<unsigned char>(((b4[1] & 0x0f) << 4) + ((b4[2] & 0x3c) >> 2));
		b3[2] = static_cast<unsigned char>(((b4[2] & 0x03) << 6) + ((b4[3] & 0x3f) >> 0));
		// Add the byte to the return value if it isn't part of an '=' character (indicated by 0xff)
		if (b4[1] != 0xff)
			ret.push_back(b3[0]);
		if (b4[2] != 0xff)
			ret.push_back(b3[1]);
		if (b4[3] != 0xff)
			ret.push_back(b3[2]);
	}

	std::string cat;

	for (std::vector<unsigned char>::iterator it = ret.begin(); it != ret.end(); ++it)
		cat += static_cast<char>(*it);
	return (cat);
}

	/* (4) allowed method */
void	Methods::_checkAllowedMethods(void)
{
	std::string	cat_meth;

	if (!_methods.empty()) /* if not empty */
	{
		for (std::vector<std::string>::iterator it = _methods.begin(); it != _methods.end(); ++it)
		{
			if (serv->req.method == *it)
				return ;
			if (*it != *(_methods.rbegin()))
				cat_meth += *it + ", ";
			else
				cat_meth += *it;
		}
		serv->resp.header_fields.insert(std::make_pair("Allow", cat_meth));
		throw ServerBloc::MethodNotAllowed();
	}
}

	/* (5) max_body_size */
void	Methods::_checkMaxBodySize(void)
{
	if (serv->req.body.length() > _max_body_size)
		throw ServerBloc::PayloadTooLarge();
}
