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
	this->_sock_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (this->_sock_fd < 0)
	{
		std::cerr << "Error: socket opening failed." << std::endl;
		Server::stop();
	}
	this->operators.insert(std::pair<std::string, std::string>("pmagnero", "adminpass"));
	// if (setsockopt(this->_sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)1, sizeof(int)) < 0)
	// {
	// 	std::cerr << "Error: setsockopt() failed." << std::endl;
	// 	close(this->_sock_fd);
	// 	exit(1);
	// }
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
		// Server::stop();
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
			std::cout << "Client " << src_ip << ":" << src_port << " has been accepted!" << std::endl;
			return (0);
		}
	}
	std::cout << "Slot is full, can't accept any more client at the moment." << std::endl;
	close(client_fd);
	return (0);
}

bool Server::pass_cmd(std::vector<std::string> params, Client *client)
{
	if (params.size() == 1 || params[1].empty())
		return (client->send_buffer += "461 :",
				client->send_buffer += client->nick,
				client->send_buffer += " <PASS> :Not enough parameters\n", false);
	params[1].erase(params[1].size() - 1);
	if (params[1] != this->_password)
		return (client->send_buffer += "464 :",
				client->send_buffer += client->nick,
				client->send_buffer += " :Password incorrect\n", false);
	if (client->registered)
		return (client->send_buffer += "462 :",
				client->send_buffer += client->nick,
				client->send_buffer += " :You may not reregister\n", false);
	client->authenticated = true;
	client->send_buffer += "\n";
	return (true);
}

bool Server::is_valid_nick(const std::string& nick)
{
	if (std::isdigit(nick[0]))
		return (false);
	for (std::string::const_iterator it = nick.begin(); it < nick.end(); it++)
	{
		if (!std::isalnum(*it) && *it != '[' && *it != ']' && *it != '{' && *it != '}'
			&& *it != '\\' && *it != '|')
			return (false);
	}
	return (true);
}

bool Server::nick_exist(const std::string& nick)
{
	for (int i = 0; i < MAX_NB_CLIENT; i++)
	{
		// std::cout << "nick: " << this->clients[i].nick << " nick2: " << nick << std::endl;
		if (this->clients[i].nick == nick)
			return (true);
	}
	return (false);
}

bool Server::nick_cmd(std::vector<std::string> params, Client *client)
{
	std::string msg;
	if (params.size() == 1 || params[1].empty())
		return (client->send_buffer += "431 :",
				client->send_buffer += client->nick,
				client->send_buffer += " :No nickname given\n", false);
	params[1].erase(params[1].size() - 1);
	if (!is_valid_nick(params[1]))
		return (client->send_buffer += "432 :",
				client->send_buffer += client->nick,
				client->send_buffer += " :Erroneus nickname\n", false);
	if (nick_exist(params[1]))
		return (client->send_buffer += "433 :",
				client->send_buffer += client->nick,
				client->send_buffer += " :Nickname is already in use\n", false);
	if (client->registered)
	{
		std::cout << "Changing nickname" << std::endl;
		msg = ":";
		msg += client->nick;
		msg += " NICK ";
		msg += params[1];
		msg += "\n";
		client->nick = params[1];
		return (client->send_buffer += msg, false);
	}
	client->nick = params[1];
	return (client->send_buffer += "\n", true);
}

//TODO: Add Datetime of creation of the server
//TODO: Add better message
bool Server::user_cmd(std::vector<std::string> params, Client *client)
{
	std::string msg;
	if (!client->authenticated)
		return (client->send_buffer += "451 :",
				client->send_buffer += client->nick,
				client->send_buffer += " : You have not registered\n", false);
	if (params.size() == 1 || params[1].empty())
		return (client->send_buffer += "461 :",
				client->send_buffer += client->nick,
				client->send_buffer += " <USER> :Not enough parameters\n", false);
	params[1].erase(params[1].size() - 1);
	params[3].erase(params[3].size() - 1);
	params[4].erase(params[4].size() - 1);
	if (client->registered)
		return (client->send_buffer += "462 :",
				client->send_buffer += client->nick,
				client->send_buffer += " :You may not reregister\n", false);
	client->user = params[1];
	client->user = params[3] + params[4];
	msg = "001 ";
	msg += client->nick;
	msg += " :Welcome to the KEK Network ";
	msg += client->nick;
	msg += "\n";
	msg += "002 ";
	msg += client->nick;
	msg += " :Your host is KEKservername, running version 3.0\n";
	msg += "003 ";
	msg += client->nick;
	msg += " :This server was created today\n";
	client->registered = true;
	return (client->send_buffer += msg, true);
}

bool Server::oper_cmd(std::vector<std::string> params, Client *client)
{
	if (params.size() < 3 || params[1].empty() || params[2].empty())
		return (client->send_buffer += "461 : ",
			client->send_buffer += client->nick,
			client->send_buffer += "<OPER> :Not enough parameters\n", false);
	params[2].erase(params[2].size() - 1);
	std::map<std::string, std::string>::iterator it = this->operators.find(params[1]);
	if (it == this->operators.end() || params[2] != it->second)
		return (client->send_buffer += "464 :",
			client->send_buffer += client->nick,
			client->send_buffer += " :Password incorrect\n", false);
	client->is_operator = true; 
	client->send_buffer += "381 : ";
	client->send_buffer += client->nick;
	client->send_buffer += " :You are now an IRC operator\n";
	return (true);
}

