#include "./ServerBloc.hpp"

/* ServerBloc Class Declaration */
/* Constructor */
/*	default		(1)	*/
ServerBloc::ServerBloc(void) : port_no(0), is_default(false), is_unique(true), pid(0), _server_no(0), _parent(NULL) {}

/*	default		(1)	*/
ServerBloc::ServerBloc(ConfigParser * parent) : port_no(0), is_default(false), is_unique(true), pid(0), _server_no(0), _parent(parent) {}

/*	copy		(2)	*/
ServerBloc::ServerBloc(ServerBloc const & cpy)
{
	*this = cpy;
}

/* Destructor */
ServerBloc::~ServerBloc() {}

/* Operators */
ServerBloc &	ServerBloc::operator=(ServerBloc const & rhs)
{
	dir = rhs.dir;
	loc = rhs.loc;

	port_no = rhs.port_no;
	is_default = rhs.is_default;
	is_unique = rhs.is_unique;

	serv_port = rhs.serv_port;
	serv_select = rhs.serv_select;

	pid = rhs.pid;
	
	_server_no = rhs._server_no;
	_parent = rhs._parent;
	return (*this);
}

/* Getters and Setters */
size_t &	ServerBloc::getNo(void)
{
	return (_server_no);
}

ConfigParser *	ServerBloc::getParent(void)
{
	return (_parent);
}

/* Member Functions */
void	ServerBloc::parseException(Client & client, const char * code)
{
	std::string str(code);
	std::map<std::string, std::string>::iterator it = _parent->getDictionary().errorDic.find(str);

	if (it != _parent->getDictionary().errorDic.end())
	{
		/* Fill Status Line */
		client.resp.status_code = it->first;
		client.resp.reason_phrase = it->second;

		Methods	implementedMethods(*this, client, client.resp.status_code, client.resp.reason_phrase);
		client.req.clear();	/* Clean Client Request - need to answer client now */
	}
}

bool	ServerBloc::readClient(Client & client)
{
	char	recv_buffer[MAX_HEADER_SIZE];

	ssize_t receivedBytes = recv(client.socket.fd, &recv_buffer, MAX_HEADER_SIZE, MSG_DONTWAIT);

	if (receivedBytes <= 0)
	{
		if (receivedBytes < 0)
			CERR << "Error in recv(): " << strerror(errno) << ENDL;
		else
			CERR << "Error in recv(): client closed connection" << ENDL;
		client.clientClosed = true;
		return (true);
	}

	size_t old_pos = client.req.getData().size() > 4 ? client.req.getData().size() - 4 : 0;

	client.req.getData().append(recv_buffer, static_cast<size_t>(receivedBytes));

	if (client.req.headerComplete)	/* Headers seems complete */
		return (true);
	else if ((client.req.getData().find("\r\n\r\n", old_pos) == std::string::npos))
		return (false); /* Not found ending sequence */
	client.req.headerComplete = 1;
	return (true);
}

bool	ServerBloc::processRequest(Client & client)
{
	if (!client.req.headerParsed)
	{
		client.req.parseRequestLine();
		client.req.parseHeaders();
		if (client.req.headers.find("Host") == client.req.headers.end()
		|| !client.req.str_is(client.req.headers.find("Host")->second, client.req.isValidHost))
			throw BadRequest();
		client.req.headerParsed = true;

		client.req.getData().erase(0, client.req.getData().find("\r\n\r\n") + 4);
		client.req.getPos() = 0;
	}
	if (client.req.parseBody())
	{
		client.finishedReading = 1;

		/* Cleaning */
		client.req.getData().clear();			/* Clearing _req buffer */
		client.req.getData().reserve();

		client.req.headerComplete = false;		/* Reseting bool indicator if header is complete or not */
		client.req.headerParsed = false;		/* Reseting bool indicator if header is parsed or not */

		// COUT << MAGENTA << "Avant Exec" << RESET << ENDL;
		

		ServerBloc * ptr = NULL;
		if (!is_unique && client.req.headers.find("Host") != client.req.headers.end())
		{
			bool found = 0;
			std::string server_name = client.req.headers.find("Host")->second.substr(0, client.req.headers.find("Host")->second.rfind(":"));

			ConfigParser::Servers::iterator it = getParent()->getServers().begin();

			while (!found && it != getParent()->getServers().end())
			{
				if (it->port_no == port_no)
				{
					if (!it->is_default && it->dir.find("server_name") != it->dir.end())
					{
						std::vector<std::string>::iterator vec_it = it->dir.find("server_name")->second.begin();
						std::vector<std::string>::iterator vec_ite = it->dir.find("server_name")->second.end();

						while (!found && vec_it != vec_ite)
						{
							if (*vec_it == server_name)
							{
								ptr = &(*it);
								found = true;
							}
							vec_it++;
						}
					}
					else
						ptr = &(*it);
				}
				it++;
			}
		}
		else
			ptr = this;

		// /* Execute the parsed request */
		Methods	implementedMethods(*ptr, client);
		implementedMethods.execute();

		// COUT << MAGENTA << "AFter Exec" << RESET << ENDL;

		client.req.clear();	/* Clean Client Request - need to answer client now */

		return (true);
	}
	return (false);
}

void	ServerBloc::_addHeaderFields(Client & client)
{
	/* Create corresponding header fields */
	client.resp.header_fields.insert(std::make_pair("Date", _getDate()));
	client.resp.header_fields.insert(std::make_pair("Server", dir.find("server_name")->second[0]));

	/* Cache indications */
	client.resp.header_fields.insert(std::make_pair("Cache-Control", "no-store"));
	// resp.header_fields.insert(std::make_pair("Cache-Control", "max-age=10"));

	client.resp.header_fields.insert(std::make_pair("Connection", "close"));
}

bool	ServerBloc::sendResponse(Client & client)
{
	if (!client.resp.isComplete)
	{
		_addHeaderFields(client);	/* Add the right header fields */
		client.resp.concatenateResponse();	/* Fill response msg */
	}
	// COUT << "Sending to Client|" << client.request_no << "|\n";
	if (client.resp.sendResptoClient(client) == true)
	{
		// COUT << "Cleaning now\n";
		client.resp.cleanResponse();
		return (true);
	}
	return (false);
}

std::string	ServerBloc::_getDate(void)
{
	struct timeval	time_val;
	struct tm		*time;
	char			buffer[30];

	gettimeofday(&time_val, NULL);
	time = gmtime(&time_val.tv_sec); // to check (time_t to struct tm)
	strftime(buffer, 30, "%a, %d %b %Y %H:%M:%S GMT", time);
	std::string	str(buffer);
	return (str);
}
