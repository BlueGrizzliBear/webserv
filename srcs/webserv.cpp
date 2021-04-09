# include "./webserv.hpp"
# include "./ConfigParser.hpp"

void	displayError(const char * main_err, const char * err)
{
	COUT << main_err << ": ";
	COUT << err << ENDL;
}

void	exitServerOnError(const char * main_err, const char * err, ServerBloc & server, int client_socket)
{
	displayError(main_err, err);
	FD_ZERO(&server.serv_select.readfds);
	FD_ZERO(&server.serv_select.writefds);
	FD_ZERO(&server.serv_select.exceptfds);
	close(client_socket);
	close(server.serv_port.fd);
	exit(EXIT_FAILURE);
}

bool	parseClientRequest(ServerBloc & server, Socket & client)
{
	try
	{
		/* Read Client Request with recv */
		if (server.readClient(client.fd))
		{
			/* Displaying Client request */
			// COUT << "Received Data from client\n";
			// std::cerr << "Displaying length|" << GREEN << server.req.getData().length() << RESET << "|" << std::endl;

			// std::cerr << "Displaying header|" << GREEN;
			// for (size_t i = 0; i != server.req.getData().find("\r\n\r\n") + 4; i++)
			// 	std::cerr << server.req.getData()[i];
			// std::cerr << RESET << "|" << std::endl;

			// std::cerr << "Displaying all data|" << GREEN << server.req.getData() << RESET << "|" << std::endl;

			/* Parse Client Request first */
			if (server.processRequest(client))
				return (true);
		}
	}
	catch(const std::exception & e)
	{
		/* Catching exception from parsing request or execute request */
		std::cerr << RED << e.what() << RESET << std::endl; // Display Exception what() for debug
		server.parseException(e.what());
		return (true);	/* send true to execute the error msg to Response */
	}
	return (false);
}

bool	parseServerResponse(ServerBloc & server, Socket & client)
{
	try
	{
		if (server.sendResponse(client))
			return (true);
	}
	catch(const std::exception& e)
	{
		std::cerr << RED << e.what() << RESET << '\n';
	}
	return (false);
}

void	displayDebug(const char * str)
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

	if (yes == 1)
		CERR << str << ENDL;
}

int	getMaxFd(ServerBloc & server)
{
	int result = server.serv_port.fd;

	for (std::list<Socket>::iterator it = server.clientList.begin(); it != server.clientList.end(); ++it)
	{
		if (result < it->fd)
			result = it->fd;
	}
	return (result);
}

