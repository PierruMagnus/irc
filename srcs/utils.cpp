#include "../includes/Server.hpp"

std::string Server::getTime() const
{
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80];
	std::string res;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 80, "[%F %T]", timeinfo);
	res = buffer;
	return (res);
}

void Server::debug(std::string msg, Client *from, bool to)
{
	msg.erase(msg.find_last_not_of('\n') + 1);
	if (from && !to)
		std::cout << "INFO: " << getTime() << ": " << from->src_ip << ":" << from->src_port << ": Send: " << msg << std::endl;
	else if (to && from)
		std::cout << "INFO: " << getTime() << ": " << "Server send" << msg << " to " << from->src_ip << ":" << from->src_port << std::endl;
	if (!from)
		std::cout << "INFO: " << getTime() << ": " << msg << std::endl;
}
