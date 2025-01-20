#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <set>
#include <algorithm>

class Client
{
	public:
		Client();
		~Client();
		bool is_used;
		int client_fd;
		std::string src_ip;
		uint16_t src_port;
		uint16_t my_index;
		std::string send_buffer;
		bool registered;
		bool authenticated;
		bool send_to_all;
		bool is_operator;
		bool quit;
		std::string nick;
		std::string user;
		std::string realname;
		std::set<Client *> sendto;
};
