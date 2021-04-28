#include "./Methods.hpp"

void	Methods::_exitChild(int pipefd_in, int pipefd_out)
{
	close(pipefd_in);
	close(pipefd_out);
	exit(EXIT_FAILURE);
}

void	Methods::_closePipes(int * pipefd_in, int * pipefd_out)
{
	close(pipefd_in[0]);
	close(pipefd_in[1]);
	close(pipefd_out[0]);
	close(pipefd_out[1]);
	throw ServerBloc::InternalServerError();
}

void	Methods::_launchCGI(void)
{
	/* Pipe creation to communicate with the CGI program */
	int pipefd_in[2];
	int pipefd_out[2];
	if (pipe(pipefd_in) || pipe(pipefd_out))
	{
		CERR << "Error in pipe(): " << strerror(errno) << ENDL;
		_closePipes(pipefd_in, pipefd_out);
	}

	/* Set the pipe fds to non blocking */
	if (fcntl(pipefd_in[0], F_SETFL, O_NONBLOCK)
	|| fcntl(pipefd_in[1], F_SETFL, O_NONBLOCK)
	|| fcntl(pipefd_out[0], F_SETFL, O_NONBLOCK)
	|| fcntl(pipefd_out[1], F_SETFL, O_NONBLOCK))
	{
		CERR << "Error in fcntl(): " << strerror(errno) << ENDL;
		_closePipes(pipefd_in, pipefd_out);
	}

	/* Fork() the program for the CGI */
	pid_t pid;
	if ((pid = fork()) == -1)
	{
		CERR << "Error in fork(): " << strerror(errno) << ENDL;
		_closePipes(pipefd_in, pipefd_out);
	}
	else if (pid == 0)	/* Child program */
	{
		/* Closing parent's Fd duplicates */
		close(pipefd_in[1]);
		close(pipefd_out[0]);

		/* Array for execve env */
		_createEnvpMap();
		char ** envp = _createEnvpArray();
		if (envp == NULL)
			_exitChild(pipefd_in[0], pipefd_out[1]);

		/* Array for execve arguments */
		_createArgvMap();
		char ** argv = _createArgvArray();
		if (argv == NULL)
		{
			_freeArray(envp);
			_exitChild(pipefd_in[0], pipefd_out[1]);
		}

		/* Duplicating Fd for STDIN and STDOUT */
		if (dup2(pipefd_in[0], STDIN_FILENO) < 0	/* Lecture par le CGI dans fd_in[0] */
		|| dup2(pipefd_out[1], STDOUT_FILENO) < 0)	/* Ecriture par le CGI dans fd_out[1] */
		{
			CERR << "Error in dup2(): " << strerror(errno) << ENDL;
			_freeArray(envp);
			_freeArray(argv);
			_exitChild(pipefd_in[0], pipefd_out[1]);
		}
		/* Closing Child's Fd duplicates */
		close(pipefd_in[0]);
		close(pipefd_out[1]);

		/* Execve-ing */
		execve(_cgi_path.data(), argv, envp);
		CERR << "Error in execve(): " << strerror(errno) << ENDL;
		_freeArray(envp);
		_freeArray(argv);
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		exit(EXIT_FAILURE);
	}
	else	/* Parent program */
	{
		/* Closing Child's Fd duplicates */
		close(pipefd_in[0]);
		close(pipefd_out[1]);

		/* Going to communicate with CGI */
		_communicateWithCGI(pipefd_out[0], pipefd_in[1], pid);
	}
}

