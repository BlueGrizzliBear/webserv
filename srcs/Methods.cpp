#include "./Methods.hpp"

/* Methods Class Declaration */
/* Constructor */
/*	default		(1)	*/
Methods::Methods(void) : serv(NULL), client(NULL) {}

/*	parent	(2)	*/
Methods::Methods(ServerBloc & server, Client & client, std::string const & code, std::string const & phrase)
: serv(&server), client(&client), _max_body_size(1000000), _headerIncomplete(true), _writtenBytes(0)
{
	/* uri resolution process (treat ../ and ./) */
	_URIResolutionProcess();
	_queryResolutionProcess();

	/* Check if path exist on server */
	_findPath();

	if (code.empty() && phrase.empty())
	{
		_checkRequiredAuthentication();	/* check authenticate */
		_checkMaxBodySize();
		_checkAllowedMethods();	/* return exeption if method not allowed */
	}
	else
		customError(code, phrase);
}

/*	copy		(3)	*/
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
	if (client->req.method == "GET")
		_applyGet();
	else if (client->req.method == "HEAD")
		_applyHead();
	else if (client->req.method == "POST")
		_applyPost();
	else if (client->req.method == "PUT")
		_applyPut();

	// client->req.display();
}

void	Methods::customError(std::string const & status_code, std::string const & reason_phrase)
{
	std::vector<std::string>	error_pages;

	_findVect(serv->dir, "error_page", &error_pages);
	if (!error_pages.empty() && _ErrorNbInErrorPageList(error_pages, status_code))
	{
		client->req.uri = error_pages.back();
		_findPath();
		if (_fileExist(_path) == true)
		{
			_checkContentType();
			_fillStrFromFile(client->resp.body, _path);
		}
		else
		{
			_fillDefaultExceptionBody(status_code, reason_phrase);
			client->resp.header_fields["Content-Type"] = "text/html";
		}
	}
	else
	{
		_fillDefaultExceptionBody(status_code, reason_phrase);
		client->resp.header_fields["Content-Type"] = "text/html";
	}
	client->resp.header_fields["Content-Type"].append("; charset=utf-8");
	client->resp.header_fields.insert(std::make_pair("Content-Length", _getSizeOfStr(client->resp.body)));
	client->resp.header_fields.insert(std::make_pair("Transfer-Encoding", "identity"));
}

bool		Methods::_ErrorNbInErrorPageList(std::vector<std::string> & list, std::string const & status)
{
	for (std::vector<std::string>::iterator it = list.begin(); it != list.end(); ++it)
	{
		if (*it == status)
			return true;
	}
	return false;
}

void	Methods::_fillDefaultExceptionBody(std::string const & status, std::string const & reason)
{
	client->resp.body = "<html>\n";
	client->resp.body += "<head><title>" + status + " " + reason + "</title></head>\n";
	client->resp.body += "<body>\n";
	client->resp.body += "<center><h1>" + status + " " + reason + "</h1></center>\n";
}

void	Methods::_URIResolutionProcess(void)
{
	size_t pos = 0;
	std::string tmp;
	std::string new_uri;

	/* Alorithm from RFC */
	while (!client->req.uri.empty())
	{
		/* 1st */
		if (!client->req.uri.find(tmp = "../") || !client->req.uri.find(tmp = "./"))
			client->req.uri.erase(0, tmp.size());
		/* 2nd */
		else if (!client->req.uri.find(tmp = "/./") || client->req.uri == (tmp = "/."))
			client->req.uri.replace(0, tmp.size(), "/");
		/* 3rd */
		else if (!client->req.uri.find(tmp = "/../") || client->req.uri == (tmp = "/.."))
		{
			client->req.uri.replace(0, tmp.size(), "/");
			if ((pos = new_uri.rfind("/")) != std::string::npos)
				new_uri.erase(pos, pos - new_uri.size());
		}
		/* 4th */
		else if (client->req.uri == "." || client->req.uri == "..")
			client->req.uri.erase(0, tmp.size());
		/* 5th */
		else
		{
			size_t i = (client->req.uri.find("/") == 0) ? 1 : 0;
			if ((pos = client->req.uri.find("/", i)) == std::string::npos)
				pos = client->req.uri.size();
			new_uri += client->req.uri.substr(0, pos);
			client->req.uri.erase(0, pos);
		}
	}
	client->req.uri = new_uri;
}

