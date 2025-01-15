/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmagnero <pmagnero@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/10 10:01:22 by pmagnero          #+#    #+#             */
/*   Updated: 2025/01/15 09:40:13 by pmagnero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

#include <vector>
#include <algorithm>

#include "Client.hpp"

class Server
{
	public:
		Server(unsigned int port, const std::string& password);
		~Server();

		void run();
		void stop();

		int accept_new_client(int fd);
		void handle_client_event(int client_fd, uint32_t revents);
	
	private:
		unsigned int _port;
		const std::string _password;
		int _epoll_fd;
		int _sock_fd;
		// int _client_sock_fd;
		sockaddr_in _serverAddr;
		// sockaddr_in _clientAddr;
		uint32_t client_map[10000];
		Client clients[10];
};
