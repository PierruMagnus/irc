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
		Server::stop();
	}
	this->_sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->_sock_fd < 0)
	{
		std::cerr << "Error: socket opening failed." << std::endl;
		Server::stop();
	}
	int flag = 1;
	if (setsockopt(this->_sock_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0)
	{
		std::cerr << "Error: setsockopt() failed." << std::endl;
		Server::stop();
	}

	if (fcntl(this->_sock_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Error: fcntl() failed." << std::endl;
		Server::stop();
	}

	this->operators.insert(std::pair<std::string, std::string>("pmagnero", "adminpass"));
}

Server::~Server()
{
	if (this->_sock_fd >= 0)
		close(this->_sock_fd);
	for (int i = 0; i < MAX_NB_CLIENT; i++)
	{
		this->clients[i].~Client();
	}
	this->operators.clear();
	for (std::set<Channel *>::iterator it = this->channels.begin();it != this->channels.end();it++)
		delete *it;
	this->channels.clear();
	exit(0);
}

void Server::epoll_add_new(int epoll_fd, int serv_fd, uint32_t events)
{
	struct epoll_event event;

	memset(&event, 0, sizeof(event));

	event.events = events;
	event.data.fd = serv_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serv_fd, &event) < 0)
	{
		std::cerr << "Error: epoll_ctl(EPOLL_CTL_ADD) failed." << std::endl;
		close(serv_fd);
		this->_sock_fd = -1;
		Server::stop();
	}
}

void Server::epoll_mod(int epoll_fd, int client_fd, uint32_t events, int index)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = events;
    event.data.fd = client_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &event) < 0) {
        std::cerr << "Error: epoll_ctl(EPOLL_CTL_MOD) failed." << std::endl;
        close(client_fd);
		this->clients[index].client_fd = -1;
		// Server::stop();
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

			epoll_add_new(this->_epoll_fd, client_fd, EPOLLIN);
			debug("has been accepted", &this->clients[i], 0);
			return (0);
		}
	}
	debug("Slot is full, can't accept any more client at the moment.", NULL, 0);
	close(client_fd);
	return (0);
}

