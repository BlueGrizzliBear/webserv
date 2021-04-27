#include "./Methods.hpp"

/* Header server response */
void	Methods::_PutHeaderStatusCode(void)
{
	if (!_fileExist(_path))
	{
		client->resp.status_code = "201";
		client->resp.reason_phrase = "Created";
		client->resp.header_fields.insert(std::make_pair("Location", client->req.uri));
	}
	else /* if (client->resp.body.empty()) */
	{
		client->resp.status_code = "204";
		client->resp.reason_phrase = "No Content";
	}
}

void	Methods::_PostHeaderStatusCode(void)
{
	if (!_fileExist(_path))
	{
		client->resp.status_code = "201";
		client->resp.reason_phrase = "Created";
		client->resp.header_fields.insert(std::make_pair("Location", client->req.uri));
	}
	else
	{
		client->resp.status_code = "200";
		client->resp.reason_phrase = "OK";
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
		timeinfo = gmtime(&info.st_mtime);
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

	time1 = mktime(t1);
	time2 = mktime(t2);

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