void	Methods::_communicateWithCGI(int fd_in, int fd_out, pid_t pid)
{
	bool	finishedWriting = 0;
	bool	finishedReading = 0;
	bool	CGIfinished = 0;

	Select	cgi;

	cgi.timeout.tv_sec = 1;
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
				int status = 0;
				if (waitpid(pid, &status, WNOHANG) == pid)
				{
					if (WIFSIGNALED(status))
					{
						CERR << "Child terminated with signal: " << WTERMSIG(status) << ENDL;
						close(fd_in);
						close(fd_out);
						throw ServerBloc::InternalServerError();
					}
					CGIfinished = true;
				}
				break ;
			}
			case -1:
			{
				CERR << "Error in select(): " << strerror(errno) << ENDL;
				close(fd_in);
				close(fd_out);
				throw ServerBloc::InternalServerError();
			}
			default:
			{
	/* STOP */	if (FD_ISSET(STDIN_FILENO, &cgi.readfds))	/* Keyboard was pressed, exiting server properly */
				{
					CERR << "CGI Select: Keyboard was pressed, killing CGI process properly" << ENDL;
					close(fd_in);
					close(fd_out);
					kill(pid, SIGKILL);
					return ;
				}
	/* WRITE */	else if (FD_ISSET(fd_out, &cgi.writefds))
				{
					if (_writeReqtoCGI(fd_out) == true)
					{
						if (client->clientClosed)
						{
							close(fd_out);
							close(fd_in);
							kill(pid, SIGKILL);
							return ;
						}
						close(fd_out);
						finishedWriting = 1;
					}
				}
	/* READ */	else if (FD_ISSET(fd_in, &cgi.readfds))
				{
					if (_readCGItoResp(fd_in) == true)
					{
						if (client->clientClosed)
						{
							close(fd_in);
							kill(pid, SIGKILL);
							return ;
						}
						close(fd_in);
						finishedReading = 1;
					}
				}
			}
		}
	}
	return ;
}

bool	Methods::_writeReqtoCGI(int & fd_out)
{
	ssize_t sentBytes = write(fd_out, &client->req.body.data()[_writtenBytes], client->req.body.length() - _writtenBytes);
	if (sentBytes < 0)
	{
		if (sentBytes < 0)
			CERR << "Error in write(): " << strerror(errno) << ENDL;
		client->clientClosed = true;
		return (true);
	}
	if ((_writtenBytes += static_cast<size_t>(sentBytes)) == client->req.body.length())
		return (true);
	return (false);
}

bool	Methods::_readCGItoResp(int & fd_in)
{
	char	recv_buffer[MAX_HEADER_SIZE];
	ssize_t receivedBytes = read(fd_in, &recv_buffer, MAX_HEADER_SIZE);

	if (receivedBytes <= 0)
	{
		if (receivedBytes == 0)
			_parseCGIResponse();
		_receivedMessage.clear();
		_receivedMessage.reserve();
		return (true);
	}
	_receivedMessage.append(recv_buffer, static_cast<size_t>(receivedBytes));
	_parseCGIResponse();
	return (false);
}

bool	Methods::_parseHeaderField(void)
{
	bool	lf = false;
	size_t	size;

	if ((size = _receivedMessage.find("\r\n")) == std::string::npos)
	{
		size = _receivedMessage.find("\n");
		lf = true;
	}
	if (size == 0)
	{
		(lf == true) ? _receivedMessage.erase(0, 1) : _receivedMessage.erase(0, 2);
		return (true);
	}

	size_t	pos = 0;
	bool	return_value = 0;
	size_t	first_osp = 0;

	if ((pos = _receivedMessage.find(":")) != std::string::npos)
	{
		if (_receivedMessage.find(": ", pos) == pos || _receivedMessage.find(":\t", pos) == pos)
			first_osp = 1;

		std::string key = _receivedMessage.substr(0, pos);

		if (client->req.strFindCaseinsensitive(key, "Status") == 0)
		{
			size_t	second_osp = 1;

			client->resp.status_code = _receivedMessage.substr(7 + first_osp, 3);
			if (10 + first_osp == size)
				second_osp = 0;
			client->resp.reason_phrase = _receivedMessage.substr(10 + first_osp + second_osp, size - 10 - first_osp - second_osp);

			if (!client->req.str_is(client->resp.status_code, client->req.ft_isdigit) || !client->req.str_is(client->resp.reason_phrase, client->req.ft_isprint))
				return_value = true;
		}
		else
		{
			if (client->resp.status_code.empty())
			{
				if (client->req.strFindCaseinsensitive(key, "Location") == 0)
				{
					client->resp.status_code = "301";
					client->resp.reason_phrase = "Found";
				}
				else if (client->req.strFindCaseinsensitive(key, "Content-Type") == 0)
				{
					client->resp.status_code = "200";
					client->resp.reason_phrase = "OK";
				}
			}
			client->resp.header_fields.insert(std::make_pair(key, _receivedMessage.substr(pos + 1 + first_osp, size - pos - 1 - first_osp)));
		}
	}
	_receivedMessage.erase(0, size + (lf == true ? 1 : 2));
	return (return_value);
}

