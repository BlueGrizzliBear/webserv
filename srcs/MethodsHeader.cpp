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

void	Methods::_lastModifiedHeader(void)
{
	struct stat			info;
	struct tm			timeinfo;
	struct timeval		time_val;
	struct timezone		tz;
	std::stringstream	s_str;

	char	date[30];

	if (!lstat(_path.c_str(), &info))
	{
		gettimeofday(&time_val, &tz);
		s_str << (info.st_mtime + tz.tz_minuteswest * 60 - tz.tz_dsttime * 60 * 60);
		if (strptime(s_str.str().c_str(), "%s", &timeinfo) != NULL)
		{
			strftime(date, 30, "%a, %d %b %Y %H:%M:%S GMT", &timeinfo);
			client->resp.header_fields.insert(std::make_pair("Last-Modified", date));
		}
	}
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
				weight = static_cast<float>(Request::ft_atof(language_range.substr(pos + 3).c_str()));
				language_range = language_range.substr(0, pos);
			}
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
				pos = language_range.find(";");
				size_t pos_after = pos;
				if (language_range.find(" ;") == pos - 1 || language_range.find("\t;") == pos - 1)
					pos -= 1;
				if (language_range.find("; q=") == pos_after + 1 || language_range.find(";\tq=") == pos_after + 1)
					pos_after += 1;

				weight = static_cast<float>(Request::ft_atof(language_range.substr(pos_after + 2).c_str()));

				language_range = language_range.substr(0, pos);
			}
		}
		(*storage)[weight].push_back(language_range);
	}
}
