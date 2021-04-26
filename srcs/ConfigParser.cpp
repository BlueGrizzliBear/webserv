#include "./ConfigParser.hpp"

/* Constructor */
/*	default		(1)	*/
ConfigParser::ConfigParser() {}

/*	argument	(2)	*/
ConfigParser::ConfigParser(const char * path, char const ** envp) : _path(path), _line_no(0), _count(0), _bracket(0), _envp(envp)
{
	_try_open_file(path);	/* Parse the .conf file to create de server objects */

	_initServers();	/* Initialize server parts before getting our first connection */
}

/*	copy		(3)	*/
ConfigParser::ConfigParser(ConfigParser const & cpy) : _envp(cpy._envp)
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

	_main_dir = rhs._main_dir;
	_servers = rhs._servers;

	return (*this);
}

/* Member Functions */
ConfigParser::Servers &	ConfigParser::getServers(void)
{
	return (_servers);
}

ConfigParser::Directives &	ConfigParser::getMainDirs(void)
{
	return (_main_dir);
}

const char **	ConfigParser::getEnvp(void)
{
	return (_envp);
}

ServerDictionary &	ConfigParser::getDictionary(void)
{
	return (_dic);
}

void	ConfigParser::_display_string(std::string const & str)
{
	COUT << str << " ";
}

void	ConfigParser::_display_arguments(std::string const & arg)
{
	COUT << "[ " << arg << " ]";
}

void	ConfigParser::_display_dir(std::pair<const std::string, std::vector<std::string> > & pair)
{
	COUT << "( ";
	_display_string(pair.first);
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
	std::for_each(serv.dir.begin(), serv.dir.end(), _display_dir);
	COUT << ENDL;

	COUT << "--- Server Locations ---\n";
	std::for_each(serv.loc.begin(), serv.loc.end(), _display_location_bloc);
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

void	ConfigParser::_display_init_error(const char * main_err, const char * err)
{
	COUT << main_err << ": ";
	COUT << err << ENDL;
}

void	ConfigParser::_try_open_file(const char * path)
{
	struct stat	file_stat;
	std::string config(path);

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
					_parse_main_context(file, _dic.mainDic, _main_dir, _servers, _servers.back().loc);
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
			_parse_main_context(file, _dic.serverDic, tmp.dir, serv, tmp.loc);
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
		while (old != _bracket && ++_line_no && getline(file, _line))
			_parse_main_context(file, _dic.locationDic, tmp.loc_dir, serv, loc);
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

	_count = _line.find(key) + key.length();
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
			if (len != std::string::npos)
			{
				std::string word(_line.substr(_count, len - _count));
				values.push_back(word);
				_count += (len - _count);
				it += static_cast<long>(word.length());
			}
			else if (_count != _line.size())
			{
				len = _line.size() - _count;
				std::string word(_line.substr(_count, len - _count));
				values.push_back(word);
				_count += len;
				it += static_cast<long>(word.length());
			}
			else
				break ;
		}
	}
	if (_check_directive(key, values))
	{
		_display_parsing_error(_count - 1);
		throw UnexpectedToken();
	}
	if (*it != ';')
	{
		_display_parsing_error(_count);
		throw UnexpectedToken();
	}
	if (!dir.insert(std::make_pair(key, values)).second)
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

