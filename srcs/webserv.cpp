# include "./webserv.hpp"
# include "./ConfigParser.hpp"

void	displayError(const char * main_err, const char * err)
{
	COUT << main_err << ": ";
	COUT << err << ENDL;
}

// void	exitServerOnError(const char * main_err, const char * err, ServerBloc & server, int client_socket)
// {
// 	displayError(main_err, err);
// 	FD_ZERO(&server.serv_select.readfds);
// 	FD_ZERO(&server.serv_select.writefds);
// 	FD_ZERO(&server.serv_select.exceptfds);
// 	close(client_socket);
// 	close(server.serv_port.fd);
// 	exit(EXIT_FAILURE);
// }

bool	parseClientRequest(ServerBloc & server, Client & client)
{
	try
	{
		/* Read Client Request with recv */
		if (server.readClient(client))
		{
			// std::cerr << "Displaying header|" << GREEN;
			// std::cerr << client.req.getData().substr(0, client.req.getData().find("\r\n\r\n") + 4);
			// std::cerr << RESET << "|" << std::endl;

			// std::cerr << "Displaying all data|" << GREEN << client.req.getData() << RESET << "|" << std::endl;

			// std::cerr << "Displaying data.length|" << GREEN << client.req.getData().length() << RESET << "|" << std::endl;

			if (client.clientClosed)
				return (true);

			/* Parse Client Request first */
			if (server.processRequest(client))
				return (true);
		}
	}
	catch(const std::exception & e)
	{
		/* Catching exception from parsing request or execute request */
		std::cerr << RED << e.what() << RESET << std::endl; // Display Exception what() for debug
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

void	displayDebug(const char * str, int request_no)
{
	static int mystatic = -1;
	bool yes = 0;

	if (!strcmp(str, "I"))
	{
		yes = (mystatic != 0) ? 1 : 0;
		mystatic = 0;
	}
	else if (!strcmp(str, "R"))
	{
		yes = (mystatic != 1) ? 1 : 0;
		mystatic = 1;
	}
	else if (!strcmp(str, "W"))
	{
		yes = (mystatic != 2) ? 1 : 0;
		mystatic = 2;
	}
	else if (!strcmp(str, "Reading"))
	{
		yes = (mystatic != 3) ? 1 : 0;
		mystatic = 3;
	}
	else if (!strcmp(str, "Writing"))
	{
		yes = (mystatic != 4) ? 1 : 0;
		mystatic = 4;
	}
	else if (!strcmp(str, "New"))
	{
		yes = (mystatic != 5) ? 1 : 0;
		mystatic = 5;
	}
	else if (!strcmp(str, "Time Out"))
	{
		yes = (mystatic != 6) ? 1 : 0;
		mystatic = 6;
	}
	else if (!strcmp(str, "Read or Write"))
	{
		yes = (mystatic != 7) ? 1 : 0;
		mystatic = 7;
	}

	if (yes == 1)
		CERR << str << ENDL;
	// else if (strcmp(str, "Time Out"))
	else
	{
		CERR << "\r" << str << " " << request_no << ENDL;
	}
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
	if (server.is_default)
	{
		// COUT << "Server #" << server.getNo() << ENDL;
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
				// displayDebug("Time Out", -1);
				break ;
			}
			case -1:
			{
				displayError("Error in Select()", strerror(errno));
				break ;
			}
			default:
			{
				/* Keyboard was pressed, exiting server properly */
	/* STOP */	if (FD_ISSET(STDIN_FILENO, &server.serv_select.readfds))
				{
					COUT << "Keyboard was pressed, exiting server properly\n";
					FD_ZERO(&server.serv_select.readfds);
					FD_ZERO(&server.serv_select.writefds);
					FD_ZERO(&server.serv_select.exceptfds);
					for (std::list<Client>::iterator it = server.clientList.begin(); it != server.clientList.end(); ++it)
						close (it->socket.fd);
					close(server.serv_port.fd);
					// server.getParent()->abortServers("Aborting", "");
					exit(EXIT_SUCCESS);
				}
	/* NEW */	else if (FD_ISSET(server.serv_port.fd, &server.serv_select.readfds))
				{
					static int i = 0;
					COUT << GREEN << "Request #" << ++i << RESET << ENDL;
					// displayDebug("New", i);

					/* Opening socket for new client */
					Client new_client;

					// new_client.request_no = i;
					new_client.socket.fd = accept(server.serv_port.fd, reinterpret_cast<struct sockaddr *>(&new_client.socket.address), reinterpret_cast<socklen_t *>(&new_client.socket.addrlen));
					if (new_client.socket.fd == -1)
					{
						displayError("Error in accept()", strerror(errno));
						break ;
					}
					fcntl(new_client.socket.fd, F_SETFL, O_NONBLOCK);	/* Set the socket to non blocking */

					new_client.finishedReading = 0;
					new_client.clientClosed = 0;

					server.clientList.push_back(new_client);
				}
	/* R | W */	else if (!server.clientList.empty())
				{
					for (std::list<Client>::iterator it = server.clientList.begin(); it != server.clientList.end(); it++)
					{
			/* READ */	if (FD_ISSET(it->socket.fd, &server.serv_select.readfds))
						{
							// displayDebug("Reading", it->request_no);
							if (parseClientRequest(server, *it))	/* Parsing Client Request */
							{
								if (it->clientClosed)
								{
									COUT << MAGENTA << "Closing prematurely Client" << RESET << ENDL;
									close(it->socket.fd);
									server.clientList.erase(it--);
								}
								else
									it->finishedReading = 1;
							}
							break;
						}
			/* WRITE */	else if (FD_ISSET(it->socket.fd, &server.serv_select.writefds))
						{
							// displayDebug("Writing", it->request_no);
							if (parseServerResponse(server, *it))	/* Parsing Server Response */
							{
								close(it->socket.fd);
								server.clientList.erase(it--);
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
			std::string path = (argc == 1 ? "./configuration/default.conf" : argv[1]);
			ConfigParser	config(path.c_str(), envp);
			CME << "Parsing Complete !" << EME;
			// config.display_config();

			CME << "Launching All Servers . . ." << EME;
			while (1)
				std::for_each(config.getServers().begin(), config.getServers().end(), selectServer);

			CME << "All servers came back . . ." << EME;
		}
		catch(const std::exception& e)
		{
			std::cerr << RED << e.what() << RESET << std::endl;
		}
	}
	else
		COUT << "Incorrect argument number" << ENDL;
    return 0;
}
