#include "./Methods.hpp"

/* Header server response */
void	Methods::_PutHeaderStatusCode(void)
{
/*	If the target resource does not have a current representation and the
	PUT successfully creates one, then the origin server MUST inform the
	user agent by sending a 201 (Created) response.  If the target
	resource does have a current representation and that representation
	is successfully modified in accordance with the state of the enclosed
	representation, then the origin server MUST send either a 200 (OK) or
	a 204 (No Content) response to indicate successful completion of the
	request. */
	if (!_fileExist(_path))
	{
		/* Fill Status Line */
		serv->resp.status_code = "201";
		serv->resp.reason_phrase = "Created";
		/* Add Header Content-Location pointing to the path of newly created file */
		serv->resp.header_fields.insert(std::make_pair("Content-Location", serv->req.uri));
	}
	else
	{
		if (serv->req.body.empty())
		{
			/* Fill Status Line */
			serv->resp.status_code = "204";
			serv->resp.reason_phrase = "No Content";
			serv->resp.header_fields.insert(std::make_pair("Content-Location", serv->req.uri));
		}
		else
		{
			/* Fill Status Line */
			serv->resp.status_code = "200";
			serv->resp.reason_phrase = "OK";
		}
	}
}

void	Methods::_GetHeaderStatusCode(void)
{
	serv->resp.status_code = "200";
	serv->resp.reason_phrase = "OK";
}

struct tm	*Methods::_getFileTime(void)
{
	struct stat			info;
	struct tm			*timeinfo = NULL;

	if (!lstat(_path.c_str(), &info))
		timeinfo = gmtime(&info.st_mtime); // to check (time_t to struct tm)
	return timeinfo;
}

void	Methods::_lastModifiedHeader(struct tm * timeinfo)
{
	char				date[30];

	if (timeinfo)
	{
		strftime(date, 30, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
			serv->resp.header_fields.insert(std::make_pair("Last-Modified", date));
	}
}

struct tm *	Methods::_getHeaderIfUnmodifiedSinceTime(void)
{
	std::string		date;
	struct tm		*timeinfo = NULL;

	if (serv->req.headers.find("If-Unmodified-Since") != serv->req.headers.end())
		date = serv->req.headers.find("If-Unmodified-Since")->second;
	else
		return timeinfo;
	strptime(date.c_str(), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
	return timeinfo;
}

int	Methods::_cmpTimeInfo(struct tm * t1, struct tm * t2)
{
	if (!t1 || !t2)
		return -1;
	time_t	time1;
	time_t	time2;

	time1 = mktime(t1); // to check (struct tm to time_t)
	time2 = mktime(t2); // to check (struct tm to time_t)

	if (time1 < time2)
		return 0;
	return 1;
}

std::string	Methods::_readFileToStr(void)
{
	std::string		str;
	std::ifstream	file(_path.c_str());

	str.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	file.close();
	return str;
}
