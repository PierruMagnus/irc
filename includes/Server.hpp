/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmagnero <pmagnero@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/10 10:01:22 by pmagnero          #+#    #+#             */
/*   Updated: 2025/01/21 13:51:48 by pmagnero         ###   ########.fr       */
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
#include <signal.h>
#include <ctime>
#include <iomanip>
#include <cstddef>

#include <vector>
#include <set>
#include <map>
#include <algorithm>

// #include "Client.hpp"
#include "Channel.hpp"

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
		std::map<std::string, std::string> operators;
		// const std::string _networkname = "KEK IRC";
		int _epoll_fd;
		int _sock_fd;
		sockaddr_in _serverAddr;
		uint32_t client_map[10000];
		Client clients[MAX_NB_CLIENT];
		std::set<Channel *> channels;

		int accept_new_client(int fd);
		void handle_client_event(int client_fd, uint32_t revents);

		void epoll_add_new(int epoll_fd, int serv_fd, uint32_t events);
		void epoll_mod(int epoll_fd, int client_fd, uint32_t events, int index);

		void parse_cmd(char *buffer, int client_fd, uint32_t index);

		bool pass_cmd(std::vector<std::string> params, Client *client);
		bool nick_cmd(std::vector<std::string> params, Client *client);
		bool user_cmd(std::vector<std::string> params, Client *client);
		bool ping_cmd(std::vector<std::string> params, Client *client);
		bool oper_cmd(std::vector<std::string> params, Client *client);
		// bool quit_cmd(std::vector<std::string> params, Client *client);
		bool join_cmd(std::vector<std::string> params, Client *client);
		bool privmsg_cmd(std::vector<std::string> params, Client *client);

		bool is_valid_nick(const std::string& nick);
		Client *nick_exist(const std::string& nick);
		Channel *channel_exist(const std::string& channel);

		std::string getTime() const;
		void debug(std::string msg, Client *from, bool to);
};
