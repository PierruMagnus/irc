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

/**
 *
Code Color Code Color
30	Black	90	Bright Black
31	Red	91	Bright Red
32	Green	92	Bright Green
33	Yellow	93	Bright Yellow
34	Blue	94	Bright Blue
35	Magenta 95	Bright Magenta
36	Cyan	96	Bright Cyan
37	White	97	Bright White
*/
void Server::setColor( int color )
{
    std::cout << "\033[" << color << "m";
}

void Server::resetColor( void )
{
    std::cout << "\033[0m";
}


void Server::debug(std::string msg, Client *from, bool to)
{
	msg.erase(msg.find_last_not_of('\n') + 1);
	if (from && !to)
	{
		std::cout << "INFO: " << getTime() << ": ";
		setColor(32);
		std::cout << from->src_ip << ":" << from->src_port << ": RECV:\n";
		resetColor();
		std::cout << msg << std::endl;
	}
	else if (to && from)
	{
		std::cout << "INFO: " << getTime() << ": ";
		setColor(36);
		std::cout << from->src_ip << ":" << from->src_port << ": Send:\n";
		resetColor();
		std::cout << msg << std::endl;
	}
	else if (!from)
		std::cout << "INFO: " << getTime() << ": " << msg << std::endl;
}
