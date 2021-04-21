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
		client->resp.status_code = "201";
		client->resp.reason_phrase = "Created";
		/* Add Header Content-Location pointing to the path of newly created file */
		// client->resp.header_fields.insert(std::make_pair("Content-Location", client->req.uri));
	}
	else
	{
		if (client->req.body.empty())
		{
			/* Fill Status Line */
			client->resp.status_code = "204";
			client->resp.reason_phrase = "No Content";
			// client->resp.header_fields.insert(std::make_pair("Content-Location", client->req.uri));
		}
		else
		{
			/* Fill Status Line */
			client->resp.status_code = "200";
			client->resp.reason_phrase = "OK";
		}
	}
}

void	Methods::_PostHeaderStatusCode(void)
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
		client->resp.status_code = "201";
		client->resp.reason_phrase = "Created";
		/* Add Header Content-Location pointing to the path of newly created file */
		client->resp.header_fields.insert(std::make_pair("Location", client->req.uri));
	}
	else
	{
		if (client->req.body.empty())
		{
			/* Fill Status Line */
			client->resp.status_code = "204";
			client->resp.reason_phrase = "No Content";
		}
		else
		{
			/* Fill Status Line */
			client->resp.status_code = "200";
			client->resp.reason_phrase = "OK";
		}
	}
}

void	Methods::_GetHeaderStatusCode(void)
{
	client->resp.status_code = "200";
	client->resp.reason_phrase = "OK";
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
			client->resp.header_fields.insert(std::make_pair("Last-Modified", date));
	}
}

struct tm *	Methods::_getHeaderIfUnmodifiedSinceTime(void)
{
	std::string		date;
	struct tm		*timeinfo = NULL;

	if (client->req.headers.find("If-Unmodified-Since") != client->req.headers.end())
		date = client->req.headers.find("If-Unmodified-Since")->second;
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

	if (file.good())
	{
		str.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		file.close();
	}
	return (str);
}

void	Methods::_createAcceptedMap(std::string header, std::map<float, std::vector<std::string> > * storage)
{
	COUT << "Creating Accepted Map\n";

	std::string value = client->req.headers.find(header)->second;

	while (!value.empty())
	{
		std::string language_range;
		float weight = 1;
		size_t pos = 0;

		if (value.find(",") == std::string::npos)
		{
			language_range = value.substr(0, value.size());

			if (language_range.find(";") != std::string::npos)
			{
				pos = language_range.find(";q=");

				weight = static_cast<float>(std::atof(language_range.substr(pos + 3).c_str()));

				language_range = language_range.substr(0, pos);

			}
			// COUT << "Inserting Language|" << language_range << "| and weight|" << weight << "|\n";
			// COUT << "value|" << value << "| and size|" << value.size() << "|\n";
			value.clear();
		}
		else
		{
			pos = value.find(",");
			language_range = value.substr(0, pos);

			if (value.find_first_of(" \t", pos) == pos + 1)
				pos++;
			value.erase(0, pos + 1);
			
			if (language_range.find(";") != std::string::npos)
			{
				// pos = language_range.find(";q=");
				pos = language_range.find(";");
				size_t pos_after = pos;
				if (language_range.find(" ;") == pos - 1 || language_range.find("\t;") == pos - 1)
					pos -= 1;
				if (language_range.find("; q=") == pos_after + 1 || language_range.find(";\tq=") == pos_after + 1)
					pos_after += 1;

				weight = static_cast<float>(std::atof(language_range.substr(pos_after + 2).c_str()));

				language_range = language_range.substr(0, pos);
			}
			// COUT << "Inserting Language|" << language_range << "| and weight|" << weight << "|\n";
		}
		(*storage)[weight].push_back(language_range);
	}
}