void	Methods::_queryResolutionProcess(void)
{
	size_t	begin;
	size_t	end;

	/*    /over/there?name=ferret&name2=ferret2#nose  */
	/*    \_________/ \_______________________/ \__/  */
	/*        |            |                     |    */
	/*       path        query               fragment */
	if ((begin = client->req.uri.find('?')) != std::string::npos)
	{
		end = client->req.uri.find('#');	/* until # or end of uri */
		_query = client->req.uri.substr(begin + 1, end);
		client->req.uri.erase(begin, std::string::npos);
	}
	else
		_query = "";
}

void	Methods::_applyGet(void)
{
	if (_cgi_path.empty())
	{
		/* execute specific to GET request */
		_executeGetReq();
		/* Check if server knows file type */
		_checkContentType();
		/* Fill body with file content */
		if (_path != "./dir_listing.html")
			_fillStrFromFile(client->resp.body, _path);
		/* Fill header informations */
		/* (1) Fill Status Line */
		_GetHeaderStatusCode();

		/* (2) Fill Last-Modified */
		if (_path != "./dir_listing.html")
			_lastModifiedHeader(_getFileTime());
	}
	else
		_launchCGI();
	/* (4) Fill Content-lenght */
	client->resp.header_fields.insert(std::make_pair("Content-Length", _getSizeOfStr(client->resp.body)));
}

void	Methods::_applyHead(void)
{
	/* Same as GET method but don't send body part */
	_applyGet();
	client->resp.body.clear();
}

void	Methods::_applyPost()
{
	/* execute specific to POST request */
	if (_cgi_path.empty())
	{
		/* (1) Fill Status Line header */
		_PostHeaderStatusCode();
		/* Execute request */
		_executePostReq();
	}
	else
	{
		_launchCGI();
		/* (2) Fill Content-length */
		client->resp.header_fields.insert(std::make_pair("Content-Length", _getSizeOfStr(client->resp.body)));
	}
	/* if header 201 creater ou content negotation header request (Accept, Accept-*) */
	if (client->resp.status_code == "201"
	|| (client->req.headers.find("Accept-Charset") != client->req.headers.end())
	|| (client->req.headers.find("Accept-Language") != client->req.headers.end()))
		client->resp.header_fields.insert(std::make_pair("Content-Location", client->req.uri));
}

void	Methods::_applyPut(void)
{
	/* MUST send a 400 (Bad Request) response to a PUT request that contains a Content-Range header field */
	if (client->req.headers.find("Content-Range") != client->req.headers.end())
		throw ServerBloc::BadRequest();

	/* Indepotent method check (execute request only if changes are made */
	/* An origin server MUST NOT send a Last-Modified field, in a
	successful response to PUT unless the request's representation data
	was saved without any transformation applied to the body and the Last-Modified
	value reflects the new representation. */
	if (_cmpTimeInfo(_getHeaderIfUnmodifiedSinceTime(), _getFileTime()) == 0)
		throw ServerBloc::PreconditionFailed();
	else if (client->req.body == _readFileToStr() && _cmpTimeInfo(_getHeaderIfUnmodifiedSinceTime(), _getFileTime()) == 1)
	{
		_lastModifiedHeader(_getFileTime());
		_GetHeaderStatusCode(); /* Fill with 200 OK status code */
	}
	else	/* execute specific to PUT request */
	{
		/* (1) Fill Status Line header */
		_PutHeaderStatusCode();
		/* Execute request */
		_executePutReq();
	}
	if (client->resp.status_code == "201"
	|| (client->req.headers.find("Accept-Charset") != client->req.headers.end())
	|| (client->req.headers.find("Accept-Language") != client->req.headers.end()))
		client->resp.header_fields.insert(std::make_pair("Content-Location", client->req.uri));
}