int	launchServer(ServerBloc & server)
{
	static int i = 0;
	static float t = 0.;

    while (1)
    {
		FD_ZERO(&server.serv_select.readfds);
		FD_ZERO(&server.serv_select.writefds);

		FD_SET(STDIN_FILENO, &server.serv_select.readfds);
		FD_SET(server.serv_port.fd, &server.serv_select.readfds);

		for (std::list<Socket>::iterator it = server.clientList.begin(); it != server.clientList.end(); ++it)
		{
			if (it->finishedReading)
				FD_SET(it->fd, &server.serv_select.writefds);
			else
				FD_SET(it->fd, &server.serv_select.readfds);
		}

		server.serv_select.fd_max = getMaxFd(server);

		/* Setting time-out */
		server.serv_select.timeout.tv_sec = 3.0;
		server.serv_select.timeout.tv_usec = 0.0;


		/* ------ Listening to sockets . . . ----------------------- */
		int recVal = 0;

		recVal = select(server.serv_select.fd_max + 1, &server.serv_select.readfds, &server.serv_select.writefds, NULL, &server.serv_select.timeout);
		switch (recVal)
		{
			case 0:
			{
				// usleep(10000);
				displayDebug("Time Out");
				break ;
			}

			// lire et ecrire en meme temps pour le body dans le 1er select

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
					for (std::list<Socket>::iterator it = server.clientList.begin(); it != server.clientList.end(); ++it)
						close (it->fd);
					close(server.serv_port.fd);
					// server.getParent()->abortServers("Aborting", "");
					// while (1);
					exit(EXIT_SUCCESS);
				}
 /* R OR W */	else if (!server.clientList.empty())
				{
					// displayDebug("Read or Write");
					for (std::list<Socket>::iterator it = server.clientList.begin(); it != server.clientList.end(); ++it)
					{
			/* READ */	if (FD_ISSET(it->fd, &server.serv_select.readfds))
						{
							// displayDebug("Reading");
							if (parseClientRequest(server, *it))	/* Parsing Client Request */
							{
								// FD_CLR(it->fd, &server.serv_select.readfds);		/* Clearing read list from client socket */
								// FD_SET(it->fd, &server.serv_select.writefds);		/* Adding the client socket to the write playlist */
								it->finishedReading = 1;
							}
							break;
						}
			/* WRITE */	else if (FD_ISSET(it->fd, &server.serv_select.writefds))
						{
							// displayDebug("Writing");
							// if ((finishedWriting = parseServerResponse(server, *it)))	/* Parsing Server Response */
							if (parseServerResponse(server, *it))	/* Parsing Server Response */
							{
								it->finishedReading = 0;
								// FD_CLR(it->fd, &server.serv_select.writefds);		/* Clearing read list from client socket */

								if (i > 96)
								{
									gettimeofday(&it->mytime2, NULL);
									if (t < static_cast<float>((it->mytime2.tv_sec - it->mytime1.tv_sec) * 1000 + ((it->mytime2.tv_usec - it->mytime1.tv_usec) / 1000)))
									{
										t = static_cast<float>((it->mytime2.tv_sec - it->mytime1.tv_sec) * 1000 + ((it->mytime2.tv_usec - it->mytime1.tv_usec) / 1000));
										COUT << RED << "Time elapsed: " << t << " ms." << RESET << ENDL;
									}
								}

								close(it->fd);
								server.clientList.erase(it);
							}
							break;
						}
					}
				}
	/* NEW */	else if (FD_ISSET(server.serv_port.fd, &server.serv_select.readfds))
				{
					// displayDebug("New");
					i++;
					// COUT << GREEN << "Request #" << i++ << RESET << ENDL;

					/* Opening socket for new client */
					Socket new_client;

					new_client.fd = accept(server.serv_port.fd, reinterpret_cast<struct sockaddr *>(&new_client.address), reinterpret_cast<socklen_t *>(&new_client.addrlen));
					if (new_client.fd == -1)
					{
						displayError("Error in accept()", strerror(errno));
						break ;
					}
					fcntl(new_client.fd, F_SETFL, O_NONBLOCK);	/* Set the socket to non blocking */

					new_client.finishedReading = 0;
					
					server.clientList.push_back(new_client);

					if (i > 96)
						gettimeofday(&new_client.mytime1, NULL); // to display the time consumption of request

					// FD_SET(new_client.fd, &server.serv_select.readfds);			/* Adding the respective socket to the read playlist */
				}
				else
					COUT << "WTF\n";
				break ;
			}
		}
    }
	CME << "Error in the Server: exiting now" << EME;
	exit(1);
}

int	initServer(ServerBloc & server)
{
	/* Fork() the program for each server bloc */
	if ((server.pid = fork()) == -1)
		server.getParent()->abortServers("Error in fork()", strerror(errno));
	/* Child program */
	else if (server.pid == 0)
	{
		CME << "Launching Server #" << server.getNo() <<  ". . ." << EME;
		launchServer(server);
	}
	/* Parent program */
	else
	{
	}
	return (0);
}

int main(int argc, char const ** argv)
{
	if (argc == 1 || argc == 2)
	{
		try
		{
			std::string path = (argc == 1 ? "./configuration/default.conf" : argv[1]);
			ConfigParser	config(path.c_str());
			CME << "Parsing Complete !" << EME;
			// config.display_config();

			CME << "Launching All Servers . . ." << EME;
			unsigned long size = config.getServers().size();
			std::for_each(config.getServers().begin(), config.getServers().end(), initServer);

			while (size-- > 0)
			{
				CME << "Waiting for Servers . . ." << EME;
				waitpid(-1, &config.getStatus(), 0);
				CME << "One server came back . . ." << EME;
				CME << "Status of return|" << config.getStatus() << "|" << EME;
			}
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
