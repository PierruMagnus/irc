#include "../includes/Server.hpp"



Server::Server(unsigned int port, const std::string& password): _port(port), _password(password)
{
	this->_epoll_fd = epoll_create(255);
	if (this->_epoll_fd < 0)
	{
		std::cerr << "Error: epoll creation failed." << std::endl;
		exit(1);
	}
	this->_sock_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (this->_sock_fd < 0)
	{
		std::cerr << "Error: socket opening failed." << std::endl;
		exit(1);
	}
	// if (setsockopt(this->_sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)1, sizeof(int)) < 0)
	// {
	// 	std::cerr << "Error: setsockopt() failed." << std::endl;
	// 	close(this->_sock_fd);
	// 	exit(1);
	// }
}

Server::~Server()
{
}

void epoll_add_new(int epoll_fd, int serv_fd, uint32_t events)
{
	struct epoll_event event = {0, {0}};

	event.events = events;
	event.data.fd = serv_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serv_fd, &event) < 0)
	{
		std::cerr << "Error: epoll_ctl() failed." << std::endl;
		close(serv_fd);
		exit(1);
	}
}

void Server::run()
{
	// socklen_t client_len;
	this->_serverAddr.sin_family = AF_INET;
	this->_serverAddr.sin_addr.s_addr = INADDR_ANY;
	this->_serverAddr.sin_port = htons(this->_port);
	if (bind(this->_sock_fd, (struct sockaddr *)&this->_serverAddr, sizeof(this->_serverAddr)) < 0)
	{
		std::cerr << "Error: binding failed." << std::endl;
		close(this->_sock_fd);
		exit(1);
	}
	listen(this->_sock_fd, 10);

	std::cout << "Server running." << std::endl;
	
	epoll_add_new(this->_epoll_fd, this->_sock_fd, EPOLLIN | EPOLLPRI);

	std::cout << "Server listening on port " << this->_port << "." << std::endl;

	int epoll_ret;
	struct epoll_event events[32];

	while (1)
	{
		epoll_ret = epoll_wait(this->_epoll_fd, events, 32, 3000);
		if (epoll_ret == 0)
		{
			std::cerr << "no events within 3000 milliseconds." << std::endl;
			continue ;
		}
		if (epoll_ret == -1)
		{
			std::cerr << "Error: epoll_wait() failed." << std::endl;
			break ;
		}
		for (int i = 0; i < epoll_ret; i++)
		{
			int fd = events[i].data.fd;

			if (fd == this->_sock_fd)
			{
				if ()
			}
		}
	}

	// client_len = sizeof(this->_clientAddr);
	// this->_client_sock_fd = accept(this->_sock_fd, (struct sockaddr *)&this->_clientAddr, &client_len);
	// if (this->_client_sock_fd < 0)
	// {
	// 	std::cerr << "Error: accept failed." << std::endl;
	// 	close(this->_sock_fd);
	// 	exit(1);
	// }
	// char buffer[1024] = {0};

	// if (recv(this->_client_sock_fd, buffer, sizeof(buffer), 0) < 0)
	// {
	// 	std::cerr << "Error: reading from socket failed." << std::endl;
	// 	close(this->_sock_fd);
	// 	exit(1);
	// }

	// std::cout << "Message from client: " << buffer << std::endl;
	// close(this->_sock_fd);
}

void Server::stop()
{
	std::cout << "Server stopping" << std::endl;
}
