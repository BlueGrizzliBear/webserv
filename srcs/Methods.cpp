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
	/* uri resolution process (treat ../ and ./) */
	_URIResolutionProcess();
	_queryResolutionProcess();

	if (serv->req.method == "GET")
		_applyGet();
	else if (serv->req.method == "HEAD")
		_applyHead();
	else if (serv->req.method == "POST")
		_applyPost();
	else if (serv->req.method == "PUT")
		_applyPut();

	// serv->req.display(); 
}

void	Methods::customError(std::string & status_code, std::string & reason_phrase)
{
	std::vector<std::string>	error_pages;

	_findVect(serv->dir, "error_page", &error_pages);
	if (!error_pages.empty() && _ErrorNbInErrorPageList(error_pages, status_code))
	{
		serv->req.uri = error_pages.back();
		_findPath();
		if (_fileExist(_path) == true)
		{
			_checkContentType();
			_fillBody();
		}
		else
		{
			_fillDefaultExceptionBody(status_code, reason_phrase);
			serv->resp.header_fields.insert(std::make_pair("Content-Type", "text/html"));
		}
	}
	else
	{
		_fillDefaultExceptionBody(status_code, reason_phrase);
		serv->resp.header_fields.insert(std::make_pair("Content-Type", "text/html"));
	}
	serv->resp.header_fields.insert(std::make_pair("Content-Length", _getSizeOfStr(serv->resp.body)));
	serv->resp.header_fields.insert(std::make_pair("Transfer-Encoding", "identity"));
	serv->req.clear();
}

bool		Methods::_ErrorNbInErrorPageList(std::vector<std::string> & list, std::string & status)
{
	for (std::vector<std::string>::iterator it = list.begin(); it != list.end(); ++it)
	{
		if (*it == status)
			return true;
	}
	return false;
}

void	Methods::_fillDefaultExceptionBody(std::string & status, std::string & reason)
{
	serv->resp.body = "<html>\n";
	serv->resp.body += "<head><title>" + status + " " + reason + "</title></head>\n";
	serv->resp.body += "<body>\n";
	serv->resp.body += "<center><h1>" + status + " " + reason + "</h1></center>\n";
}

