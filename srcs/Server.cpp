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

	// Channel *oi = new Channel("#OI");
	// Channel *channel1 = new Channel("#Channel1");
	// this->channels.insert(oi);
	// this->channels.insert(channel1);

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
	for (std::set<Channel *>::iterator it = this->channels.begin();it != this->channels.end();it++)
	{
		// (*it)->name.clear();
		// (*it)->topic.clear();
		// (*it)->key.clear();
		// (*it)->users.clear();
		// (*it)->operators.clear();
		delete *it;
	}
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
			// std::cout << getTime() << "Client " << src_ip << ":" << src_port << " has been accepted!" << std::endl;
			return (0);
		}
	}
	debug("Slot is full, can't accept any more client at the moment.", NULL, 0);
	// std::cout << getTime() << "Slot is full, can't accept any more client at the moment." << std::endl;
	close(client_fd);
	return (0);
}

bool Server::pass_cmd(std::vector<std::string> params, Client *client)
{
	client->sendto.insert(client);
	if (params.size() == 1 || params[1].empty())
		return (client->send_buffer += "461 :",
				// client->send_buffer += client->nick,
				client->send_buffer += " <PASS> :Not enough parameters\n", false);
	// params[1].erase(params[1].size() - 1);
	std::cout << "KEK: " << params[1] << " | " << this->_password << (params[1] != this->_password) << std::endl;
	if (params[1] != this->_password)
		return (client->send_buffer += "464 :",
				// client->send_buffer += client->nick,
				client->send_buffer += " Password incorrect\n", false);
	if (client->registered)
		return (client->send_buffer += "462 :",
				// client->send_buffer += client->nick,
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

Client *Server::nick_exist(const std::string& nick)
{
	for (int i = 0; i < MAX_NB_CLIENT; i++)
	{
		if (this->clients[i].is_used && this->clients[i].nick == nick)
			return (&this->clients[i]);
	}
	return (NULL);
}

Channel *Server::channel_exist(const std::string& channel)
{
	for (std::set<Channel *>::iterator it = this->channels.begin();it != this->channels.end();it++)
	{
		if ((*it)->name == channel)
		{
			return ((*it));
		}
	}
	return (NULL);
}

bool Server::nick_cmd(std::vector<std::string> params, Client *client)
{
	std::string msg;
	if (params.size() == 1 || params[1].empty())
		return (client->send_buffer += "431 :",
				// client->send_buffer += client->nick,
				client->send_buffer += "No nickname given\n", false);
	// params[1].erase(params[1].size() - 1);
	if (!is_valid_nick(params[1]))
		return (client->send_buffer += "432 :",
				// client->send_buffer += params[1],
				client->send_buffer += "Erroneus nickname\n", false);
	if (nick_exist(params[1]))
		return (client->send_buffer += "433 :",
				client->send_buffer += params[1],
				client->send_buffer += " :Nickname is already in use\n", false);
	if (client->registered)
	{
		debug("Changing nickname.", client, 0);
		// std::cout << getTime() << "Changing nickname" << std::endl;
		msg = ":";
		msg += client->nick;
		msg += " NICK ";
		msg += params[1];
		msg += "\n";
		client->nick = params[1];
		return (client->send_buffer += msg, false);
	}
	client->nick = params[1];
	client->sendto.insert(client);
	return (client->send_buffer += "\n", true);
}

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
	// params[1].erase(params[1].size() - 1);
	// params[3].erase(params[3].size() - 1);
	// params[4].erase(params[4].size() - 1);
	if (client->registered)
		return (client->send_buffer += "462 :",
				client->send_buffer += client->nick,
				client->send_buffer += " :You may not reregister\n", false);
	client->user = params[1];
	// client->user = params[3] + params[4];
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
	msg += " :This server was created ";
	msg += getTime();
	msg += "\n";
	client->registered = true;
	client->sendto.insert(client);
	return (client->send_buffer += msg, true);
}

bool Server::oper_cmd(std::vector<std::string> params, Client *client)
{
	if (params.size() < 3 || params[1].empty() || params[2].empty())
		return (client->send_buffer += "461 : ",
			client->send_buffer += client->nick,
			client->send_buffer += "<OPER> :Not enough parameters\n", false);
	// params[2].erase(params[2].size() - 1);
	std::map<std::string, std::string>::iterator it = this->operators.find(params[1]);
	if (it == this->operators.end() || params[2] != it->second)
		return (client->send_buffer += "464 :",
			client->send_buffer += client->nick,
			client->send_buffer += " :Password incorrect\n", false);
	client->is_operator = true; 
	client->send_buffer += "381 : ";
	client->send_buffer += client->nick;
	client->send_buffer += " :You are now an IRC operator\n";
	client->sendto.insert(client);
	return (true);
}

bool Server::ping_cmd(std::vector<std::string> params, Client *client)
{
	(void)client;
	std::string msg;
	// params[1].erase(params[1].size() - 1);
	msg = "PONG KEKserver ";
	msg += params[1];
	msg += "\n";
	client->sendto.insert(client);
	return (client->send_buffer += msg, true);
}

bool Server::privmsg_cmd(std::vector<std::string> params, Client *client)
{
	if (params.size() < 3 || params[1].empty() || params[2].empty())
		return (client->send_buffer += "461 : ",
			client->send_buffer += client->nick,
			client->send_buffer += " <PRIVMSG> :Not enough parameters\n", false);
	// char *rec = std::strtok(&params[1][0], ",");
	// std::vector<std::string> recipients;
	// while (rec != NULL)
	// {
	// 	recipients.push_back(rec);
	// 	rec = std::strtok(NULL, ",");
	// }
	// for (std::vector<std::string>::iterator it = recipients.begin(); it != recipients.end();it++)
	// {
	(params[1]).erase((params[1]).find_last_not_of(' ') + 1);
	(params[1]).erase(0, (params[1]).find_first_not_of(' '));
	std::cout << "debug: nick/channel: " << (params[1]) << std::endl;
	if ((params[1])[0] != '#')
	{
		Client *c = nick_exist((params[1]));
		if (!c)
			return (client->sendto.insert(client),
					client->send_buffer += "401 : ",
					client->send_buffer += client->nick,
					client->send_buffer += " ",
					client->send_buffer += (params[1]),
					client->send_buffer += " :No such nick\n", false);
		client->sendto.insert(c);
		// continue ;
	}
	else
	{
		Channel *cc = channel_exist(params[1]);

		if (!cc)
			return (client->sendto.insert(client),
				client->send_buffer += "401 : ",
				client->send_buffer += client->nick,
				client->send_buffer += " ",
				client->send_buffer += (params[1]),
				client->send_buffer += " :No such channel\n", false);
		if (!cc->is_user(client))
			return (client->sendto.insert(client),
				client->send_buffer += "404 : ",
				client->send_buffer += client->nick,
				client->send_buffer += " ",
				client->send_buffer += (params[1]),
				client->send_buffer += " :Cannot send to channel\n", false);
		for (std::vector<Client *>::iterator itt = cc->users.begin();itt != cc->users.end();itt++)
		{
			if (client->nick != (*itt)->nick)
				client->sendto.insert(*itt);
			std::cout << "sendto: " << (*itt)->nick << std::endl;
		}
	}
	// }
	client->send_buffer += ":";
	client->send_buffer += client->nick;
	client->send_buffer += " PRIVMSG ";
	client->send_buffer += params[1]; 
	client->send_buffer += " ";
	for (std::vector<std::string>::iterator it = params.begin() + 2;it != params.end();it++)
	{
		client->send_buffer += (*it).c_str();
		if (it < params.end() - 1)
			client->send_buffer += " ";
	}
	client->send_buffer += "\n";
	std::cout << "debug: privmsg: " << client->send_buffer << std::endl;
	return (true);
}

bool Server::join_cmd(std::vector<std::string> params, Client *client)
{
	std::string key;
	std::cout << "debug: channel: " << std::endl;
	if (params.size() < 2 || params[1].empty())
		return (client->send_buffer += "461 : ",
			client->send_buffer += client->nick,
			client->send_buffer += " JOIN :Not enough parameters\n", false);
	std::cout << "chan: " << params[1] << std::endl;
	// char *chan = std::strtok(&params[1][0], ",");
	// std::vector<std::string> channellist;
	// while (chan != NULL)
	// {
	// 	channellist.push_back(chan);
	// 	chan = std::strtok(NULL, ",");
	// }
	// std::cout << "chan: " << channellist[0] << std::endl;
	// std::vector<std::string> keylist;
	// if (params.size() == 3 && !params[2].empty())
	// {
	// 	char *chankey = std::strtok(&params[2][0], ",");
	// 	while (chankey != NULL)
	// 	{
	// 		keylist.push_back(chankey);
	// 		chankey = std::strtok(NULL, ",");
	// 	}
	// }
	// else
	// 	keylist.push_back("");
	// std::vector<std::string>::iterator kit = keylist.begin();
	// for (std::vector<std::string>::iterator it = channellist.begin(); it != channellist.end();it++)
	// {
	// params[1].erase(params[1].size() - 1);
	if (params.size() != 3 || params[2].empty())
		key =  "";
	(params[1]).erase((params[1]).find_last_not_of(' ') + 1);
	(params[1]).erase(0, (params[1]).find_first_not_of(' '));
	std::cout << "debug: channel: " << params[1] << std::endl;

	Channel *c = channel_exist((params[1]));
	if (!c)
	{
		Channel *cc = new Channel(params[1]);
		std::cout << "KEK: " << (params[1]).size() << " | " << cc->name.size() << std::endl;
		c = cc;
		this->channels.insert(cc);
	}
		// return (client->sendto.insert(client),
		// 	client->send_buffer += "403 : ",
		// 	client->send_buffer += client->nick,
		// 	client->send_buffer += " ",
		// 	client->send_buffer += (*it),
		// 	client->send_buffer += " :No such channel\n", false);
	if (c->users.size() == c->limit)
		return (client->sendto.insert(client),
			client->send_buffer += ":localhost 471 :",
			client->send_buffer += client->nick,
			client->send_buffer += " ",
			client->send_buffer += c->name,
			client->send_buffer += " :Cannot join channel (+l)\n", false);
	if (key != c->key)
		return (client->sendto.insert(client),
			client->send_buffer += "475 : ",
			client->send_buffer += client->nick,
			client->send_buffer += " ",
			client->send_buffer += c->name,
			client->send_buffer += " :Cannot join channel (+k)\n", false);
	c->users.push_back(client);
	std::cout << "OI: " << c->users.size() << this->channels.size() << std::endl;
	client->send_buffer += ":";
	client->send_buffer += client->nick;
	client->send_buffer += " JOIN ";
	client->send_buffer += c->name;
	client->send_buffer += "\n";
	// kit++;
	// }
	std::cout << "debug: join: " << client->send_buffer << std::endl;
	return (true);
}


bool Server::quit_cmd(std::vector<std::string> params, Client *client)
{
	(void)params;
	// std::string msg = "Quit: ";
	// msg += params[1];
	// debug(msg, client, 0);
	// params[1].erase(params[1].size() - 1);
	// client->quit = true;
	// if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client->client_fd, NULL) < 0)
	// 		std::cerr << "epoll_ctl(EPOLL_CTL_DEL) failed." << std::endl;
	// close(client->client_fd);
	client->is_used = false;
	client->registered = false;
	client->authenticated = false;
	// client->client_fd = -1;
	return (true);
// 	return (client->send_buffer += "ERROR :Connection timeout\n", true);
}

void Server::parse_cmd(std::string buffer, int client_fd, uint32_t index)
{
	// std::cout << "Client " << this->clients[index].src_ip << ":" << this->clients[index].src_port << " sends: " << buffer << std::endl;
	(void)client_fd;
	// std::cout << "buffer: " << buffer << std::endl;
	char *cmd = std::strtok(&buffer[0], "\r\n");
	std::vector<std::string> cmds;
	std::vector<std::vector<std::string> > tokens;
	while (cmd != NULL)
	{
		cmds.push_back(cmd);
		cmd = std::strtok(NULL, "\r\n");
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
		if ((*it)[0] == "PASS" && !pass_cmd(*it, &this->clients[index]))
			break ;
		if ((*it)[0] == "NICK" && !nick_cmd(*it, &this->clients[index]))
			break ;
		if ((*it)[0] == "USER" && !user_cmd(*it, &this->clients[index]))
			break ;
		if ((*it)[0] == "OPER" && !oper_cmd(*it, &this->clients[index]))
			break ;
		if ((*it)[0] == "QUIT" && quit_cmd(*it, &this->clients[index]))
			break ;
		if ((*it)[0] == "PRIVMSG" && privmsg_cmd(*it, &this->clients[index]))
			break ;
		if ((*it)[0] == "JOIN" && join_cmd(*it, &this->clients[index]))
			break ;
		if ((*it)[0] == "PING" && !ping_cmd(*it, &this->clients[index]))
			break ;
		// if ((*it)[0] == "WHOIS" && whois_cmd(*it, &this->clients[index]))
		// 	break ;
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
		std::cout << "buffersize: " << recv_ret << std::endl;
		buffer[recv_ret] = '\0';
		std::string buf = buffer;
		buf = buf.substr(0, recv_ret - 2);
		// if (buffer[recv_ret - 2] == '\r')
		// 	buffer[recv_ret - 2] = '\0';
		// if (buffer[recv_ret - 1] == '\n')
		// 	buffer[recv_ret - 1] = '\0';
		debug(buf, &this->clients[index], 0);
		// std::cout << getTime() << ": Client " << this->clients[index].src_ip << ":" << this->clients[index].src_port << " sends: " << buffer << std::endl;
		parse_cmd(buf, client_fd, index);
        epoll_mod(this->_epoll_fd, client_fd, EPOLLIN | EPOLLOUT, index);
	}
	if (revents & EPOLLOUT)
	{
        std::string& send_buffer = this->clients[index].send_buffer;
        if (!send_buffer.empty())
		{
			// debug(send_buffer, &this->clients[index], 1);
			// std::cout << "debug: vector: " << this->clients[index].sendto.size() << std::endl;
			for (std::set<Client *>::iterator it = this->clients[index].sendto.begin();it != this->clients[index].sendto.end();it++)
			{
				std::cout << "send_buffer: " << send_buffer << std::endl;
				ssize_t sent = send((*it)->client_fd, send_buffer.c_str(), send_buffer.size(), 0);
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
			}
			// send_buffer.erase(0, sent);
			send_buffer.clear();
			this->clients[index].sendto.clear();
			if (send_buffer.empty()) {
				epoll_mod(this->_epoll_fd, client_fd, EPOLLIN, index);
			}
			// if (this->clients[index].quit)
			// {
            //     close(client_fd);
			// 	this->clients[index].client_fd = -1;
            //     this->clients[index].is_used = false;
			// 	this->clients[index].quit = false;
			// 	this->clients[index].src_ip.clear();
			// 	this->clients[index].src_port = 0;
			// 	this->clients[index].send_buffer.clear();
			// 	this->clients[index].registered = false;
			// 	this->clients[index].authenticated = false;
			// 	this->clients[index].is_operator = false;
			// 	this->clients[index].nick.clear();
			// 	this->clients[index].realname.clear();
			// 	// epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
			// }
        }
    }
}

std::string Server::getTime() const
{
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80];
	std::string res;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 80, "[%F %T]: ", timeinfo);
	res = buffer;
	return (res);
}

void Server::debug(std::string msg, Client *from, bool to)
{
	msg.erase(msg.find_last_not_of('\n') + 1);
	if (from && !to)
		std::cout << "INFO: " << getTime() << from->src_ip << ":" << from->src_port << ": Send: " << msg << std::endl;
	else if (to && from)
		std::cout << "INFO: " << getTime() << "Server send" << msg << " to " << from->src_ip << ":" << from->src_port << std::endl;
	if (!from)
		std::cout << "INFO: " << getTime() << msg << std::endl;
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
	// std::cout << getTime() << "Server running." << std::endl;
	
	epoll_add_new(this->_epoll_fd, this->_sock_fd, EPOLLIN);

	// debug("Server listening on port " + std::to_string(this->_port), NULL, 0);
	std::cout << "INFO: " << getTime() << "Server listening on port " << this->_port << "." << std::endl;

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
}

void Server::stop()
{
	debug("Server stopping", NULL, 0);
	// std::cout << getTime() << "Server stopping" << std::endl;
	Server::~Server();
}
