# include "./webserv.hpp"
# include "./ConfigParser.hpp"

bool	parseClientRequest(ServerBloc & server, Client & client)
{
	try
	{
		if (server.readClient(client))	/* Read Client Request with recv */
		{
			if (client.clientClosed)
				return (true);
			if (server.processRequest(client))	/* Parse Client Request first */
				return (true);
		}
	}
	catch(const std::exception & e)	/* Catching exception from parsing request or execute request */
	{
		server.parseException(client, e.what());
		return (true);	/* send true to execute the error msg to Response */
	}
	return (false);
}

bool	parseServerResponse(ServerBloc & server, Client & client)
{
	if (server.sendResponse(client))
		return (true);
	return (false);
}

int	getMaxFd(ServerBloc & server)
{
	int result = server.serv_port.fd;

	for (std::list<Client>::iterator it = server.clientList.begin(); it != server.clientList.end(); ++it)
	{
		if (it->socket.fd > result)
			result = it->socket.fd;
	}
	return (result);
}

void	selectServer(ServerBloc & server)
{
	static int maxSize = 1;

	if (server.is_default)
	{
		server.serv_select.timeout.tv_sec = 0;
		server.serv_select.timeout.tv_usec = 0;

		FD_ZERO(&server.serv_select.readfds);
		FD_ZERO(&server.serv_select.writefds);

		FD_SET(STDIN_FILENO, &server.serv_select.readfds);
		FD_SET(server.serv_port.fd, &server.serv_select.readfds);
		server.serv_select.fd_max = server.serv_port.fd;

		for (std::list<Client>::iterator it = server.clientList.begin(); it != server.clientList.end(); it++)
		{
			if (it->finishedReading)
				FD_SET(it->socket.fd, &server.serv_select.writefds);
			else
				FD_SET(it->socket.fd, &server.serv_select.readfds);
			if (server.serv_select.fd_max < it->socket.fd)
				server.serv_select.fd_max = it->socket.fd;
		}

		switch (select(server.serv_select.fd_max + 1, &server.serv_select.readfds, &server.serv_select.writefds, NULL, &server.serv_select.timeout))
		{
			case 0:
			{
				break ;
			}
			case -1:
			{
				break ;
			}
			default:
			{
	/* STOP */	if (FD_ISSET(STDIN_FILENO, &server.serv_select.readfds))	/* Keyboard was pressed, exiting server properly */
				{
					CERR << "IO Select: Keyboard was pressed, exiting server properly" << ENDL;
					FD_ZERO(&server.serv_select.readfds);
					FD_ZERO(&server.serv_select.writefds);
					FD_ZERO(&server.serv_select.exceptfds);
					for (std::list<Client>::iterator it = server.clientList.begin(); it != server.clientList.end(); ++it)
						close (it->socket.fd);
					close(server.serv_port.fd);
					server.getParent()->~ConfigParser();
					exit(EXIT_SUCCESS);
				}
	/* NEW */	else if (server.totalClients < maxSize && FD_ISSET(server.serv_port.fd, &server.serv_select.readfds))
				{
					static bool hasCapped = false;

					/* Opening socket for new client */
					Client new_client;
					new_client.socket.addrlen = sizeof(struct sockaddr_in);
					new_client.finishedReading = 0;
					new_client.clientClosed = 0;

					new_client.socket.fd = accept(server.serv_port.fd, reinterpret_cast<struct sockaddr *>(&new_client.socket.address), reinterpret_cast<socklen_t *>(&new_client.socket.addrlen));
					if (new_client.socket.fd == -1)
					{
						CERR << "Error in accept(): " << strerror(errno) << ENDL;
						break ;
					}
					server.totalClients++;
					if (new_client.socket.fd < FD_SETSIZE - 5) /* corresponding to 4 fd used for CGI implementations */
					{
						if (hasCapped == false)
							maxSize = server.totalClients + 1;
					}
					else
					{
						hasCapped = true;
						maxSize = server.totalClients;
					}
					server.clientList.push_back(new_client);
				}
	/* R | W */	else if (!server.clientList.empty())
				{
					for (std::list<Client>::iterator it = server.clientList.begin(); it != server.clientList.end(); it++)
					{
			/* READ */	if (FD_ISSET(it->socket.fd, &server.serv_select.readfds))
						{
							if (parseClientRequest(server, *it))	/* Parsing Client Request */
							{
								if (it->clientClosed)	/* Client closed prematurely socket */
								{
									close(it->socket.fd);
									server.clientList.erase(it--);
									server.totalClients--;
								}
								else
									it->finishedReading = 1;
							}
							break;
						}
			/* WRITE */	else if (FD_ISSET(it->socket.fd, &server.serv_select.writefds))
						{
							if (parseServerResponse(server, *it))	/* Parsing Server Response */
							{
								close(it->socket.fd);
								server.clientList.erase(it--);
								server.totalClients--;
							}
							break;
						}
					}
				}
				break ;
			}
		}
	}
}

int main(int argc, char const ** argv, char const ** envp)
{
	errno = 0;
	if (argc == 1 || argc == 2)
	{
		try
		{
			ConfigParser	config((argc == 1 ? "./configuration/default.conf" : argv[1]), envp);

			CERR << "> Launching All Servers . . ." << ENDL;
			while (1)
				std::for_each(config.getServers().begin(), config.getServers().end(), selectServer);
		}
		catch(const std::exception& e)
		{
			std::cerr << RED << e.what() << RESET << std::endl;
		}
	}
	else
		CERR << "Error: Incorrect argument number" << ENDL;
    return 0;
}