void	Methods::_URIResolutionProcess(void)
{
	size_t pos = 0;
	std::string tmp;
	std::string new_uri;

	/* Alorithm from RFC */
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

void	Methods::_queryResolutionProcess(void)
{
	// to implement
}

void	Methods::_applyGet(void)
{
	/* Check if path exist on server */
	_findPath();

	/* execute specific to GET request */
	_executeGetReq();
	/* Check if server knows file type */
	_checkContentType();
	/* Fill body with file content */
	if (_path != "./dir_listing.html")
		_fillBody();
	/* Fill header informations */
	/* (1) Fill Status Line */
	_GetHeaderStatusCode();

	/* (2) Fill Content-lenght */
	serv->resp.header_fields.insert(std::make_pair("Content-Length", _getSizeOfStr(serv->resp.body)));

	/* (3) Fill Last-Modified */
	if (_path != "./dir_listing.html")
		_lastModifiedHeader(_getFileTime());
	/* (4) Fill Transfer-Encoding */
	serv->resp.header_fields.insert(std::make_pair("Transfer-Encoding", "identity"));


}

void	Methods::_applyHead(void)
{
	/* Same as GET method but don't send body part */
	_applyGet();
	serv->resp.body.clear();
}

void	Methods::_applyPost()
{
	/* Check if path exist on server */
	_findPath();

	/* execute specific to POST request */
	if (_cgi_path.empty())
	{
		_executePostReq();
		/* Fill header informations */
		/* (1) Fill Status Line */
		_GetHeaderStatusCode();
	}
	else
		_executeCGI();

	// /* (2) Fill Content-length */
	serv->resp.header_fields.insert(std::make_pair("Content-Length", _getSizeOfStr(serv->resp.body)));
}

void	Methods::_applyPut(void)
{
	/* Check if path exist on server */
	_findPath();

	/* MUST send a 400 (Bad Request) response to a PUT request that contains a Content-Range header field */
	if (serv->req.headers.find("Content-Range") != serv->req.headers.end())
		throw ServerBloc::BadRequest();

	/* Indepotent method check (execute request only if changes are made */
	/* An origin server MUST NOT send a Last-Modified field, in a
	successful response to PUT unless the request's representation data
	was saved without any transformation applied to the body and the Last-Modified
	value reflects the new representation. */
	if (_cmpTimeInfo(_getHeaderIfUnmodifiedSinceTime(), _getFileTime()) == 0)
		throw ServerBloc::PreconditionFailed();
	else if (serv->req.body == _readFileToStr() && _cmpTimeInfo(_getHeaderIfUnmodifiedSinceTime(), _getFileTime()) == 1)
		_lastModifiedHeader(_getFileTime());
	else	/* execute specific to PUT request */
	{
		/* (1) Fill Status Line header */
		_PutHeaderStatusCode();
		/* Execute request */
		_executePutReq();
	}

	/* Fill header informations */
	/* (2) Fill Content-Location */
		// return the path to the newly or modified file
}

/* Execute Get request */
void	Methods::_executeGetReq(void)
{
	if (!_path.empty() && *(_path.rbegin()) != '/' && _isDirectory(_path) == true)
		_path.append("/");
	/* create html list directory if autoindex off and copy to body */
	if (_autoindex == false)
	{
		if (!_path.empty() && *(_path.rbegin()) == '/')
			_createIndexHTML();
	}
	else	/* copy asked file to body if exist */
	{
		if (!_path.empty() && *(_path.rbegin()) == '/')
			_findIndex(_indexes);
		if (!_path.empty() && *(_path.rbegin()) == '/')
			throw ServerBloc::Forbidden();
		if (_fileExist(_path) == false)
			throw ServerBloc::NotFound();
	}
	// COUT << "_path|" << _path << "|" << ENDL;
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
	struct dirent		*dp;
	struct stat			info;
	struct tm			*timeinfo;
	char				date[20];
	std::string			file_path;
	std::stringstream	dir_list;

	dir_list << "<html>\n";
	dir_list << "<head><title>Index of /</title></head>\n";
	dir_list << "<body bgcolor=\"white\">\n";
	dir_list << "<h1>Index of " << serv->req.uri << "</h1><hr><pre>\n";
	while ((dp = readdir(dir)) != NULL)
	{
		if (strcmp(dp->d_name, "."))
		{
			std::string		points(30 - strlen(dp->d_name), '.');

			file_path = _path + dp->d_name;
			dir_list << "<a href=\"" << serv->req.uri;
			if (!serv->req.uri.empty() && *(serv->req.uri.rbegin()) != '/')
				dir_list << '/';
			dir_list << dp->d_name << "\">" << dp->d_name;
			if (dp->d_type == DT_DIR)
				dir_list << '/';
			dir_list << "</a>";
			if (!lstat(file_path.c_str(), &info))
			{
  				timeinfo = localtime(&info.st_mtime);
				strftime(date, 20, "%d-%b-%Y %H:%M", timeinfo);
				if (strcmp(dp->d_name, ".."))
					dir_list << points << date << "................" << info.st_size;
				dir_list << '\n';
			}
			else
			{
				if (strcmp(dp->d_name, ".."))
					dir_list << points << "unauthorized..................unauthorized";
				dir_list << '\n';
			}
		}
	}
	dir_list << "</pre><hr></body>\n";
	dir_list << "</html>";
	serv->resp.body = dir_list.str();
}

void	Methods::_findIndex(std::vector<std::string> & indexes)
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

/* Execute Put request */
void	Methods::_executePutReq(void)
{
	std::ofstream	file(_path);

	file << serv->req.body;
	file.close();
}

/* Execute Post request */
void	Methods::_executePostReq(void)
{
	std::ofstream	file(_path, std::ios_base::app);

	file << serv->req.body;
	file.close();
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

/* Fill Body */
void	Methods::_fillBody()
{
	std::ifstream		file(_path.c_str());

	serv->resp.body.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	file.close();
}

std::string	Methods::_getSizeOfStr(std::string const & str)
{
	std::stringstream size;
	size << str.length();
	return (size.str());
}
