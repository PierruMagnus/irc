/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmagnero <pmagnero@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/10 10:01:22 by pmagnero          #+#    #+#             */
/*   Updated: 2025/01/28 19:05:29 by pmagnero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <cstring>
#include <string>
#include <signal.h>
#include <ctime>
#include <iomanip>
#include <cstddef>
#include <fcntl.h>
#include <sstream>
#include <fstream>
#include <iterator>


#include <vector>
#include <set>
#include <map>
#include <algorithm>

// #include "Client.hpp"
// #include "Channel.hpp"
#include "Numerics.hpp"


#define MAX_NB_CLIENT 10

class Server
{
	public:
		Server(unsigned int port, const std::string& password);
		~Server();

		void run();
		void stop();
	
	private:
		unsigned int _port;
		const std::string _password;
		int _epoll_fd;
		int _sock_fd;
		sockaddr_in _serverAddr;
		// uint32_t client_map[10000];
		// Client clients[MAX_NB_CLIENT];
		bool connection;

		struct client {
			std::string name;
			std::string wordtoguess;
			std::string lettersfound;
			int			attemptsnb;
		};

		std::vector<std::string>	wordlist;
		std::vector<client>			clients;
		
		int accept_new_client(int fd);
		void handle_client_event(int client_fd, uint32_t revents);

		void epoll_add_new(int epoll_fd, int serv_fd, uint32_t events);
		void epoll_mod(int epoll_fd, int client_fd, uint32_t events);

		void parse_cmd(std::string buffer);

		void sendTo(const std::string &msg);
		bool privmsg_cmd(std::vector<std::string> params);


		bool is_valid_nick(const std::string& nick);

		std::string getTime() const;
		void debug(const std::string &msg);
		void setColor( int color );
		void resetColor( void );
		std::vector<std::string> sendto;
};
