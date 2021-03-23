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

int	parseClientRequest(ServerBloc & server, int client_socket)
{
	try
	{
		/* Read Client Request with recv */
		server.readClient(client_socket);

		if (server.serv_select.incomplete)
			return (0);

		/* Parse Client Request first */
		server.processRequest();
	}
	catch(const std::exception& e)
	{
		/* Catching exception from parsing request or execute request */
		std::cerr << RED << e.what() << RESET << std::endl;
		/* The request is done for */
		server.req.finished = 1;
	}
	return (0);
}

int	parseServerResponse(ServerBloc & server, Socket & client)
{
	try
	{
		if (server.sendResponse(client))
		{
			// FD_CLR(client.fd, &server.serv_select.writefds);	/* Removing client socket from write playlist */
			close(client.fd);	/* Closing client socket, finished processing request */
			client.fd = -1;
			server.getParent()->_initSelect(server);
			server.serv_select.fd_max = server.serv_port.fd > client.fd ? server.serv_port.fd : client.fd;	/* Re-assgning fd_max value for select */
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << RED << e.what() << RESET << '\n';
		return (1);
	}
	
	return (0);
}

int	launchServer(ServerBloc & server)
{
	Socket new_client;
	new_client.fd = -1;


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
				if (new_client.fd == -1)
				{
					// COUT << "reseting server\n";
					server.getParent()->_initSelect(server);
				}
				else
				{
					// COUT << "reseting read fds\n";
					FD_ZERO(&server.serv_select.readfds);
					FD_SET(new_client.fd, &server.serv_select.readfds);
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
				if (FD_ISSET(STDIN_FILENO, &server.serv_select.readfds))
				{
					COUT << "Keyboard was pressed, exiting server properly\n";
					FD_ZERO(&server.serv_select.readfds);
					FD_ZERO(&server.serv_select.writefds);
					FD_ZERO(&server.serv_select.exceptfds);
					close(new_client.fd);
					close(server.serv_port.fd);
					exit(EXIT_SUCCESS);
				}
				/* Someone is talking through their respective socket */
				else if ((new_client.fd != -1) && FD_ISSET(new_client.fd, &server.serv_select.readfds))
				{
					CMEY << "Respective socket is ready for reading request" << EME;

					/* Parsing Client Request */
					if (parseClientRequest(server, new_client.fd))
						exitServerOnError("Error in parseClientRequest()", "Unknown error occured", server, new_client.fd);

					/* Removing client socket from read playlist if finished */
					if (server.req.finished)
					{
						/* Clearing read list from client socket */
						FD_CLR(new_client.fd, &server.serv_select.readfds);
						/* Adding the client socket to the write playlist */
						FD_SET(new_client.fd, &server.serv_select.writefds);
					}
				}
				/* Respective socket is ready for writing request response */
				else if ((new_client.fd != -1) && FD_ISSET(new_client.fd, &server.serv_select.writefds))
				{
					CMEY << "Respective socket is ready for writing request response" << EME;

					/* Parsing Server Response */
					if (parseServerResponse(server, new_client))
						exitServerOnError("Error in parseServerResponse()", "Unknown error occured", server, new_client.fd);
				}
				/* Someone is talking to the server socket */
				else if ((new_client.fd == -1) && FD_ISSET(server.serv_port.fd, &server.serv_select.readfds))
				{
					CMEY << "Someone is talking to the server socket" << EME;

					/* Opening socket for new client */
					new_client.fd = accept(server.serv_port.fd, reinterpret_cast<struct sockaddr *>(&new_client.address), reinterpret_cast<socklen_t *>(&new_client.addrlen));

					/* EAGAIN = no connections */
					if (new_client.fd == -1)
					{
						displayError("Error in accept()", strerror(errno));
						break ;
					}
					/* Set the socket to non blocking */
					fcntl(new_client.fd, F_SETFL, O_NONBLOCK);

					/* removing server fd from reading list to process request first */
					FD_CLR(server.serv_port.fd, &server.serv_select.readfds);

					/* Adding the respective socket to the read playlist */
					FD_SET(new_client.fd, &server.serv_select.readfds);

					/* Re-assgning fd_max value for select */
					server.serv_select.fd_max = server.serv_select.fd_max > new_client.fd ? server.serv_select.fd_max : new_client.fd;

					// COUT << "Finished creating new client\n";
				}
				else
				{
					// COUT << "FD weird\n";
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
			config.display_config();

			CME << "Launching All Servers . . ." << EME;
			unsigned long size = config.getServers().size();
			std::for_each(config.getServers().begin(), config.getServers().end(), initServer);

			while (size-- > 0)
			{
				CME << "Waiting for Servers . . ." << EME;
				waitpid(-1, &config.getStatus(), 0);
				CME << "One server came back . . ." << EME;
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
