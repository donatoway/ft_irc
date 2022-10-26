#ifndef SERVER_HPP
#define SERVER_HPP

#pragma once

#include <unistd.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <string>
#include <sstream>
#include <poll.h>
#include <vector>
#include <map>
#include <cstdio>
#include <cerrno>

#include "utils.hpp"
#include "Client.hpp"

#define MAX_CONNECTIONS 200

#define	connect			500
#define	disconnect		501
#define join			502
#define part			503
#define set_nick		504
#define nick			505
#define whois			506
#define msg				507
#define kick			508
#define ban				509


class Server
{
	private:
		std::vector<std::string> cmds;
		std::string				host;
		std::string				port;
		std::string				password;
		std::string				buffer;
		int						sock;
		int						running;
		std::vector<pollfd>		poll_fds;
		std::map<int, Client *>	clients;

	public:
		Server(std::string const &port, std::string const &password);
		~Server();

		void		start();
		int			create_socket();
		void		handle_connection();
		std::string	recive(int fd);
		int			handle_message(int fd);
		void		handle_disconnection(int fd);
		void		execute_cmd(int	cmd);
		int			parse_cmd(std::string cmd);
};

#endif
