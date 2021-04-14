#include "./Methods.hpp"

void	Methods::_launchCGI(void)
{
	// COUT << "Creating Pipes\n";
	/* Pipe creation to communicate with the CGI program */
	int pipefd_in[2];
	int pipefd_out[2];
	if (pipe(pipefd_in) || pipe(pipefd_out))
		serv->getParent()->abortServers("Error in pipe()", strerror(errno));

	/* Set the pipe fds to non blocking */
	fcntl(pipefd_in[0], F_SETFL, O_NONBLOCK);
	fcntl(pipefd_in[1], F_SETFL, O_NONBLOCK);
	fcntl(pipefd_out[0], F_SETFL, O_NONBLOCK);
	fcntl(pipefd_out[1], F_SETFL, O_NONBLOCK);

	// COUT << "Forking\n";
	/* Fork() the program for the CGI */
	pid_t pid;
	if ((pid = fork()) == -1)
		serv->getParent()->abortServers("Error in fork()", strerror(errno));
	/* Child program */
	else if (pid == 0)
	{
		close(pipefd_in[1]);
		close(pipefd_out[0]);

		// COUT << "Inside CHild\n";

		/* Array for execve env */
		_createEnvpMap();
		char ** envp = _createEnvpArray();
		if (envp == NULL)
			exit(EXIT_FAILURE);

		// COUT << "Creating Arguments\n";
		/* Array for execve arguments */
		_createArgvMap();
		char ** argv = _createArgvArray();
		if (argv == NULL)
		{
			_freeArray(envp);
			exit(EXIT_FAILURE);
		}

		// _displayArray(envp);
		// _displayArray(argv);

		// COUT << "Duping\n";
		/* Duplicating Fd for STDIN and STDOUT */
		if (dup2(pipefd_in[0], STDIN_FILENO) < 0 || close(pipefd_in[0])		/* Lecture par le CGI dans fd_in[0] */
		|| dup2(pipefd_out[1], STDOUT_FILENO) < 0 || close(pipefd_out[1]))	/* Ecriture par le CGI dans fd_out[1] */
		{
			CERR << "Error in dup2(): " << strerror(errno) << ENDL;
			_freeArray(envp);
			_freeArray(argv);
			exit(EXIT_FAILURE);
		}

		CERR << "Execve-ing\n";
		execve(_cgi_path.data(), argv, envp);
		CERR << "Error in execve(): " << strerror(errno) << ENDL;
		_freeArray(envp);
		_freeArray(argv);
		exit(EXIT_FAILURE);
	}
	/* Parent program */
	else
	{
		// COUT << "Inside Parent\n";
		close(pipefd_in[0]);
		close(pipefd_out[1]);

		COUT << "Parent: Going to communicate with CGI" << ENDL;
		_communicateWithCGI(pipefd_out[0], pipefd_in[1], pid);
	}
}

