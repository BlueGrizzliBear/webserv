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
	if (_fileExist(_path))
	{
		/* Fill Status Line */
		serv->resp.status_code = "201";
		serv->resp.reason_phrase = "Created";
	}
	else
	{
		if (serv->req.body.empty())
		{
			/* Fill Status Line */
			serv->resp.status_code = "204";
			serv->resp.reason_phrase = "No Content";
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

void	Methods::_lastModifiedHeader(void)
{
	struct stat			info;
	struct tm			*timeinfo;
	char				date[30];

	if (!lstat(_path.c_str(), &info))
	{
		timeinfo = gmtime(&info.st_mtime);
		strftime(date, 30, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
		serv->resp.header_fields.insert(std::make_pair("Last-Modified", date));
	}
}
