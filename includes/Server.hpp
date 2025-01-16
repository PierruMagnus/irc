/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmagnero <pmagnero@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/10 10:01:22 by pmagnero          #+#    #+#             */
/*   Updated: 2025/01/16 13:30:57 by pmagnero         ###   ########.fr       */
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
		// const std::string _networkname = "KEK IRC";
		int _epoll_fd;
		int _sock_fd;
		// int _client_sock_fd;
		sockaddr_in _serverAddr;
		// sockaddr_in _clientAddr;
		uint32_t client_map[10000];
		Client clients[MAX_NB_CLIENT];

		int accept_new_client(int fd);
		void handle_client_event(int client_fd, uint32_t revents);

		void parse_cmd(char *buffer, int client_fd, uint32_t index);

		std::string pass_cmd(std::vector<std::string> params, Client *client);
		std::string nick_cmd(std::vector<std::string> params, Client *client);
		std::string user_cmd(std::vector<std::string> params, Client *client);
		std::string ping_cmd(std::vector<std::string> params, Client *client);
		std::string join_cmd(std::vector<std::string> params, Client *client);

		bool is_valid_nick(const std::string& nick);
		bool nick_exist(const std::string& nick);
};
