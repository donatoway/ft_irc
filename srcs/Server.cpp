#include "Server.hpp"
#include "Client.hpp"
Server::Server(std::string const &port, std::string const &password)
{
	this->host = "127.0.0.1";
	this->port = port;
	this->password = password;
	this->running = 1;
	this->sock = this->create_socket();
	console_log("Main Socket Created");
}

Server::~Server()
{
	close(this->sock);
	console_log("Main Socket Closed");
}

void	Server::start()
{
	pollfd	server_fd = {this->sock, POLLIN | POLLHUP, 0};
	poll_fds.push_back(server_fd);

	console_log("Server waiting for connections");

	while (this->running)
	{
		// waiting for events
		if (poll(poll_fds.begin().base(), poll_fds.size(), -1) < 0)
			throw std::runtime_error("Error while polling");
		// event handling
		for (std::vector<pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end(); ++it)
		{
			// no events
			if (it->revents == 0)
				continue ;
			// on connect
			if ((it->revents & POLLIN) == POLLIN)
			{
				if (it->fd == this->sock)
				{
					this->handle_connection();
					break ;
				}
				if (this->handle_message(it->fd))
					break ;
			}
			// on disconnect
			if ((it->revents & POLLHUP) == POLLHUP)
			{
				this->handle_disconnection(it->fd);
				break ;
			}
		}
	}
}

int	Server::create_socket()
{
	int	sockfd;
	int	int_port;

	// open the socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		throw std::runtime_error("Error while opening socket");
	// set to non-blocking mode
	if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("Error while setting socket to NON-BLOCKING");
	// setup the binding informations
	struct sockaddr_in addr = {};
	bzero((char *)&addr, sizeof(addr));
	std::istringstream(this->port) >> int_port;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(int_port);
	// bind the socket
	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("Error while binding socket");
	// listen on the socket
	if (listen(sockfd, MAX_CONNECTIONS) < 0)
		throw std::runtime_error("Error while listening in socket");
	return (sockfd);
}

void	Server::handle_connection()
{
	int			fd;
	sockaddr_in	addr = {};
	socklen_t 	size;
	char		_msg[1000];

	// accept connection
	size = sizeof(addr);
	fd = accept(this->sock, (sockaddr *)&addr, &size);
	if (fd < 0)
		throw std::runtime_error("Error while accepting new client");
	pollfd	poll_fd = {fd, POLLIN, 0};
	this->poll_fds.push_back(poll_fd);
	// get client info
	if (getsockname(fd, (struct sockaddr *)&addr, &size) != 0)
		throw std::runtime_error("Error while gathering client informations");
	// create a new client
	Client *new_client = new Client(fd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	this->clients.insert(std::make_pair(fd, new_client));
	// log new connection
	sprintf(_msg, "%s:%d has connected", new_client->getHostname().c_str(), new_client->getPort());
	console_log(_msg);
}

std::string	Server::recive(int fd)
{
	std::string	_msg;
	char		buffer[100];

	bzero(buffer, 100);
	// recive until new line
	while (!std::strstr(buffer, "\n"))
	{
		bzero(buffer, 100);
		if (recv(fd, buffer, 100, 0) < 0)
		{
			if (errno != EWOULDBLOCK)
				throw std::runtime_error("Error while reciving from client");
		}
		// clear buffer means exit (UNIX)
		if (!buffer[0])
			return ("");
		_msg.append(buffer);
	}
	return (_msg);
}

int	Server::handle_message(int fd)
{
	std::string _msg = this->recive(fd);
	// if disconnected
	if (_msg[0] == 0)
	{
		this->handle_disconnection(fd);
		return (1);
	}
	// TODO command handler
	//console_log("TODO command handler");
	execute_cmd(parse_cmd(_msg));
	// TODO command handler
	console_log(_msg);
	return (0);
}

void	Server::handle_disconnection(int fd)
{
	try
	{
		Client	*client = this->clients.at(fd);
		char 	_msg[1000];

		// message of disconnection
		sprintf(_msg, "%s:%d has disconnected.", client->getHostname().c_str(), client->getPort());
		console_log(_msg);
		// remove the client
		this->clients.erase(fd);
		for (std::vector<pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end(); ++it)
		{
			if (it->fd != fd)
				continue ;
			this->poll_fds.erase(it);
			close(fd);
			break ;
		}
		delete client;
	}
	catch (std::out_of_range const &err) {}
}

int		Server::parse_cmd(std::string cmd)
{


	//creare una funzione che splitta comando e argomento
	// e li salva in un vettore
	if (cmd.compare("nick\n") == 0) return (nick);
	if (cmd.compare("connect\n") == 0) return (connect);
	if (cmd.compare("disconnect\n") == 0) return (disconnect);
	if (cmd.compare("join\n") == 0) return (join);
	if (cmd.compare("part\n") == 0) return (part);
	if (cmd.compare("set nick\n") == 0) return (set_nick);
	if (cmd.compare("whois\n") == 0) return (whois);
	if (cmd.compare("msg\n") == 0) return (msg);
	if (cmd.compare("kick\n") == 0) return (kick);
	if (cmd.compare("ban\n") == 0) return (ban);
	return (0);
}

void	Server::execute_cmd(int	cmd)
{
	switch (cmd)
	{
	case nick : console_log("CMD: comando scelto nick\n"); break;
	case connect : console_log("CMD: comando scelto connect\n");break;
	case disconnect : console_log("CMD: comando scelto disconnect\n");break;
	case join : console_log("CMD: comando scelto join\n");break;
	case part : console_log("CMD: comando scelto part\n");break;
	case set_nick : console_log("CMD: comando scelto set nick\n");break;
	case whois : console_log("CMD: comando scelto  whois\n");break;
	case msg : console_log("CMD: comando scelto  msg\n");break;
	case kick : console_log("CMD: comando scelto set kick\n");break;
	case ban : console_log("CMD: comando scelto  ban\n");break;

	default: console_log("CMD: Command not found\n");
	break;
	}
}



