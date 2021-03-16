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

ssize_t	readClient(int socket, char * buffer)
{
	ssize_t n = 0;

	// COUT << "Waiting to receive message\n";
	if ((n = recv(socket, buffer, (1024 - 1), 0)) < 0)
	{
		displayError("Error in recv()", strerror(errno));
		n = 0;
	}
	buffer[n] = 0;
	return (n);
}

int	parseClientRequest(ServerBloc & server, int client_socket)
{
	/* Reading request from new client */
	if (readClient(client_socket, server.serv_select.buf) == -1)
	{
		displayError("Error in readClient()", "disconnecting client");
		return (-1);
	}

	/* Initialize the Request object of ServerBloc */
	try
	{
		server.parseRequest(server.serv_select.buf);
	}
	catch(const std::exception& e)
	{
		/* Catching exception from request parsing */
		std::cerr << RED << e.what() << RESET << std::endl;
	}
	

	/* Check if last characters in request are the end of http request */
	// if (!the_end)
	// 	return (1);
	server.serv_select.finished = 1;

	return (0);
}

int	parseServerResponse(ServerBloc & server, Socket & client)
{
	/* Answering to the Client tmp */
	char hello[3000] = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
	write(client.fd, hello, strlen(hello));
	COUT << "------------------Hello message sent-------------------" << ENDL;

	/* Removing client socket from write playlist */
	FD_CLR(client.fd, &server.serv_select.writefds);

	/* Closing client socket, finished processing request */
	close(client.fd);
	client.fd = -1;
	server.serv_select.finished = 1;

	/* Re-assgning max value for select */
	server.serv_select.max = server.serv_port.fd > client.fd ? server.serv_port.fd : client.fd;

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
		recVal = select(server.serv_select.max + 1, &server.serv_select.readfds, &server.serv_select.writefds, NULL, &server.serv_select.timeout);
		switch (recVal)
		{
			case 0:
			{
				// displayError("Error in Select()", "select timed out");
				if (new_client.fd == -1)
					server.getParent()->_initSelect(server);
				else
					FD_SET(new_client.fd, &server.serv_select.readfds);
				break ;
			}
			case -1:
			{
				displayError("Error in Select()", strerror(errno));
				break ;
			}
			default:
			{
				// COUT << "default, new_client.fd|" << new_client.fd << "\n";
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
					// COUT << "Someone is talking through their respective socket" << ENDL;

					/* Parsing Client Request */
					// COUT << "Parsing Request . . ." << ENDL;
					if (parseClientRequest(server, new_client.fd))
						exitServerOnError("Error in parseClientRequest()", "Unknown error occured", server, new_client.fd);

					/* Removing client socket from read playlist if finished */
					if (server.serv_select.finished)
					{
						/* Clearing read list from client socket */
						FD_CLR(new_client.fd, &server.serv_select.readfds);
						/* Adding the client socket to the write playlist */
						FD_SET(new_client.fd, &server.serv_select.writefds);
					}
					// COUT << "Finished reading from client\n";
				}
				/* Respective socket is ready for writing request response */
				else if ((new_client.fd != -1) && FD_ISSET(new_client.fd, &server.serv_select.writefds))
				{
					// COUT << "Respective socket is ready for writing request response" << ENDL;

					/* Parsing Server Response */
					if (parseServerResponse(server, new_client))
						exitServerOnError("Error in parseServerResponse()", "Unknown error occured", server, new_client.fd);
					// COUT << "Finished writing to client\n";

					/* Reseting server fd in reading list */
					server.getParent()->_initSelect(server);
				}
				/* Someone is talking to the server socket */
				else if ((new_client.fd == -1) && FD_ISSET(server.serv_port.fd, &server.serv_select.readfds))
				{
					// COUT << "Someone is talking to the server socket" << ENDL;

					/* Opening socket for new client */
					new_client.fd = accept(server.serv_port.fd, reinterpret_cast<struct sockaddr *>(&new_client.address), reinterpret_cast<socklen_t *>(&new_client.addrlen));
					/* EAGAIN = no connections */
					if (new_client.fd == -1)
					{
						displayError("Error in accept()", strerror(errno));
						break ;
					}
					fcntl(new_client.fd, F_SETFL, O_NONBLOCK);

					/* removing server fd from reading list to process request first */
					FD_CLR(server.serv_port.fd, &server.serv_select.readfds);

					/* Adding the respective socket to the read playlist */
					FD_SET(new_client.fd, &server.serv_select.readfds);

					/* Re-assgning max value for select */
					server.serv_select.max = server.serv_select.max > new_client.fd ? server.serv_select.max : new_client.fd;

					// COUT << "Finished creating new client\n";
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