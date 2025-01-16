#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <vector>
#include <algorithm>

class Command
{
	public:
		Command();
		~Command();

		void pass_cmd();
		void nick_cmd();
		void user_cmd();

		const std::string cmd;
		std::vector<std::string> params;
};