void	Methods::_communicateWithCGI(int fd_in, int fd_out, pid_t pid)
{
	std::string receivedMessage;
	bool	finishedWriting = 0;
	bool	finishedReading = 0;
	bool	CGIfinished = 0;

	int status = 0;
	Select	cgi;

	/* Pas de timeout */
	cgi.timeout.tv_sec = 0;
	cgi.timeout.tv_usec = 0;

	while (!(finishedWriting && finishedReading && CGIfinished))
	{
		FD_ZERO(&cgi.readfds);
		FD_ZERO(&cgi.writefds);

		FD_SET(STDIN_FILENO, &cgi.readfds);
		if (!finishedReading)
		{
			if (finishedWriting)
			{
				FD_SET(fd_in, &cgi.readfds);
				cgi.fd_max = fd_in;
			}
			else
			{
				FD_SET(fd_in, &cgi.readfds);
				FD_SET(fd_out, &cgi.writefds);
				cgi.fd_max = fd_in > fd_out ? fd_in : fd_out;
			}
		}

		switch (select(cgi.fd_max + 1, &cgi.readfds, &cgi.writefds, NULL, &cgi.timeout))
		{
			case 0:
			{
				if (waitpid(pid, &status, WNOHANG) == pid)
				{
					COUT << "Child was terminated";
					if (WIFEXITED(status))
						COUT << " normally with signal |" << WEXITSTATUS(status) << "|";
					else if (WIFSIGNALED(status))
						COUT << " ab-normally with signal |" << WTERMSIG(status) << "|";
					COUT << ENDL;
					CGIfinished = true;
				}
				break ;
			}
			case -1:
			{
				// COUT << "Error in select(): " << strerror(errno) << ENDL;
				break ;
			}
			default:
			{
	/* STOP */	if (FD_ISSET(STDIN_FILENO, &cgi.readfds))	/* Keyboard was pressed, exiting server properly */
				{
					COUT << "Keyboard was pressed, killing CGI properly\n";
					kill(pid, SIGKILL);
					close(fd_in);
					close(fd_out);
					return ;
				}
	/* WRITE */	else if (FD_ISSET(fd_out, &cgi.writefds))
				{
					// COUT << "FD is available to write\n";
					if (serv->resp.sendMsgCGI(fd_out, serv->req.body) == true)
					{
						// COUT << "Body to CGI|" << serv->req.body << "|\n";
						close(fd_out);
						finishedWriting = 1;
						COUT << "Sent EOF to CGI\n";
					}
				}
	/* READ */	else if (FD_ISSET(fd_in, &cgi.readfds))
				{
					// COUT << "FD is available to read\n";
					char	recv_buffer[MAX_HEADER_SIZE];

					ssize_t receivedBytes = read(fd_in, &recv_buffer, MAX_HEADER_SIZE);
					if (receivedBytes < 0)
					{
						close(fd_in);
						serv->getParent()->abortServers("Error in read()", strerror(errno));
					}
					else if (receivedBytes == 0)
					{
						_parseCGIResponse(receivedMessage);

						COUT << RED << "> CGI RESPONSE\n";
						COUT << "Status: " << serv->resp.status_code << " " << serv->resp.reason_phrase << ENDL;
						std::map<std::string, std::string, ci_less>::iterator begin = serv->resp.header_fields.begin();
						while (begin != serv->resp.header_fields.end())
						{
							COUT << begin->first + ": " + begin->second + "\r\n";
							begin++;
						}
						COUT << "Body first line|" << serv->resp.body.substr(0, 100) << "|\n";
						COUT << "Body Length |" << serv->resp.body.length() << "|\n";
						COUT << RESET;

						close(fd_in);
						finishedReading = 1;
					}
					else
					{
						receivedMessage.append(recv_buffer, static_cast<size_t>(receivedBytes));
						// COUT << "ReceivedBytes|" << receivedMessage << "|\n";
						_parseCGIResponse(receivedMessage);
					}
				}
			}
		}
	}
	COUT << "CGI finished\n";
	return ;
}

bool	Methods::_str_is(std::string str, int func(int))
{
	std::string::iterator begin = str.begin();

	while (begin != str.end())
	{
		if (!func(*begin))
			return (false);
		++begin;
	}
	return (true);
}

bool	Methods::_parseHeaderField(std::string & receivedMessage)
{
	size_t size = receivedMessage.find("\r\n");

	if (size == 0)
	{
		receivedMessage.erase(0, 2);
		return (true);
	}

	size_t	pos = 0;
	bool	return_value = 0;
	size_t	osp = 0;
	if ((pos = receivedMessage.find(":")) != std::string::npos)
	{
		if (receivedMessage.find(": ", pos) == pos)
			osp = 1;

		std::string key = receivedMessage.substr(0, pos);

		if (serv->req.strFindCaseinsensitive(key, "Status") == 0)
		{
			serv->resp.status_code = receivedMessage.substr(7 + osp, 3);
			serv->resp.reason_phrase = receivedMessage.substr(11 + osp, size - 11 - osp);
			if (!_str_is(serv->resp.status_code, isdigit) || !_str_is(serv->resp.reason_phrase, isprint))
				return_value = true; // Status value is incorrect - return true to finish parsing
		}
		else
		{
			if (serv->req.strFindCaseinsensitive(key, "Location") == 0) // a implementer davantage
			{
				serv->resp.status_code = "301";
				serv->resp.reason_phrase = "Found";
			}
			else if (serv->req.strFindCaseinsensitive(key, "Content-Type") == 0)
			{
				serv->resp.status_code = "200";
				serv->resp.reason_phrase = "OK";
			}
			serv->resp.header_fields.insert(std::make_pair(key, receivedMessage.substr(pos + 1 + osp, size - pos - 1 - osp)));
		}
	}
	receivedMessage.erase(0, size + 2);
	return (return_value);
}