int		ConfigParser::_check_directive(std::string & key, std::vector<std::string> & values)
{
	// static size_t default_server = 0;
	/* Check it has at least 1 argument */
	if (key.empty())
		return (1);
	if (key == "listen")
	{
		/* Check if not 1 or 2 arguments in values */
		if (!(values.size() == 2 || values.size() == 1))
			return (1);
		/* Check if normal port number */
		if (!_str_is_digit(values[0]))
			return (1);
		/* Check if 2nd argument is 'default_server' */
		// if (values.size() == 2)
		// {
		// 	if (default_server || values[1] != "default_server")
		// 		return (1);
		// 	default_server++;
		// }
		/* Check is port already exists in another server bloc */
		// std::vector<ServerBloc>::iterator it_base = _servers.begin();
		// std::vector<ServerBloc>::iterator ite_base = _servers.end();
		// while (it_base != ite_base)
		// {
		// 	if ((*it_base).dir.find(key)->second == values)
		// 		return (1);
		// 	it_base++;
		// }
	}
	// else if (key[0] == "server_name")
	// {
	// }
	// else if (key == "error_page")
	// {
	// }
	else if (key == "root")
	{
		if (values.empty())
			return (1);
		if (values[0][0] != '/')
			return (1);
	}
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

void	ConfigParser::_verify_uniqueness(ServerBloc & serv, std::string str)
{
	std::map<std::string, std::vector<std::string> >::iterator begin = serv.dir.begin();
	std::map<std::string, std::vector<std::string> >::iterator end = serv.dir.end();
	unsigned int exists = 0;

	while (begin != end)
	{
		if ((*begin).first == str)
			exists++;
		begin++;
	}
	if (exists == 1)
		return ;
	_display_parsing_error(_count);
	throw MissingKey();
}

void	ConfigParser::_verify_serverbloc(ServerBloc & serv)
{
	_verify_uniqueness(serv, "listen");
	_verify_uniqueness(serv, "root");
	_verify_uniqueness(serv, "server_name");
}

void	ConfigParser::abortServers(const char * main_err, const char * err)
{
	_display_init_error(main_err, err);

	Servers::iterator s_it = _servers.begin();
	Servers::iterator s_ite = _servers.end();

	while (s_it != s_ite)
	{
		if (s_it->serv_port.fd != 0)
			close(s_it->serv_port.fd);
		s_it++;
	}
	throw Abort();
}

void	ConfigParser::_initPort(ServerBloc & serv)
{
	if (serv.is_default)
	{
		/* Creating socket file descriptor */
		if ((serv.serv_port.fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)		/* AF_INET: Protocoles Internet IPv4	|	SOCK_STREAM: Virtual Circuit Service */
			abortServers("Error in socket()", strerror(errno));

		/* Set the socket to non blocking */
		fcntl(serv.serv_port.fd, F_SETFL, O_NONBLOCK);

		/* Defining address struct */
		serv.serv_port.address.sin_family = AF_INET;						/* corresponding to IPv4 protocols */
		serv.serv_port.address.sin_addr.s_addr = htonl(INADDR_ANY);			/* corresponding to 0.0.0.0 */
		serv.serv_port.address.sin_port = htons(serv.port_no);	/* corresponding to the server port, must be > 1024 */

		/* Defining address length */
		serv.serv_port.addrlen = sizeof(serv.serv_port.address);

		/* Initialising other adress attributes to 0 */
		memset(serv.serv_port.address.sin_zero, '\0', sizeof(serv.serv_port.address.sin_zero));

		/* Assigning adress to the socket */
		if (bind(serv.serv_port.fd, reinterpret_cast<struct sockaddr *>(&serv.serv_port.address), sizeof(serv.serv_port.address)) < 0)
			abortServers("Error in bind()", strerror(errno));

		/* Enable socket to accept connections */
		if (listen(serv.serv_port.fd, MAX_CLIENTS) < 0)
			abortServers("Error in listen()", strerror(errno));
	}
}

void	ConfigParser::_initDefaultServer(ServerBloc & serv)
{
	Directives::iterator d_it = serv.dir.begin();
	Directives::iterator d_ite = serv.dir.end();

	while (d_it != d_ite)
	{
		if ((*d_it).first == "listen")
		{
			/* Check if default_server exists for this port */
			Servers::iterator s_it = _servers.begin();
			Servers::iterator s_ite = _servers.end();
			bool defaultDetected = 0;

			while (s_it != s_ite)
			{
				if (&*s_it != &serv && s_it->port_no == serv.port_no)
				{
					serv.is_unique = false;
					Directives::iterator sd_it = s_it->dir.begin();
					Directives::iterator sd_ite = s_it->dir.end();

					while (sd_it != sd_ite)
					{
						if ((*sd_it).first == "listen"
						&& ((((*sd_it).second.size() > 1 && (*sd_it).second[1] == "default_server")) || (*s_it).is_default == true))
						{
							if (defaultDetected)
								throw UnexpectedToken();
							defaultDetected = 1;
						}
						sd_it++;
					}
				}
				s_it++;
			}
			if (defaultDetected)
				serv.is_default = 0;
			else
				serv.is_default = 1;
		}
		d_it++;
	}
}

void	ConfigParser::_setPortNo(void)
{
	Servers::iterator s_it = _servers.begin();
	Servers::iterator s_ite = _servers.end();

	while (s_it != s_ite)
	{
		Directives::iterator d_it = s_it->dir.begin();
		Directives::iterator d_ite = s_it->dir.end();

		while (d_it != d_ite)
		{
			if ((*d_it).first == "listen")
			{
				s_it->port_no = static_cast<unsigned short>(std::atoi((*d_it).second[0].c_str()));
			}
			d_it++;
		}
		s_it++;
	}
}

void	ConfigParser::_setNonDefaultServers(void)
{
	Servers::iterator s_it = _servers.begin();
	Servers::iterator s_ite = _servers.end();

	while (s_it != s_ite)
	{
		if (!s_it->is_default)
		{
			Servers::iterator other_it = _servers.begin();
			Servers::iterator other_ite = _servers.end();

			while (other_it != other_ite)
			{
				if (other_it->is_default && other_it->port_no == s_it->port_no)
				{
					// COUT << "socket.fd|" << s_it->serv_port.fd << "|\n";
					// COUT << "s_addr|" << s_it->serv_port.address.sin_addr.s_addr << "|\n";
					s_it->serv_port = other_it->serv_port;
					// COUT << "APRES socket.fd|" << s_it->serv_port.fd << "| et other|" << other_it->serv_port.fd << "|\n";
					// COUT << "APRES s_addr|" << s_it->serv_port.address.sin_addr.s_addr << "| et other|" << other_it->serv_port.address.sin_addr.s_addr << "|\n";
					break ;
				}
				other_it++;
			}
		}
		s_it++;
	}
}

void	ConfigParser::_initServers(void)
{
	_setPortNo();

	Servers::iterator s_it = _servers.begin();
	Servers::iterator s_ite = _servers.end();

	while (s_it != s_ite)
	{
		_initDefaultServer(*s_it);
		_initPort(*s_it);
		// COUT << "Server.port #" << s_it->port_no << "| and isdefault|" << s_it->is_default << "|\n";
		s_it++;
	}

	_setNonDefaultServers();
}
