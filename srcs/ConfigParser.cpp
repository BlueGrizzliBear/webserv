#include "./ConfigParser.hpp"

/* Constructor */
/*	default		(1)	*/
ConfigParser::ConfigParser() {}

/*	argument	(2)	*/
ConfigParser::ConfigParser(const char * path) : _path(path), _line_no(0), _count(0), _bracket(0), _status(0)
{
	_try_open_file(path);
}

/*	copy		(3)	*/
ConfigParser::ConfigParser(ConfigParser const & cpy)
{
	*this = cpy;
}

/* Destructor */
ConfigParser::~ConfigParser() {}

/* Operators */
ConfigParser &ConfigParser::operator=(ConfigParser const & rhs)
{
	_path = rhs._path;
	_line = rhs._line;
	_line_no = rhs._line_no;
	_count = rhs._count;
	_bracket = rhs._bracket;
	_servers = rhs._servers;
	return (*this);
}

/* Member Functions */
ConfigParser::Servers &	ConfigParser::getServers(void)
{
	return (_servers);
}

int &	ConfigParser::getStatus(void)
{
	return (_status);
}

void	ConfigParser::_display_string(std::string const & str)
{
	COUT << str << " ";
}

void	ConfigParser::_display_arguments(std::string const & arg)
{
	COUT << "[ " << arg << " ]";
}

void	ConfigParser::_display_dir(std::pair<const std::vector<std::string>, std::vector<std::string> > & pair)
{
	COUT << "( ";
	std::for_each(pair.first.begin(), pair.first.end(), _display_string);
	COUT << " : ";
	std::for_each(pair.second.begin(), pair.second.end(), _display_string);
	COUT << ")\n";
}

void	ConfigParser::_display_location_bloc(std::pair<const std::vector<std::string>, LocationBloc> & pair)
{
	static size_t i = 0;
	COUT << GREEN << "-- Location #" << i++ << " --" << RESET << ENDL;

	COUT << "* Location Arguments *\n";
	std::for_each(pair.first.begin(), pair.first.end(), _display_arguments);
	COUT << ENDL;
	
	COUT << "* Locaiton Directives *\n";
	std::for_each(pair.second.loc_dir.begin(), pair.second.loc_dir.end(), _display_dir);
	COUT << ENDL;
}

void	ConfigParser::_display_server_bloc(ServerBloc & serv)
{
	static size_t i = 0;
	COUT << MAGENTA << "--- Server #" << i++ << " ---" << RESET << ENDL;

	COUT << "--- Server Directives ---\n";
	std::for_each(serv.serv_dir.begin(), serv.serv_dir.end(), _display_dir);
	COUT << ENDL;
	
	COUT << "--- Server Locations ---\n";
	std::for_each(serv.serv_loc.begin(), serv.serv_loc.end(), _display_location_bloc);
	COUT << ENDL;
}

void	ConfigParser::display_config(void)
{
	COUT << "=== Main Directives ===\n";
	std::for_each(_main_dir.begin(), _main_dir.end(), _display_dir);
	COUT << ENDL;

	COUT << "=== Servers ===\n";
	std::for_each(_servers.begin(), _servers.end(), _display_server_bloc);


}

bool	ConfigParser::_is_in_dictionnary(Dic dic, std::string word)
{
	if (dic.find(word) == dic.end())
		return (0);
	return (1);
}

void	ConfigParser::_display_parsing_error(size_t new_count)
{
	_count = new_count;
	std::string fill;
	for (size_t i = 0; i < _count; i++)
		fill.append(1, (_line[i] == '\t' ? '\t' : ' '));
	CERR << _path << ":" << _line_no << ":" << _count << ":" << YELLOW << " error:" << RESET << ENDL;
	CERR << _line << ENDL;
	CERR << GREEN << fill << "^" << RESET << ENDL;
	CERR << "1 error generated." << ENDL;
}


