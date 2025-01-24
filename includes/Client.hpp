#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <vector>
#include <algorithm>

class Client
{
	public:
		Client();
		Client& operator=(const Client& src);
		~Client();
		bool is_used;
		int client_fd;
		std::string src_ip;
		uint16_t src_port;
		uint16_t my_index;
		std::string send_buffer;
		bool registered;
		bool authenticated;
		bool is_operator;
		bool quit;
		std::string nick;
		std::string user;
		std::string realname;
		std::vector<std::pair<Client *, std::string> > sendto;
};
