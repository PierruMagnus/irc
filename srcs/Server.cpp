#include "../includes/Server.hpp"

Server::Server(unsigned int port, const std::string& password): _port(port), _password(password)
{
	const size_t client_slot_num = sizeof(this->clients) / sizeof(*this->clients);
	const size_t client_slot_map_num = sizeof(this->client_map) / sizeof(*this->client_map);
	
	for (size_t i = 0; i < client_slot_num; i++)
	{
		this->clients[i].is_used = false;
		this->clients[i].client_fd = -1;
	}

	for (uint16_t i = 0; i < client_slot_map_num; i++)
		this->client_map[i] = 0u;

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
	struct epoll_event event;

	memset(&event, 0, sizeof(struct epoll_event));

	event.events = events;
	event.data.fd = serv_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serv_fd, &event) < 0)
	{
		std::cerr << "Error: epoll_ctl() failed." << std::endl;
		close(serv_fd);
		exit(1);
	}
}

int Server::accept_new_client(int fd)
{
	int err;
	int client_fd;
	struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    uint16_t src_port;
    const char *src_ip;
    const size_t client_slot_num = sizeof(this->clients) / sizeof(*this->clients);

	memset(&addr, 0, sizeof(addr));
	client_fd = accept(fd, (struct sockaddr *)&addr, &addr_len);
	if (client_fd < 0)
	{
		err = errno;
		if (err == EAGAIN)
			return (0);
		std::cerr << "Error: accept() failed." << std::endl;
		return (-1);
	}

	src_port = ntohs(addr.sin_port);
	src_ip = inet_ntoa(addr.sin_addr);
	if (!src_ip)
	{
		std::cerr << "Error: inet_ntoa() failed." << std::endl;
		std::cerr << "Cannot parse source address." << std::endl;
		close(client_fd);
		return (0);
	}

	for (size_t i = 0; i < client_slot_num; i++)
	{
		if (!this->clients[i].is_used)
		{
			this->clients[i].client_fd = client_fd;
			this->clients[i].src_ip = src_ip;
			this->clients[i].src_port = src_port;
			this->clients[i].is_used = true;
			this->clients[i].my_index = i;

			this->client_map[client_fd] = this->clients[i].my_index + 1u;
		
			epoll_add_new(this->_epoll_fd, client_fd, EPOLLIN | EPOLLPRI);
			std::cout << "Client " << src_ip << ":" << src_port << " has been accepted!" << std::endl;
			return (0);
		}
	}
	std::cout << "Slot is full, can't accept any more client at the moment." << std::endl;
	close(client_fd);
	return (0);
}

void Server::handle_client_event(int client_fd, uint32_t revents)
{
	int err;
	ssize_t recv_ret;
	char buffer[1024];
	const uint32_t err_mask = EPOLLERR | EPOLLHUP;

	uint32_t index = this->client_map[client_fd] - 1u;
	
	if (revents & err_mask)
	{
		std::cerr << "Client " << this->clients[index].src_ip << ":" << this->clients[index].src_port << " has closed its connection." << std::endl;
		if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL) < 0)
			std::cerr << "epoll_ctl(EPOLL_CTL_DEL) failed." << std::endl;
		close(client_fd);
		this->clients[index].is_used = false;
		return ;
	}

	recv_ret = recv(client_fd, buffer, sizeof(buffer), 0);
	if (recv_ret == 0)
	{
		std::cerr << "Client " << this->clients[index].src_ip << ":" << this->clients[index].src_port << " has closed its connection." << std::endl;
		if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL) < 0)
			std::cerr << "epoll_ctl(EPOLL_CTL_DEL) failed." << std::endl;
		close(client_fd);
		this->clients[index].is_used = false;
		return ;
	}
	if (recv_ret < 0)
	{
		err = errno;
		if (err == EAGAIN)
			return ;
		std::cerr << "recv() failed." << std::endl;
		std::cerr << "Client " << this->clients[index].src_ip << ":" << this->clients[index].src_port << " has closed its connection." << std::endl;
		if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL) < 0)
			std::cerr << "epoll_ctl(EPOLL_CTL_DEL) failed." << std::endl;
		close(client_fd);
		this->clients[index].is_used = false;
		return ;
	}

	buffer[recv_ret] = '\0';
	if (buffer[recv_ret - 1] == '\n')
		buffer[recv_ret - 1] = '\0';
	std::cout << "Client " << this->clients[index].src_ip << ":" << this->clients[index].src_port << " sends: " << buffer << std::endl;
	return ;
}

void Server::run()
{
	// socklen_t client_len;
	memset(&this->_serverAddr, 0, sizeof(this->_serverAddr));
	this->_serverAddr.sin_family = AF_INET;
	this->_serverAddr.sin_addr.s_addr = INADDR_ANY;
	this->_serverAddr.sin_port = htons(this->_port);
	if (bind(this->_sock_fd, (struct sockaddr *)&this->_serverAddr, sizeof(this->_serverAddr)) < 0)
	{
		std::cerr << "Error: binding failed." << std::endl;
		close(this->_sock_fd);
		exit(1);
	}
	if (listen(this->_sock_fd, 10) < 0)
	{
		std::cerr << "Error: listen() failed." << std::endl;
		close(this->_sock_fd);
		exit(1);
	}

	std::cout << "Server running." << std::endl;
	
	epoll_add_new(this->_epoll_fd, this->_sock_fd, EPOLLIN | EPOLLPRI);

	std::cout << "Server listening on port " << this->_port << "." << std::endl;

	int epoll_ret;
	int err;
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
			err = errno;
			if (err == EINTR)
			{
				std::cerr << "Interrupted !" << std::endl;
				continue ;
			}
			std::cerr << "Error: epoll_wait() failed." << std::endl;
			break ;
		}
		for (int i = 0; i < epoll_ret; i++)
		{
			int fd = events[i].data.fd;

			if (fd == this->_sock_fd)
			{
				if (accept_new_client(fd) < 0)
					exit(1);
				continue ;
			}
			handle_client_event(fd, events[i].events);
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