void	ConfigParser::_try_open_file(const char * path)
{
	struct stat	file_stat;
	std::string config(path);
	errno = 0;

	if (stat(path, &file_stat))
		throw FileAccess();
	else
	{
		if (S_ISREG(file_stat.st_mode))
		{
			std::fstream file;
			try
			{
				file.open(path, std::fstream::in);
			}
			catch(const std::exception& e)
			{
				throw FileOpen();
			}
			if (file.good())
			{
				while (getline(file, _line))
				{
					++_line_no;
					_parse_main_context(file, _dic.main_dictionary, _main_dir, _servers, _servers.back().serv_loc);
				}
				file.close();
			}
			else
				throw FileOpen();
		}
		else
			throw ItemNotFile();
	}
}

void	ConfigParser::_parse_main_context(std::fstream & file, Dic dic, Directives & dir, Servers & serv, Locations & loc)
{
	std::string::iterator it = _line.begin();

	_count = 0;
	while (*it)
	{
		if (*it == '#' || *it == ';')
			return ;
		if (_bracket != 0 && *it == '}')
		{
			--_bracket;
			return ;
		}
		if (*it == ' ' || *it == '\t')
			_count++;
		else
		{
			size_t len(_line.find_first_of(" \t;#", _count));
			std::string key;

			if (len != std::string::npos)
				key = _line.substr(_count, len - _count);
			if (_is_in_dictionnary(dic, key))
			{
				if (key == "server")
					_parse_server(key, file, serv);
				else if (key == "location")
					_parse_location(key, file, serv, loc);
				else
					_parse_directive(key, dir);
				return ;
			}
			else
			{
				_display_parsing_error(_count);
				throw InvalidKey();
			}
			_count += key.length();
		}
		++it;
	}
}

void	ConfigParser::_parse_server(std::string & key, std::fstream & file, Servers & serv)
{
	std::vector<std::string> values;
	std::string::iterator it = _line.begin();

	_count = _line.find(key) + key.length();
	for (size_t i = 0; i < _count; i++)
		++it;
	while (*it && (*it == ' ' || *it == '\t'))
	{
		++_count;
		++it;
	}
	while (*it)
	{
		if (*it == '#')
			break ;
		if (*it == ' ' || *it == '\t')
		{
			++_count;
			++it;
		}
		else
		{
			size_t len(_line.find_first_of(" \t#", _count));
			if ((_line.length() - _count == 1) || (len != std::string::npos))
			{
				std::string word(_line.substr(_count, len - _count));
				values.push_back(word);
				_count += len - _count;
				it += static_cast<long>(word.length());
			}
			else
				break ;
		}
	}
	if (values.size() == 1 && values[0] == "{")
	{
		ServerBloc tmp(this);
		size_t old = _bracket;

		++_bracket;
		tmp.getNo() = serv.size() + 1;
		while (old != _bracket && ++_line_no && getline(file, _line))
		{
			COUT << "line|" << _line << "|" << ENDL;
			_parse_main_context(file, _dic.server_dictionary, tmp.serv_dir, serv, tmp.serv_loc);
		}
		_verify_serverbloc(tmp);
		serv.push_back(tmp);
	}
	else
	{
		_display_parsing_error(_count);
		throw MissingArgument();
	}
	// CME << "Finished parsing server" << EME << ENDL;	
}



void	ConfigParser::_parse_location(std::string & key, std::fstream & file, Servers & serv, Locations & loc)
{
	std::vector<std::string> values;
	std::string::iterator it = _line.begin();

	_count = _line.find(key) + key.length();
	for (size_t i = 0; i < _count; i++)
		++it;
	while (*it && (*it == ' ' || *it == '\t'))
	{
		++_count;
		++it;
	}

	while (*it)
	{
		if (*it == '#')
			break ;
		if (*it == ' ' || *it == '\t')
		{
			++_count;
			++it;
		}
		else
		{
			size_t len(_line.find_first_of(" \t#", _count));
			if ((_line.length() - _count == 1) || (len != std::string::npos))
			{
				std::string word(_line.substr(_count, len - _count));
				values.push_back(word);
				_count += len - _count;
				it += static_cast<long>(word.length());
			}
			else
				break ;
		}
	}
	if ((values.size() == 2 && values[1] == "{") || (values.size() == 3 && values[2] == "{"))
	{
		LocationBloc tmp;
		size_t old = _bracket;

		++_bracket;
		while (++_line_no && getline(file, _line) && old != _bracket)
			_parse_main_context(file, _dic.location_dictionary, tmp.loc_dir, serv, loc);
		loc.insert(std::make_pair(values, tmp));
	}
	else
	{
		_display_parsing_error(_count);
		throw MissingArgument();
	}
	// CME << "Finished parsing location" << EME << ENDL;	
}

