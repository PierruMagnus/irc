/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmagnero <pmagnero@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/10 09:54:30 by pmagnero          #+#    #+#             */
/*   Updated: 2025/01/10 11:08:30 by pmagnero         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

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
	checkPassword(av[2]);

	return (port);
}

int main(int ac, char **av)
{
	unsigned int port;

	port = checkArgs(ac, av);

	Server server(port, av[2]);
	
	server.run();
	
	return (0);
}
