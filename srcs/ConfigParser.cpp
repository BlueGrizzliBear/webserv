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
				while (getline(file, _line))
				{
					_line_no++;
					CME << "Parsing line|"  << _line << "|" << EME;
					_parse_main_context(file);
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
	std::string fill(_count, ' ');

	CERR << _path << ":" << _line_no << ":" << _count << ":" << YELLOW << " error:" << RESET << ENDL;
	CERR << _line << ENDL;
	CERR << GREEN << fill << "^" << RESET << ENDL;
	CERR << "1 error generated." << ENDL;
}

void	ConfigParser::_parse_main_context(std::fstream & file)
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
				key = _line.substr(_count, len);
				COUT << "key |" << key << "|" << ENDL;
			}
			// std::string key(_line.substr(_count, _line.find_first_of(" \t;#", _count)));

			if (_is_in_dictionnary(_dic.context_dictionary, key))
			{
				CME << "The word <" << key << "> seems in the Dictionnary" << EME;
				if (key == "server")
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
void	ConfigParser::_parse_context(std::string &, std::fstream &)
{
	CME << "ADDDING SERVER" << EME;
}

void	ConfigParser::_parse_directive(std::string & key)
{
	std::vector<std::string> values;

	std::string::iterator it = _line.begin();
	_count = _line.find(key) + key.length();

	for (size_t i = 0; i < _count; i++)
		++it;

	// COUT << "_count|" << _count << "|" << ENDL;
	while (*it && (*it == ' ' || *it == '\t'))
	{
		++_count;
		++it;
	}
	// COUT << "_count|" << _count << "|" << ENDL;

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
			// std::string word(_line.substr(_count, _line.find_first_of(" \t", _count)));
			// COUT << "RESULT|" << _line.find(" ", _count) << ENDL;
			// COUT << "at letter|" << *it << "|" << ENDL;
			if (len != std::string::npos)
			{
				// COUT << "other words after" << ENDL;
				
				// COUT << "len|" << len << "|" << ENDL;
				// COUT << "len - _count|" << len - _count << "|" << ENDL;

				std::string word(_line.substr(_count, len - _count));
				// COUT << "word added is |" << word << "| of length|" << word.length() << "|" << ENDL;
				
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