void	ConfigParser::_parse_directive(std::string & key, Directives & dir)
{
	std::vector<std::string> values;
	std::string::iterator it = _line.begin();
	size_t tmp;
	std::vector<std::string> keys; /* new */

	_count = _line.find(key) + key.length();
	keys.push_back(key); /* new */
	tmp = _count - 1;
	for (size_t i = 0; i < _count; i++)
		++it;
	while (*it && (*it == ' ' || *it == '\t'))
	{
		++_count;
		++it;
	}
	while (*it)
	{
		if (*it == '#' || *it == ';')
			break ;
		if (*it == ' ' || *it == '\t')
		{
			++_count;
			++it;
		}
		else
		{
			size_t len(_line.find_first_of(" \t;#", _count));
			if (key == "listen" && keys.size() < 2)
			{
				std::string word(_line.substr(_count, len - _count));
				keys.push_back(word);
				_check_if_already_exists(keys); /* new */
				_count += len - _count;
				it += static_cast<long>(word.length());
			}
			else if (len != std::string::npos)
			{
				std::string word(_line.substr(_count, len - _count));
				values.push_back(word);
				_count += len - _count;
				it += static_cast<long>(word.length());
			}
			else
				break ;
		}
	}
	if (_check_directive(keys, values))
	{
		_display_parsing_error(_count - 1);
		throw UnexpectedToken();
	}
	if (!dir.insert(std::make_pair(keys, values)).second)
	{
		_display_parsing_error(tmp);
		throw RedundantDirKey();
	}
	// CME << "Finished parsing directive" << EME << ENDL;
}

bool	ConfigParser::_str_is_digit(std::string const & str)
{
	std::string::const_iterator it = str.begin();
	std::string::const_iterator ite = str.end();
	while (it != ite && std::isdigit(*it))
		++it;
	return (!str.empty() && it == ite);
}

int		ConfigParser::_check_directive(std::vector<std::string> & key, std::vector<std::string> & values)
{
	static size_t default_server = 0;
	/* Check it has at least 1 argument */
	if (key.empty() || (key.size() == 1 && values.empty()))
		return (1);
	if (key[0] == "listen")
	{
		/* Check if not 2 arguments */
		if (!(key.size() == 2 && (values.size() == 0 || values.size() == 1)))
			return (1);
		/* Check if normal port number */
		if (!_str_is_digit(key[1]))
			return (1);
		/* Check if 2nd argument is 'default_server' */
		if (values.size() == 1)
		{
			if (default_server || values[0] != "default_server")
				return (1);
			default_server++;
		}
	}
	// else if (key[0] == "server_name")
	// {



	// }
	// else if (key == "error_page")
	// {

	// }
	// else if (key == "root")
	// {
		
	// }
	// else if (key == "autoindex")
	// {
		
	// }
	// else if (key == "limit_except")
	// {
		
	// }
	// else if (key == "upload_store")
	// {
		
	// }
	// missing 'expires', 'proxy_pass', 'cgi'
	return (0);
}

void	ConfigParser::_verify_serverbloc(ServerBloc & serv)
{
	std::map<std::vector<std::string>, std::vector<std::string> >::iterator begin = serv.serv_dir.begin();
	std::map<std::vector<std::string>, std::vector<std::string> >::iterator end = serv.serv_dir.end();

	while (begin != end)
	{
		if ((*begin).first[0] == "listen")
			return ;
		begin++;
	}
	_display_parsing_error(_count);
	throw MissingKey();
}

void	ConfigParser::_check_if_already_exists(std::vector<std::string> & new_key)
{
	std::vector<ServerBloc>::iterator it_base = _servers.begin();
	std::vector<ServerBloc>::iterator ite_base = _servers.end();

	size_t count = 1;
	while (it_base != ite_base)
	{
		count += (*it_base).serv_dir.count(new_key);
		it_base++;
	}
	if (count > 1)
	{
		_display_parsing_error(_count);
		throw InvalidKey();
	}
	return ;
}