void	Methods::_parseCGIResponse(void)
{
	size_t size = _receivedMessage.size();
	static bool	HeaderIncomplete = true;

	while (HeaderIncomplete && (_receivedMessage.find("\r\n") != std::string::npos || _receivedMessage.find("\n") != std::string::npos))
		HeaderIncomplete = !_parseHeaderField();

	if (!_receivedMessage.empty())
	{
		client->resp.body += _receivedMessage;
		_receivedMessage.clear();
	}
	else if (!size)
		HeaderIncomplete = true;
}

void	Methods::_createEnvpMap(void)
{
	std::stringstream tmp;

/* AUTH_TYPE */
	/* _checkAuthenticate already assign the correct value */
/* CONTENT_LENGTH */
	if (client->req.headers.find("Content-Length") != client->req.headers.end())
		_envp["CONTENT_LENGTH"] = client->req.headers.find("Content-Length")->second;
	else
		_envp["CONTENT_LENGTH"] = "";
/* CONTENT_TYPE */
	if (client->req.headers.find("Content-Type") != client->req.headers.end())
		_envp["CONTENT_TYPE"] = client->req.headers.find("Content-Type")->second;
	else
		_envp["CONTENT_TYPE"] = "";
/* GATEWAY_INTERFACE */
	_envp["GATEWAY_INTERFACE"] = "CGI/1.1";
/* PATH_INFO */
	_envp["PATH_INFO"] = client->req.uri;
/* PATH_TRANSLATED */
	_envp["PATH_TRANSLATED"] = _path;
/* QUERY_STRING */
	_envp["QUERY_STRING"] = _query;
/* REMOTE_ADDR */
	tmp << inet_ntoa(client->socket.address.sin_addr);
	_envp["REMOTE_ADDR"] = tmp.str();
	tmp.str("");
/* REMOTE_IDENT */
	tmp << ntohs(serv->serv_port.address.sin_port) << ", " << ntohs(client->socket.address.sin_port);
	if (_envp["REMOTE_USER"].empty())
		tmp << " : ERROR : HIDDEN-USER";
	else
		tmp << " : USERID : UNIX : " << _envp["REMOTE_USER"];
	_envp["REMOTE_IDENT"] = tmp.str();
	tmp.str("");
/* REMOTE_USER */
	/* _checkAuthenticate already assign the correct value */
/* REQUEST_METHOD */
	_envp["REQUEST_METHOD"] = client->req.method;
/* REQUEST_URI */
	_envp["REQUEST_URI"] = client->req.uri;
/* SCRIPT_NAME */
	_envp["SCRIPT_NAME"] = _cgi_path;
/* SERVER_NAME */
	if (serv->dir.find("server_name") != serv->dir.end())
		_envp["SERVER_NAME"] = serv->dir.find("server_name")->second[0];
	else
		_envp["SERVER_NAME"] = "";
/* SERVER_PORT */
	tmp << ntohs(serv->serv_port.address.sin_port);
	_envp["SERVER_PORT"] = tmp.str();
	tmp.str("");
/* SERVER_PROTOCOL */
	_envp["SERVER_PROTOCOL"] = "HTTP/1.1";
/* SERVER_SOFTWARE */
	_envp["SERVER_SOFTWARE"] = "webserv/1.0 (Unix)";

/* ADDITIONAL IMPLEMENTATION-DEFINED CGI HEADER FIELDS */
	for (std::map<std::string, std::string, ci_less>::iterator it = client->req.headers.begin(); it != client->req.headers.end(); ++it)
	{
		std::string result = "HTTP_" + it->first;
		result = client->req.transform(result, toupper);
		result = client->req.transform(result, client->req.tounderscore);
		_envp[result] = it->second;
	}
/* REDIRECT_STATUS */
	_envp["REDIRECT_STATUS"] = "1";
}

char **	Methods::_createEnvpArray(void)
{
	size_t	envp_size = 0;

	for (; serv->getParent()->getEnvp()[envp_size]; ++envp_size);
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
		++i;
		++begin;
	}
	for (int j = 0; serv->getParent()->getEnvp()[j]; ++j)
	{
		array[i] = strdup(serv->getParent()->getEnvp()[j]);
		if (array[i] == NULL)
		{
			CERR << "Error in strdup(): " << strerror(errno) << ENDL;
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
	_argv.push_back(_cgi_path.c_str());	/* Name of the program */
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

/* For debug purposes
void	Methods::_displayArray(char ** array)
{
	for (int i = 0; array[i]; ++i)
		COUT << "array[" << i << "]|" << array[i] << "|\n";
}
*/