/* Execute Get request */
void	Methods::_executeGetReq(void)
{
	if (!_path.empty() && *(_path.rbegin()) != '/' && _isDirectory(_path) == true)
		_path.append("/");

	if (_autoindex == false)	/* create html list directory if autoindex off and copy to body */
	{
		if (!_path.empty() && *(_path.rbegin()) == '/')
			_createIndexHTML();
		else
		{
			if (_fileExist(_path) == false)
				throw ServerBloc::NotFound();
		}
	}
	else	/* copy asked file to body if exist */
	{
		if (!_path.empty() && (*(_path.rbegin()) == '/' ))	/* FOLDER */
		{
			if (!_indexes.empty())
			{
				if ((client->req.headers.find("Accept-Language") != client->req.headers.end()))
					_findFile("Accept-Language", _indexes);
				else
					_findIndex();

			}
			else
				throw ServerBloc::Forbidden();
		}
		else	/* FILE */
		{
			std::vector<std::string> files;
			_createVectorFromCWD(files, _pathWithoutLastPart());

			if ((client->req.headers.find("Accept-Charset") != client->req.headers.end()))
			{
				_findFile("Accept-Charset", files);
			}
			else if ((client->req.headers.find("Accept-Language") != client->req.headers.end()))
			{
				_findFile("Accept-Language", files);
			}
		}
		if (_fileExist(_path) == false)
			throw ServerBloc::NotFound();
		if ((client->req.headers.find("Accept-Charset") != client->req.headers.end())
		|| (client->req.headers.find("Accept-Language") != client->req.headers.end()))
			client->resp.header_fields.insert(std::make_pair("Content-Location", client->req.uri));
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
	DIR		*dir;

	/* check if directory exist */
	if ((dir = opendir(_path.c_str())))
	{
		/* list directory in html format*/
		_createHTMLListing(dir);
		_path = "./dir_listing.html";
		closedir(dir);
	}
	else
		throw ServerBloc::NotFound();
}

void	Methods::_createHTMLListing(DIR * dir)
{
	struct dirent		*dp;
	struct stat			info;
	struct tm			*timeinfo;
	char				date[20];
	std::string			file_path;
	std::stringstream	dir_list;

	dir_list << "<html>\n";
	dir_list << "<head><title>Index of /</title></head>\n";
	dir_list << "<body bgcolor=\"white\">\n";
	dir_list << "<h1>Index of " << client->req.uri << "</h1><hr><pre>\n";
	while ((dp = readdir(dir)) != NULL)
	{
		if (Request::ft_strcmp(dp->d_name, "."))
		{
			std::string		points(30 - Request::ft_strlen(dp->d_name), '.');

			file_path = _path + dp->d_name;
			dir_list << "<a href=\"" << client->req.uri;
			if (!client->req.uri.empty() && *(client->req.uri.rbegin()) != '/')
				dir_list << '/';
			dir_list << dp->d_name << "\">" << dp->d_name;
			if (dp->d_type == DT_DIR)
				dir_list << '/';
			dir_list << "</a>";
			if (!lstat(file_path.c_str(), &info))
			{
  				timeinfo = localtime(&info.st_mtime);
				strftime(date, 20, "%d-%b-%Y %H:%M", timeinfo);
				if (Request::ft_strcmp(dp->d_name, ".."))
					dir_list << points << date << "................" << info.st_size;
				dir_list << '\n';
			}
			else
			{
				if (Request::ft_strcmp(dp->d_name, ".."))
					dir_list << points << "unauthorized..................unauthorized";
				dir_list << '\n';
			}
		}
	}
	dir_list << "</pre><hr></body>\n";
	dir_list << "</html>";
	client->resp.body = dir_list.str();
}

void	Methods::_createVectorFromCWD(std::vector<std::string> & files, std::string path)
{
	DIR *dir;

	if ((dir = opendir(path.c_str())))	/* list directory in html format*/
	{
		struct dirent		*dp;

		while ((dp = readdir(dir)) != NULL)
		{
			if (dp->d_type == DT_REG)
				files.push_back(dp->d_name);
		}
		closedir(dir);
	}
	else
		throw ServerBloc::NotFound();
}

std::string	Methods::_trimExtension(std::string & str)
{
	std::string	fileWithoutExt;
	size_t		i = 0;

	if ((i = str.rfind(".")) == std::string::npos)
		fileWithoutExt = str;
	else
		fileWithoutExt = str.substr(0, i);
	return (fileWithoutExt);
}

void	Methods::_findFile(std::string header, std::vector<std::string> files)
{
	std::map<float, std::vector<std::string> > * storage = NULL;
	if (header == "Accept-Charset")
		storage = &_charsets;
	else if (header == "Accept-Language")
		storage = &_languages;

	_createAcceptedMap(header, storage);
	for (std::map<float, std::vector<std::string> >::reverse_iterator l_it = storage->rbegin(); l_it != storage->rend(); ++l_it)
	{
		for (std::vector<std::string>::iterator lv_it = l_it->second.begin(); lv_it != l_it->second.end(); ++lv_it)
		{
			for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it)
			{
				if ((*lv_it == "*" || client->req.strFindCaseinsensitive(*it, lv_it->c_str()) != std::string::npos)
				&& _fileExist(_pathWithoutLastPart() + *it) == true)
				{
					_path = _pathWithoutLastPart() + *it;
					_checkContentType();
					if (header == "Accept-Language")
						client->resp.header_fields.insert(std::make_pair("Content-Language", *lv_it));
					else
						_default_charset = "; charset=" + *lv_it;
					return ;
				}
			}
		}
	}
	if (header == "Accept-Language")
	{
		for (std::map<float, std::vector<std::string> >::reverse_iterator l_it = storage->rbegin(); l_it != storage->rend(); ++l_it)
		{
			for (std::vector<std::string>::iterator lv_it = l_it->second.begin(); lv_it != l_it->second.end(); ++lv_it)
			{
				for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it)
				{
					if (lv_it->find("-") != std::string::npos && client->req.strFindCaseinsensitive(*it, lv_it->substr(0, lv_it->find("-")).c_str()) != std::string::npos
					&& _fileExist(_pathWithoutLastPart() + *it) == true)
					{
						_path = _pathWithoutLastPart() + *it;
						_checkContentType();
						if (header == "Accept-Language")
							client->resp.header_fields.insert(std::make_pair("Content-Language", *lv_it));
						return ;
					}
				}
			}
		}
	}
	for (std::vector<std::string>::iterator files_v = files.begin(); files_v != files.end(); ++files_v)
	{
		if (_trimExtension(*files_v) == _pathLastPart())
		{
			_path = _pathWithoutLastPart() + *files_v;

			_checkContentType();
			if (header == "Accept-Charset")
				_default_charset = "; charset=utf-8";
			return ;
		}
	}
	if (!_path.empty() && (*(_path.rbegin()) == '/' ))	/* FOLDER */
		_findIndex();
	if (header == "Accept-Charset")
		_default_charset = "; charset=utf-8";
}

