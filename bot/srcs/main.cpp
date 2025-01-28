/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmagnero <pmagnero@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/10 09:54:30 by pmagnero          #+#    #+#             */
/*   Updated: 2025/01/28 13:40:19 by pmagnero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

Server* instance = NULL;

int checkPort(const std::string& port_s)
{
	unsigned int port;

	if (port_s.empty())
	{
		std::cerr << "Error: port is empty." << std::endl;
		exit(1);
	}

	for (std::string::const_iterator it = port_s.begin();it < port_s.end(); it++)
	{
		if (!std::isdigit(*it))
		{
			std::cerr << "Error: port number incorrect." << std::endl;
			std::cerr << "Port is not a number." << std::endl;
			exit(1);
		}
	}
	port = std::atoi(port_s.c_str());
	if ((port_s.size() > 5 || port_s.size() < 4)
		&& (port < 2000 || port > 65535))
	{
		std::cerr << "Error: port number incorrect." << std::endl;
		std::cerr << "It must be between 2000 and 65535." << std::endl;
		exit(1);
	}
	return (port);
}

void checkPassword(const std::string& pass)
{
	if (pass.empty())
	{
		std::cerr << "Error: pass is empty." << std::endl;
		exit(1);
	}
	if (pass.size() > 20)
	{
		std::cerr << "Error: pass is too long." << std::endl;
		exit(1);
	}
	for (std::string::const_iterator it = pass.begin();it < pass.end(); it++)
	{
		if (!std::isprint(*it))
		{
			std::cerr << "Error: pass is incorrect." << std::endl;
			std::cerr << "Pass contains non-printable characters." << std::endl;
			exit(1);
		}
	}
}

unsigned int checkArgs(int ac, char **av)
{
	unsigned int port;

	if (ac != 3)
	{
		std::cerr << "Error: Bad number of arguments." << std::endl;
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		exit(1);
	}

	port = checkPort(av[1]);
	// checkPassword(av[2]);

	return (port);
}

void sig_handler(int i)
{
	(void)i;
	if (instance)
	{
		instance->stop();
		instance = NULL;
	}
	exit(0);
}
class Kek {
	public:
		std::vector<std::string> lol;
		void sendTo(const std::string &msg)
		{
			this->lol.push_back(msg);
		}
};

int main(int ac, char **av)
{
	unsigned int port;

	port = checkArgs(ac, av);

	struct sigaction sigInthandler;

	sigInthandler.sa_handler = sig_handler;
   	sigemptyset(&sigInthandler.sa_mask);
   	sigInthandler.sa_flags = 0;

	sigaction(SIGINT, &sigInthandler, NULL);

	Server server(port, av[2]);
	
	instance = &server;

	server.run();
	// (void)ac;
	// (void)av;
	// Kek kek;
	// // std::vector<std::string> kek;
	// // std::string lol = "hey";
	// const std::string& lul = "NICK bot\r\n";
	// kek.sendTo(lul);
	// // for (std::vector<std::string>::iterator it = kek.lol.begin();it != kek.lol.end();it++)
	// // 	(*it).clear();
	// // kek.lol.clear();
	// // kek.lol.clear();

	return (0);
}