void	Methods::_parseCGIResponse(std::string & receivedMessage)
{
	size_t size = receivedMessage.size();
	static bool	HeaderIncomplete = true;

	while (HeaderIncomplete && receivedMessage.find("\r\n") != std::string::npos)
		HeaderIncomplete = !_parseHeaderField(receivedMessage);

	if (!receivedMessage.empty())
	{
		serv->resp.body += receivedMessage;
		receivedMessage.clear();
	}
	else if (!size)
		HeaderIncomplete = true;
}

void	Methods::_createEnvpMap(void)
{
	std::stringstream tmp;

// AUTH_TYPE
	/* _checkAuthenticate already assign the correct value */
// CONTENT_LENGTH
	if (serv->req.headers.find("Content-Length") != serv->req.headers.end())
		_envp["CONTENT_LENGTH"] = serv->req.headers.find("Content-Length")->second;
	else
		_envp["CONTENT_LENGTH"] = "";
// CONTENT_TYPE
	if (serv->req.headers.find("Content-Type") != serv->req.headers.end())
		_envp["CONTENT_TYPE"] = serv->req.headers.find("Content-Type")->second;
	else
		_envp["CONTENT_TYPE"] = "";
// GATEWAY_INTERFACE
	_envp["GATEWAY_INTERFACE"] = "CGI/1.1";
// PATH_INFO
	_envp["PATH_INFO"] = serv->req.uri;	// SUFFIXE DE L'URI UNIQUEMENT?
// PATH_TRANSLATED
	_envp["PATH_TRANSLATED"] = _path;
// QUERY_STRING
	_envp["QUERY_STRING"] = _query;	/* put the search identifier in the uri if any (query-string part of the uri) */
// REMOTE_ADDR
	tmp << inet_ntoa(serv->req.client->address.sin_addr);
	_envp["REMOTE_ADDR"] = tmp.str();	/* Get client IP adress */
	tmp.str("");
// REMOTE_IDENT
	// A VERIFIER
	tmp << ntohs(serv->serv_port.address.sin_port) << ", " << ntohs(serv->req.client->address.sin_port);
	if (_envp["REMOTE_USER"].empty())
		tmp << " : ERROR : HIDDEN-USER";	//  6195, 23 : ERROR : NO-USER
	else
		tmp << " : USERID : UNIX : " << _envp["REMOTE_USER"];
	_envp["REMOTE_IDENT"] = tmp.str();
	tmp.str("");
// REMOTE_USER
	/* _checkAuthenticate already assign the correct value */
// REQUEST_METHOD
	_envp["REQUEST_METHOD"] = serv->req.method;
// REQUEST_URI
	_envp["REQUEST_URI"] = serv->req.uri; /* Check with PATH_INFO */
// SCRIPT_NAME
	_envp["SCRIPT_NAME"] = _cgi_path;
// SERVER_NAME
	// A VERIFIER
	if (serv->dir.find("server_name") != serv->dir.end())
		_envp["SERVER_NAME"] = serv->dir.find("server_name")->second[0];
	else
		_envp["SERVER_NAME"] = "";
// SERVER_PORT
	tmp << ntohs(serv->serv_port.address.sin_port);
	_envp["SERVER_PORT"] = tmp.str();
	tmp.str("");
// SERVER_PROTOCOL
	_envp["SERVER_PROTOCOL"] = "HTTP/1.1";
// SERVER_SOFTWARE
	_envp["SERVER_SOFTWARE"] = "HuntGaming/1.0";

// ADDITIONAL IMPLEMENTATION-DEFINED CGI HEADER FIELDS
	for (std::map<std::string, std::string, ci_less>::iterator it = serv->req.headers.begin(); it != serv->req.headers.end(); ++it)
	{
		if (serv->req.strFindCaseinsensitive(it->first.substr(0, 2), "X-") != std::string::npos)
		{
			std::string result = "HTTP_" + it->first;
			result = serv->req.transform(result, toupper);
			result = serv->req.transform(result, serv->req.tounderscore);
			_envp[result] = it->second;
		}
	}
// REDIRECT_STATUS
	_envp["REDIRECT_STATUS"] = "1";
}

