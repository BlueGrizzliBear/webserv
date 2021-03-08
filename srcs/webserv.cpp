// Server side C program to demonstrate Socket programming

#include "./webserv.hpp"
#include "./ConfigParser.hpp"

int	errorMsg(const char * main_err, const char * err, int ret)
{
	COUT << main_err << ": ";
	COUT << err << ENDL;
	return (ret);
}

void	abortServers(int ret, ConfigParser * parent)
{
	// COUT << "--- ABORTING ---" << ENDL;
	if (parent)
	{
		std::vector<ServerBloc>::iterator begin = parent->getServers().begin();
		std::vector<ServerBloc>::iterator end = parent->getServers().end();

		while (begin->pid != 0 && begin != end)
		{
			COUT << "killing pid |" << begin->pid << "|" << ENDL;
			kill(begin->pid, SIGKILL);
			begin->pid = 0;
			begin++;
		}
		if (begin->server_fd != -1)
			close(begin->server_fd);
	}
	exit(ret);
}

int	launchServer(ServerBloc & server)
{
	std::string	buf;
	int			new_socket;
	long		valread;

	/* Message to send back to Client */
    char hello[3000] = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";

	CME << "Launching Server #" << server.getNo() <<  ". . ." << EME;

    while (1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server.server_fd, (struct sockaddr *)&server.address, (socklen_t*)&server.addrlen)) < 0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        char buffer[30000] = {0};
        valread = read( new_socket , buffer, 30000);
        printf("%s\n",buffer );
        write(new_socket , hello , strlen(hello));
        printf("------------------Hello message sent-------------------\n");
        close(new_socket);
    }
	return (1);
}

int	initServer(ServerBloc & server)
{
	/* Creating socket file descriptor */
	/* AF_INET: Protocoles Internet IPv4	|	SOCK_STREAM: Virtual Circuit Service */
    if ((server.server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
		abortServers(errorMsg("Error in socket()", strerror(errno), EXIT_FAILURE), server.getParent());

	/* Defining adress */
    server.address.sin_family = AF_INET;	/* corresponding to IPv4 protocols */
    // server.address.sin_addr.s_addr = INADDR_ANY;	/* corresponding to 0.0.0.0 */
    // server.address.sin_addr.s_addr = htonl(INADDR_ANY);	/* corresponding to 0.0.0.0 */
    server.address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);	/* corresponding to 127.0.0.1 */
	server.address.sin_port = htons(server.getPort());	/* corresponding to the server port, must be > 1024 */
    // server.address.sin_port = server.getPort();	/* corresponding to the server port */
    
	/* Initialising other adress attributes to 0 */
    memset(server.address.sin_zero, '\0', sizeof(server.address.sin_zero));
    
	/* Assigning adress to the socket */
    // COUT << "server.server_fd|" << server.server_fd << "|\n";
    // COUT << "Port|" << htons(server.address.sin_port) << "|\n"; 
    if (bind(server.server_fd, reinterpret_cast<struct sockaddr *>(&server.address), sizeof(server.address)) < 0)
		abortServers(errorMsg("Error in bind()", strerror(errno), EXIT_FAILURE), server.getParent());
		// exit_with_error("Error in bind()", strerror(errno), EXIT_FAILURE);

	/* Enable socket to accept connections */
    if (listen(server.server_fd, BACK_LOG) < 0)
		abortServers(errorMsg("Error in listen()", strerror(errno), EXIT_FAILURE), server.getParent());
	if ((server.pid = fork()) == -1)
		abortServers(errorMsg("Error in fork()", strerror(errno), EXIT_FAILURE), server.getParent());
	else if (server.pid == 0)
	{
		// COUT << "THE CHILD LIVES HERE\n";
		launchServer(server);
	}
	else
	{
		// COUT << "THE PARENT CONTINUES TO LIVE\n";
	}
	return (0);
}

int main(int argc, char const ** argv)
{
	if (argc == 2)
	{
		try
		{
			ConfigParser	config(argv[1]);
			CME << "Parsing Complete !" << EME;
			config.display_config();

			CME << "Launching All Servers . . ." << EME;
			unsigned long size = config.getServers().size();
			std::for_each(config.getServers().begin(), config.getServers().end(), initServer);
			
			CME << "Now waiting for Servers . . ." << EME;
			while (size-- > 0)
				waitpid(-1, &config.getStatus(), 0);
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