#include "./ConfigParser.hpp"

/* Constructor */
/*	default		(1)	*/
ConfigParser::ConfigParser() {}

/*	argument	(2)	*/
ConfigParser::ConfigParser(const char * path)
{
	errno = 0;
	std::string config(path);
	struct stat	file_stat;

	if (stat(path, &file_stat))
		throw FileAccess();
	else
	{
		if (S_ISREG(file_stat.st_mode))
		{
			CME << "It is a file" << EME;
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
				CME << "File opening successful" << EME;
				// std::string buf;

				_path = path; 
				_line_no = 0;
				while (getline(file, _line))
				{
					++_line_no;
					CME << "Parsing line|"  << _line << "|" << EME;
					_parse_main_context(file, _dic.context_dictionary);
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

/*	copy		(3)	*/
ConfigParser::ConfigParser(ConfigParser const & cpy)
{
	*this = cpy;
}

/* Destructor */
ConfigParser::~ConfigParser() {}

/* Operators */
ConfigParser &ConfigParser::operator=( ConfigParser const & rhs )
{
	(void)rhs;
	return (*this);
}

/* Member Functions */
bool	ConfigParser::_is_in_dictionnary(std::map<std::string, int> dic, std::string word)
{
	if (dic.find(word) == dic.end())
		return (0);
	return (1);
}

void	ConfigParser::_display_error(void)
{
	std::string fill;
	for (size_t i = 0; i < _count; i++)
		fill.append(1, (_line[i] == '\t' ? '\t' : ' '));
	CERR << _path << ":" << _line_no << ":" << _count << ":" << YELLOW << " error:" << RESET << ENDL;
	CERR << _line << ENDL;
	CERR << GREEN << fill << "^" << RESET << ENDL;
	CERR << "1 error generated." << ENDL;
}

void	ConfigParser::_parse_main_context(std::fstream & file, std::map<std::string, int> dic)
{
	std::string::iterator it = _line.begin();
	_count = 0;
	std::string key = "";

	while (*it)
	{
		if (*it == '#' || *it == ';')
			return ;
		if (*it == ' ' || *it == '\t')
			_count++;
		else
		{
			COUT << "_line[count]|" << _line[_count] << "|\n";
			size_t len(_line.find_first_of(" \t;#", _count));
			std::string key;

			if (len != std::string::npos)
			{
				COUT << "len|" << len << "\n";
				key = _line.substr(_count, len - _count);
				COUT << "key |" << key << "|" << ENDL;
			}
			// std::string key(_line.substr(_count, _line.find_first_of(" \t;#", _count)));

			if (_is_in_dictionnary(dic, key))
			{
				CME << "The word <" << key << "> seems in the Dictionnary" << EME;
				if (key == "server" || key == "location")
					_parse_context(key, file);
				else
					_parse_directive(key);
				return ;
			}
			else
			{
				CME << "The word <" << key << "> IS NOT in the Dictionnary" << EME;
				_display_error();
				throw InvalidKey();
			}
			_count += key.length();
		}
		++it;
	}
}

// void	ConfigParser::_parse_context(std::string & key, std::string & line, std::fstream & file)
void	ConfigParser::_parse_context(std::string & key, std::fstream & file)
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
		{
			std::cout << "jumping out\n";
			break ;
		}
		if (*it == ' ' || *it == '\t')
		{
			++_count;
			++it;
		}
		else
		{
			size_t len(_line.find_first_of(" \t#", _count));
			if (len != std::string::npos)
			{
				std::string word(_line.substr(_count, len - _count));
				values.push_back(word);
				_count += len - _count;
				it += static_cast<long>(word.length());
			}
			else
			{
				CME << "No word after this one => we are at the end of the line" << EME;
				break ;
			}
		}
	}
	if (values.size() == 1 && values[0] == "{")
	{
		server_bloc tmp;
		// _directives.insert(std::make_pair(key, values));
		_bracket = 1;

		while (getline(file, _line) && _bracket == 1)
		{
			++_line_no;
			CME << "Parsing line|"  << _line << "|" << EME;
			_parse_main_context(file, _dic.server_dictionary);
		}

	}
	else
	{
		_display_error();
		throw MissingArgument();
	}
	CME << "Finished parsing context" << EME << ENDL;	
}

void	ConfigParser::_parse_directive(std::string & key)
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
		if (*it == '#' || *it == ';')
		{
			std::cout << "jumping out\n";
			break ;
		}
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
				_count += len - _count;
				it += static_cast<long>(word.length());
			}
			else
			{
				CME << "No word after this one => we are at the end of the line" << EME;
				break ;
			}
		}
	}

	if (values.empty())
	{
		_display_error();
		throw MissingArgument();
	}
	_directives.insert(std::make_pair(key, values));
	CME << "Finished parsing directive" << EME << ENDL;
}