void Server::parse_cmd(std::string buffer, Client *client)
{
	std::vector<std::string> cmds;
	std::vector<std::vector<std::string> > tokens;
	std::string delim = "\r\n";

	size_t pos = 0;
    std::string token;
    while ((pos = buffer.find(delim)) != std::string::npos) {
        token = buffer.substr(0, pos);
        cmds.push_back(token);
        buffer.erase(0, pos + delim.length());
    }
    cmds.push_back(buffer);

	for (std::vector<std::string>::iterator it = cmds.begin(); it < cmds.end() ;it++)
	{
		if ((*it).empty())
			continue;
		char *token = std::strtok(&(*it)[0], " ");
		std::vector<std::string> tmp;
		while (token != NULL)
		{
			tmp.push_back(token);
			token = std::strtok(NULL, " ");
		}
		tokens.push_back(tmp);
	}

	for (std::vector<std::vector<std::string> >::iterator it = tokens.begin();it < tokens.end();it++)
	{
		if ((*it)[0] == "CAP")
		{
			client->type = client->IRSSI;
			continue ;
		}
		if ((*it)[0] == "PASS" && !pass_cmd(*it, client))
			break ;
		if ((*it)[0] == "NICK" && !nick_cmd(*it, client))
			break ;
		if ((*it)[0] == "USER" && !user_cmd(*it, client))
			break ;
		if ((*it)[0] == "OPER" && !oper_cmd(*it, client))
			break ;
		if ((*it)[0] == "QUIT" && quit_cmd(*it, client))
			break ;
		if ((*it)[0] == "PRIVMSG" && !privmsg_cmd(*it, client))
			break ;
		if ((*it)[0] == "JOIN" && join_cmd(*it, client))
			break ;
		if ((*it)[0] == "PING" && !ping_cmd(*it, client))
			break ;
		if ((*it)[0] == "TOPIC" && !topic_cmd(*it, client))
			break ;
		if ((*it)[0] == "KICK" && !kick_cmd(*it, client))
			break ;
		if ((*it)[0] == "INVITE" && !invite_cmd(*it, client))
			break ;
		if ((*it)[0] == "MODE" && !mode_cmd(*it, client))
			break ;
	}
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
		this->clients[index].client_fd = -1;
		this->clients[index].is_used = false;
		return ;
	}
	if (revents & EPOLLIN)
	{
		recv_ret = recv(client_fd, buffer, sizeof(buffer), 0);
		if (recv_ret == 0)
		{
			std::cerr << "Client " << this->clients[index].src_ip << ":" << this->clients[index].src_port << " has closed its connection." << std::endl;
			if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL) < 0)
				std::cerr << "epoll_ctl(EPOLL_CTL_DEL) failed." << std::endl;
			close(client_fd);
			this->clients[index].client_fd = -1;
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
			this->clients[index].client_fd = -1;
			this->clients[index].is_used = false;
			return ;
		}
		buffer[recv_ret] = '\0';
		std::string buf = buffer;
		if (buffer[recv_ret - 2] == '\r')
			buf = buf.substr(0, recv_ret - 2);
		debug(buf, &this->clients[index], 0);
		parse_cmd(buf, &this->clients[index]);
        epoll_mod(this->_epoll_fd, client_fd, EPOLLIN | EPOLLOUT, index);
	}
	if (revents & EPOLLOUT)
	{
		for (std::vector<std::pair<Client *, std::string> >::iterator it = this->clients[index].sendto.begin();it != this->clients[index].sendto.end();it++)
		{
			debug((*it).second, (*it).first, 1);
			ssize_t sent = send((*it).first->client_fd, (*it).second.c_str(), (*it).second.size(), 0);
			if (sent < 0) {
				if (errno == EAGAIN)
					return;
				std::cerr << "Error: send() failed." << std::endl;
				close(client_fd);
				this->clients[index].client_fd = -1;
				this->clients[index].is_used = false;
				epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
				return;
			}
			(*it).second.clear();
		}
		this->clients[index].sendto.clear();
		epoll_mod(this->_epoll_fd, client_fd, EPOLLIN, index);
    }
}

void Server::run()
{
	memset(&this->_serverAddr, 0, sizeof(this->_serverAddr));
	this->_serverAddr.sin_family = AF_INET;
	this->_serverAddr.sin_addr.s_addr = INADDR_ANY;
	this->_serverAddr.sin_port = htons(this->_port);
	if (bind(this->_sock_fd, (struct sockaddr *)&this->_serverAddr, sizeof(this->_serverAddr)) < 0)
	{
		std::cerr << "Error: binding failed." << std::endl;
		close(this->_sock_fd);
		this->_sock_fd = -1;
		Server::stop();
	}
	if (listen(this->_sock_fd, 10) < 0)
	{
		std::cerr << "Error: listen() failed." << std::endl;
		close(this->_sock_fd);
		this->_sock_fd = -1;
		Server::stop();
	}

	debug("Server running.", NULL, 0);
	
	epoll_add_new(this->_epoll_fd, this->_sock_fd, EPOLLIN);

	std::cout << "INFO: " << getTime() << ": " << "Server listening on port " << this->_port << "." << std::endl;

	int epoll_ret;
	int err;
	struct epoll_event events[32];

	while (1)
	{
		epoll_ret = epoll_wait(this->_epoll_fd, events, 32, 5000);
		if (epoll_ret == 0)
		{
			std::cout << "no events within 5000 milliseconds." << std::endl;
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
					Server::stop();
				continue ;
			}
			handle_client_event(fd, events[i].events);
		}
	}
}

void Server::stop()
{
	debug("Server stopping", NULL, 0);
	Server::~Server();
}