char **	Methods::_createEnvpArray(void)
{
	size_t	envp_size = 0;

	for (; serv->getParent()->envp[envp_size]; ++envp_size);
	char ** array = static_cast<char **>(malloc(sizeof(char *) * (_envp.size() + envp_size + 1)));
	if (array == NULL)
	{
		CERR << "Error in malloc(): " << strerror(errno) << ENDL;
		return (NULL);
	}
	array[_envp.size() + envp_size] = 0;

	std::map<std::string, std::string, ci_less>::iterator begin = _envp.begin();
	int i = 0;

	while (begin != _envp.end())
	{
		std::string	tmp = (*begin).first + "=" + (*begin).second;
		array[i] = static_cast<char *>(malloc(sizeof(char) * (tmp.size() + 1)));
		if (array[i] == NULL)
		{
			CERR << "Error in malloc(): " << strerror(errno) << ENDL;
			while (--i > 0)
				free(array[i]);
			free(array);
			return (NULL);
		}
		array[i][tmp.size()] = 0;
		tmp.copy(array[i], tmp.size(), 0);
		// COUT << "lign #" << i << "|" << array[i] << "|" << ENDL;
		++i;
		++begin;
	}
	for (int j = 0; serv->getParent()->envp[j]; ++j)
	{
		array[i] = strdup(serv->getParent()->envp[j]);
		if (array[i] == NULL)
		{
			CERR << "Error in dup(): " << strerror(errno) << ENDL;
			while (--i > 0)
				free(array[i]);
			free(array);
			return (NULL);
		}
		++i;
	}
	return (array);
}

void	Methods::_createArgvMap(void)
{
	/* Name of the program */
	_argv.push_back(_cgi_path.c_str());
}

char **	Methods::_createArgvArray(void)
{
	char ** array = static_cast<char **>(malloc(sizeof(char *) * (_argv.size() + 1)));
	if (array == NULL)
	{
		CERR << "Error in malloc(): " << strerror(errno) << ENDL;
		return (NULL);
	}
	array[_argv.size()] = 0;

	std::vector<std::string>::iterator begin = _argv.begin();
	int i = 0;

	while (begin != _argv.end())
	{
		array[i] = static_cast<char *>(malloc(sizeof(char) * ((*begin).size() + 1)));
		if (array[i] == NULL)
		{
			CERR << "Error in malloc(): " << strerror(errno) << ENDL;
			while (--i > 0)
				free(array[i]);
			free(array);
			return (NULL);
		}
		array[i][(*begin).size()] = 0;
		(*begin).copy(array[i], (*begin).size(), 0);
		// COUT << "lign #" << i << "|" << array[i] << "|" << ENDL;
		++i;
		++begin;
	}
	return (array);
}

void	Methods::_freeArray(char ** array)
{
	for (int i = 0; array[i]; ++i)
		free(array[i]);
	free(array);
}

void	Methods::_displayArray(char ** array)
{
	for (int i = 0; array[i]; ++i)
		COUT << "array[" << i << "]|" << array[i] << "|\n";
}
