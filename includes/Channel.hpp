#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <set>
#include <vector>
#include <algorithm>

#include "Client.hpp"

class Channel
{
	public:
		Channel();
		Channel& operator=(const Channel& src);
		Channel(const std::string& name);
		~Channel();
		std::vector<Client *> users;
		std::vector<Client *> invite;
		std::vector<Client *> operators;
		std::string topic;
		std::string name;
		uint16_t mode;
		bool topic_mode;
		std::string key;
		uint16_t limit;

		bool is_user(Client *c);
		bool user_exist(const std::string &client);
		bool is_operator(Client *client);
		bool is_invited(Client *client);

		void rm_invite(Client *client);

		Client *kick_client(const std::string &user);
};
