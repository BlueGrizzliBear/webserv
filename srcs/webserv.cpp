#include "./webserv.hpp"
#include "./ConfigParser.hpp"

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

bool	parseClientRequest(ServerBloc & server)
{
	try
	{
		/* Read Client Request with recv */
		if (server.readClient(server.client.fd))
		{
			/* Displaying Client request */
			// COUT << "Received Data from client\n";
			std::cerr << "Displaying length|" << GREEN << server.req.getData().length() << RESET << "|" << std::endl;

			// std::cerr << "Displaying header|" << GREEN;
			// for (size_t i = 0; i != server.req.getData().find("\r\n\r\n") + 4; i++)
			// 	std::cerr << server.req.getData()[i];
			// std::cerr << RESET << "|" << std::endl;

			// std::cerr << "Displaying all data|" << GREEN << server.req.getData() << RESET << "|" << std::endl;

			/* Parse Client Request first */
			if (server.processRequest())
				return (true);
		}
	}
	catch(const std::exception & e)
	{
		/* Catching exception from parsing request or execute request */
		std::cerr << RED << e.what() << RESET << std::endl;
		server.parseException(e.what());
		return (true);	/* send true to execute the error msg to Response */
	}
	return (false);
}

bool	parseServerResponse(ServerBloc & server)
{
	try
	{
		if (server.sendResponse(server.client))
		{
			gettimeofday(&mytime2, NULL);
			COUT << RED << "Time elapsed: " << static_cast<float>((mytime2.tv_sec - mytime1.tv_sec) * 1000 + ((mytime2.tv_usec - mytime1.tv_usec) / 1000)) << " ms." << RESET << ENDL;
			close(server.client.fd);	/* Closing client socket, finished processing request */
			server.client.fd = -1;
			server.getParent()->_initSelect(server);
			server.serv_select.fd_max = server.serv_port.fd;	/* Re-assgning fd_max value for select */
			return (true);
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << RED << e.what() << RESET << '\n';
	}
	return (false);
}

int	launchServer(ServerBloc & server)
{
	server.client.fd = -1;

	static bool status = 1;

    while (1)
    {
		/* ------ Configuring which fds should be listened to ------ */
		/* Adding the Standard Input Fd to the read playlist */
		FD_SET(STDIN_FILENO, &server.serv_select.readfds);

		/* ------ Listening to sockets . . . ----------------------- */
		int recVal = 0;

		// CME << "Listening to sockets" << EME;
		recVal = select(server.serv_select.fd_max + 1, &server.serv_select.readfds, &server.serv_select.writefds, NULL, &server.serv_select.timeout);
		switch (recVal)
		{
			case 0:
			{
				// displayError("Error in Select()", "select timed out");
				if (server.client.fd == -1)
				{
					// COUT << "reseting server\n";
					server.getParent()->_initSelect(server);
				}
				else if (status == 1)
				{
					// COUT << "reseting read fds\n";
					FD_ZERO(&server.serv_select.readfds);
					FD_SET(server.client.fd, &server.serv_select.readfds);
				}
				else
				{
					// COUT << "reseting write fds\n";
					// FD_ZERO(&server.serv_select.readfds);
					FD_ZERO(&server.serv_select.writefds);
					FD_SET(server.client.fd, &server.serv_select.writefds);
				}
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
					close(server.client.fd);
					close(server.serv_port.fd);
					exit(EXIT_SUCCESS);
				}
	/* READ */	else if ((server.client.fd != -1) && FD_ISSET(server.client.fd, &server.serv_select.readfds))
				{
					CMEY << "Respective socket is ready for reading request" << EME;

					/* Parsing Client Request */
					if (parseClientRequest(server))
					{
						/* Removing client socket from read playlist if finished */
						FD_CLR(server.client.fd, &server.serv_select.readfds);		/* Clearing read list from client socket */
						FD_SET(server.client.fd, &server.serv_select.writefds);	/* Adding the client socket to the write playlist */
					}
				}
	/* WRITE */	else if ((server.client.fd != -1) && FD_ISSET(server.client.fd, &server.serv_select.writefds))
				{
					CMEY << "Respective socket is ready for writing request response" << EME;

					/* Parsing Server Response */
					// if (parseServerResponse(server, server.client))
					if ((status = parseServerResponse(server)))
					{
						// COUT << "Finished writing to Client\n";
					}
				}
	/* NEW */	else if ((server.client.fd == -1) && FD_ISSET(server.serv_port.fd, &server.serv_select.readfds))
				{
					gettimeofday(&mytime1, NULL);
					// time(&mytime);

					CMEY << "Someone is talking to the server socket" << EME;

					/* Opening socket for new client */
					server.client.fd = accept(server.serv_port.fd, reinterpret_cast<struct sockaddr *>(&server.client.address), reinterpret_cast<socklen_t *>(&server.client.addrlen));
					if (server.client.fd == -1)
					{
						displayError("Error in accept()", strerror(errno));
						break ;
					}
					fcntl(server.client.fd, F_SETFL, O_NONBLOCK);	/* Set the socket to non blocking */


					FD_CLR(server.serv_port.fd, &server.serv_select.readfds);	/* removing server fd from reading list to process request first */
					FD_SET(server.client.fd, &server.serv_select.readfds);	/* Adding the respective socket to the read playlist */

					server.serv_select.fd_max = server.client.fd;	/* Re-assgning fd_max value for select */
				}
				else
				{
					COUT << "FD weird\n";
				}
				// COUT << "done\n";
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