bool Server::ping_cmd(std::vector<std::string> params, Client *client)
{
	(void)client;
	std::string msg;
	params[1].erase(params[1].size() - 1);
	msg = "PONG KEKserver ";
	msg += params[1];
	msg += "\n";
	// std::cout << "Ping reply: " << msg << std::endl;
	return (client->send_buffer += msg, true);
}

void Server::parse_cmd(char *buffer, int client_fd, uint32_t index)
{
	std::string response;
	(void)client_fd;
	// std::cout << "buffer: " << buffer << std::endl;
	char *cmd = std::strtok(buffer, "\n");
	std::vector<std::string> cmds;
	std::vector<std::vector<std::string> > tokens;
	while (cmd != NULL)
	{
		cmds.push_back(cmd);
		cmd = std::strtok(NULL, "\n");
	}

	for (std::vector<std::string>::iterator it = cmds.begin(); it < cmds.end() ;it++)
	{
		// std::cout << "Command: " << *it << " " << std::endl;
		char *token = std::strtok(&(*it)[0], " ");
		std::vector<std::string> tmp;
		while (token != NULL)
		{
			// std::cout << "\ttoken: " << token << std::endl;
			tmp.push_back(token);
			token = std::strtok(NULL, " ");
		}
		tokens.push_back(tmp);
	}

	for (std::vector<std::vector<std::string> >::iterator it = tokens.begin();it < tokens.end();it++)
	{
		if ((*it)[0] == "CAP")
			continue ;
		if ((*it)[0] == "PASS" && !pass_cmd(*it, &this->clients[index]))
			break ;
		if ((*it)[0] == "NICK" && !nick_cmd(*it, &this->clients[index]))
			break ;
		if ((*it)[0] == "USER" && !user_cmd(*it, &this->clients[index]))
			break ;
		if ((*it)[0] == "OPER" && !oper_cmd(*it, &this->clients[index]))
			break ;
		// if ((*it)[0] == "JOIN")
		// 	join_cmd();
		if ((*it)[0] == "PING" && !ping_cmd(*it, &this->clients[index]))
			break ;
		// if ((*it)[0] == "WHOIS" && whois_cmd(*it, &this->clients[index]))
		// 	break ;
	}

	std::cout << "Client " << this->clients[index].src_ip << ":" << this->clients[index].src_port << " sends: " << buffer << std::endl;
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
		// if (buffer[recv_ret - 1] == '\n')
		// 	buffer[recv_ret - 1] = '\0';

		// std::string message = buffer;

		parse_cmd(buffer, client_fd, index);
        epoll_mod(this->_epoll_fd, client_fd, EPOLLIN | EPOLLOUT, index);
	}
	if (revents & EPOLLOUT)
	{
		// std::cout << "OIIIII: " << this->clients[index].send_buffer << std::endl;
        std::string& send_buffer = this->clients[index].send_buffer;
        if (!send_buffer.empty()) {
			std::cout << "Server: sending\n" << send_buffer << std::endl;
            ssize_t sent = send(client_fd, send_buffer.c_str(), send_buffer.size(), 0);
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
            send_buffer.erase(0, sent);
            if (send_buffer.empty()) {
                epoll_mod(this->_epoll_fd, client_fd, EPOLLIN, index);
            }
        }
    }
	// char *token = std::strtok(buffer, "\n");
	// const std::string response = "001 magnus :Welcome to the Internet Relay Network magnus";
	// const std::string response = "CAP * LS :draft/example-1 draft/example-2";
	// std::vector<std::string> tokens;
	// while (token != NULL)
	// {
	// 	std::cout << "KEK: " << token << std::endl;
	// 	tokens.push_back(token);
	// 	token = std::strtok(NULL, "\n");
	// }
	// for (size_t i = 0; i < tokens.size(); i++)
	// {
	// 	char *cmd = std::strtok(&(tokens[i])[0], " ");
	// 	while (cmd)
	// 	{
	// 		if (cmd[0] == 'C' && cmd[1] == 'A' && cmd[2] == 'P')
	// 		{
				// if (send(client_fd, response.c_str(), response.length(), 0) < 0)
				// {
				// 	err = errno;
				// 	if (err == EAGAIN)
				// 		return ;
				// 	std::cerr << "send() failed." << std::endl;
				// 	std::cerr << "Client " << this->clients[index].src_ip << ":" << this->clients[index].src_port << " has closed its connection." << std::endl;
				// 	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL) < 0)
				// 		std::cerr << "epoll_ctl(EPOLL_CTL_DEL) failed." << std::endl;
				// 	close(client_fd);
				// 	this->clients[index].is_used = false;
				// 	return ;
				// }
				// std::cout << "Server sends: " << response << " to " << this->clients[index].src_ip << ":" << this->clients[index].src_port << std::endl;
	// 		}
	// 		cmd = std::strtok(NULL, " ");
	// 	}
	// }
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

	std::cout << "Server running." << std::endl;
	
	epoll_add_new(this->_epoll_fd, this->_sock_fd, EPOLLIN);

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
					Server::stop();
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
	Server::~Server();
}