void	Methods::_findIndex(void)
{
	for (std::vector<std::string>::iterator it = _indexes.begin(); it != _indexes.end(); ++it)
	{
		if (_fileExist(_path + *it) == true)
		{
			_path += *it;
			_checkContentType();
			return ;
		}
	}
	_path += *(_indexes.begin());
	_checkContentType();
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

/* Execute Put request */
void	Methods::_createDirectories(void)
{
	std::string final_path;
	size_t i = 1;

	while (final_path != _pathWithoutLastPart())
	{
		final_path = _pathIterateThroughFolders(i);
		errno = 0;
		if (mkdir(final_path.c_str(), 0755) < 0 && errno != EEXIST)
		{
			CERR << "Error in mkdir(): " << strerror(errno) << ENDL;
			throw ServerBloc::InternalServerError();
		}
		i++;
	}
}

void	Methods::_executePutReq(void)
{
	_createDirectories();

	std::ofstream	file(_path.c_str());

	_fillFileFromStr(file, client->req.body);
}

/* Execute Post request */
void	Methods::_executePostReq(void)
{
	_createDirectories();

	std::ofstream	file(_path.c_str(), std::ios_base::app);

	_fillFileFromStr(file, client->req.body);
}

/* Check content type */
void	Methods::_checkContentType(void)
{
	/* If directory do nothing */
	if (!_path.empty() && *(_path.rbegin()) == '/')
		return ;
	/* Find correct content-type matching file extension */
	std::string	contentType;
	std::string	pathExt = _pathExtension(_path);

	if (serv->getParent()->getDictionary().mimeDic.find(pathExt) != serv->getParent()->getDictionary().mimeDic.end())
		contentType = serv->getParent()->getDictionary().mimeDic.find(pathExt)->second;
	else
		contentType = serv->getParent()->getDictionary().mimeDic.find("txt")->second;
	/* check if content-type exist if the request content-type */
	if (client->req.headers.find("Content-Type") != client->req.headers.end())
	{
		if (client->req.headers.find("Content-Type")->second != contentType)
			throw ServerBloc::UnsupportedMediaType();
	}
	client->resp.header_fields["Content-Type"] = contentType + _default_charset;
}

std::string	Methods::_pathExtension(const std::string & path)
{
	std::string	ext;
	std::string::const_reverse_iterator	it = path.rbegin();

	while (it != path.rend())
	{
		if (*it == '.')
		{
			if (it == path.rbegin())
				return (ext);
			while(--it != path.rbegin())
				ext += *it;
			ext += *it;
			return (ext);
		}
		++it;
	}
	return ("txt");
}

/* Fill Body */
void	Methods::_fillStrFromFile(std::string & body, std::string const & path)
{
	std::ifstream		file(path.c_str());

	if (file.good())
	{
		body.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		file.close();
	}
	else
		throw ServerBloc::InternalServerError();
}

void	Methods::_fillFileFromStr(std::ofstream & file, std::string const & body)
{
	if (file.good())
	{
		file << body;
		file.close();
	}
	else
		throw ServerBloc::InternalServerError();
}

std::string	Methods::_getSizeOfStr(std::string const & str)
{
	std::stringstream size;
	size << str.length();
	return (size.str());
}
