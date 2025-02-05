#include "../includes/Server.hpp"
// #include "cmds.cpp"

Server::Server(unsigned int port, const std::string& password): _port(port), _password(password)
{
	this->connection = true;
	// const size_t client_slot_num = sizeof(this->clients) / sizeof(*this->clients);
	// const size_t client_slot_map_num = sizeof(this->client_map) / sizeof(*this->client_map);

	// for (size_t i = 0; i < client_slot_num; i++)
	// {
	// 	this->clients[i].is_used = false;
	// 	this->clients[i].client_fd = -1;
	// }

	// for (uint16_t i = 0; i < client_slot_map_num; i++)
	// 	this->client_map[i] = 0u;

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

	std::ifstream file("./wordlist.txt");

	if (!file.is_open())
	{
		std::cerr << "Error opening file" << std::endl;
		Server::stop();
	}
	std::string line;
	while (std::getline(file, line))
		wordlist.push_back(line);
	file.close();
}

Server::~Server()
{
	if (this->_sock_fd >= 0)
		close(this->_sock_fd);
	this->wordlist.~vector();
	this->sendto.~vector();
	this->clients.~vector();
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

void Server::epoll_mod(int epoll_fd, int client_fd, uint32_t events)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = events;
    event.data.fd = client_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &event) < 0) {
        std::cerr << "Error: epoll_ctl(EPOLL_CTL_MOD) failed." << std::endl;
        close(client_fd);
		// this->clients[index].client_fd = -1;
		// Server::stop();
    }
}

void Server::sendTo(const std::string &msg)
{
	this->sendto.push_back(msg);
}

Server::client *Server::clientexist(const std::string &name)
{
	for (std::vector<client>::iterator it = this->clients.begin();it != this->clients.end();it++)
	{
		if ((*it).name == name)
			return (&(*it));
	}
	return (NULL);
}

void Server::rmclient(const std::string &name)
{
	for (std::vector<client>::iterator it = this->clients.begin();it != this->clients.end();it++)
	{
		if ((*it).name == name)
			return (this->clients.erase(it), (void)name);
	}
}

bool Server::privmsg_cmd(std::vector<std::string> params)
{
	std::string token = "";
	std::string to;
	params[0].erase(0, 1);
	to = params[0].substr(0, params[0].find_first_of('!'));
	if (params.size() < 5 || params[4].empty())
		return (this->sendTo("PRIVMSG " + to + " :" ERR_COLOR "Not enough parameters" RESET_COLOR "\r\n"), false);
	std::cout << "message: " << std::endl;
	int i = 0;
	for (std::vector<std::string>::iterator it = params.begin();it != params.end();it++)
		std::cout << i++ << ": " << (*it) << " | ";
	std::cout << std::endl;
	params[3].erase(0, 1);
	// for (std::string::iterator it = params[4].begin();it != params[4].end();it++)
	// 	std::cout << +(*it) << " | " << std::endl;
	// std::string kek = "start";
	// std::cout << "params[3]: " << params[3] << std::endl;
	// std::cout << "params[4]: " << params[4].size() << kek.size() << "." << std::endl;
	if (params[3] != "/bot")
		return (this->sendTo("PRIVMSG " + to + " :" ERR_COLOR "Unknown command" RESET_COLOR "\r\n"), false);
	if (params[4] == "start")
	{
		client c;

		c.name = to;
		c.attemptsnb = 7;
		c.wordtoguess = this->wordlist[rand() % this->wordlist.size()];
		c.letters = "";
		std::cout << "wordtoguess: " << c.wordtoguess << std::endl;
		std::string hiddenword(c.wordtoguess.size(), '*');
		c.hiddenword = hiddenword;
		std::stringstream q;
		std::string f;
		q << c.attemptsnb;
		f.append(q.str());
		if (!this->clientexist(c.name))
			this->clients.push_back(c);
		else
			return (this->sendTo("PRIVMSG " + to + " :" ERR_COLOR "You already have started a game, please stop the current one before starting a new one." RESET_COLOR "\r\n"), false);
		this->sendTo("PRIVMSG " + to + " :" MSG_COLOR "Word to guess (" + f + " attempt left): " + hiddenword + "" RESET_COLOR "\r\n");
		this->sendTo("PRIVMSG " + to + " :" MSG_COLOR + attempt1 "" RESET_COLOR "\r\n");
		return (true);
	}
	if (params.size() < 5 && params[4] == "guess")
		return (this->sendTo("PRIVMSG " + to + " :" ERR_COLOR "GUESS You must have forgot the letter to guess, dumbass." RESET_COLOR "\r\n"), false);
	if (params[4] == "guess")
	{	
		if (params.size() < 6 || params[5].empty())
			return (this->sendTo("PRIVMSG " + to + " :" ERR_COLOR "Not enough parameters" RESET_COLOR "\r\n"), false);
		if (!std::isalpha(params[5][0]))
			return (this->sendTo("PRIVMSG " + to + " :" ERR_COLOR "\"" + params[5][0] + "\" is not a letter." RESET_COLOR "\r\n"), false);
		client *c = this->clientexist(to);
		unsigned int i = 0;
		if (!c)
			return (this->sendTo("PRIVMSG " + to + " :" ERR_COLOR "No game running, so there is nothing to guess dumbass." RESET_COLOR "\r\n"), false);
		std::string::iterator it = std::find(c->letters.begin(), c->letters.end(), params[5][0]);
		if (it != c->letters.end())
			return (this->sendTo("PRIVMSG " + to + " :" ERR_COLOR "You already tried this letter, try another one." RESET_COLOR "\r\n"), true);
		c->letters.append(&params[5][0]);
		for (size_t s = 0; s < c->wordtoguess.size();s++)
		{
			if (c->wordtoguess[s] == params[5][0])
			{
				c->hiddenword[s] = c->wordtoguess[s];
				std::cout << "c->hiddenword: " << c->hiddenword << std::endl;
				i++;
			}
		}
		std::stringstream q;
		std::string f;
		if (!i)
		{
			c->attemptsnb--;
			q << c->attemptsnb;
			f.append(q.str());
			if (c->attemptsnb == 0)
				return (this->rmclient(to), this->sendTo("PRIVMSG " + to + " :" MSG_COLOR gameover RESET_COLOR "\r\n"), true);
			this->sendTo("PRIVMSG " + to + " :" MSG_COLOR "The letter \"" + params[5][0] + "\" is not in the word to guess." RESET_COLOR "\r\n");
			this->sendTo("PRIVMSG " + to + " :" MSG_COLOR "Word to guess (" + f + " attempt left): " + c->hiddenword + "" RESET_COLOR "\r\n");
			return (true);
		}
		if (!c->wordtoguess.compare(c->hiddenword))
			return (this->rmclient(to), this->sendTo("PRIVMSG " + to + " :" MSG_COLOR "Congratulation, you guessed the word !!" RESET_COLOR "\r\n"), true);
		q << c->attemptsnb;
		f.append(q.str());
		this->sendTo("PRIVMSG " + to + " :" MSG_COLOR "Word to guess (" + f + " attempt left): " + c->hiddenword + "" RESET_COLOR "\r\n");
		return (true);
	}
	if (params[4] == "stop")
	{
		client *c = this->clientexist(to);
		std::string word;
		if (!c)
			return (this->sendTo("PRIVMSG " + to + " :" ERR_COLOR "No game running." RESET_COLOR "\r\n"), false);
		word = c->wordtoguess;
		this->rmclient(to);
		this->sendTo("PRIVMSG " + to + " :" MSG_COLOR + "Current game stopped, the word to guess was " + word + "" RESET_COLOR "\r\n");
		return (true);
	}

	return (this->sendTo("PRIVMSG " + to + " :" ERR_COLOR "Unknown command" RESET_COLOR "\r\n"), false);
	// std::string name = "bot";
	// std::cout << "privmsg_cmd: " << token << std::endl;
	// return (this->sendTo(SEND_PRIVMSG(name, name, to, token)), true);
}

void Server::parse_cmd(std::string buffer)
{
	std::vector<std::string> cmds;
	std::vector<std::vector<std::string> > tokens;

	char *cmd = std::strtok(&buffer[0], "\r\n");
	while (cmd != NULL)
	{
		cmds.push_back(cmd);
		cmd = std::strtok(NULL, "\r\n");
	}

	for (std::vector<std::string>::iterator it = cmds.begin(); it < cmds.end() ;it++)
	{
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
		if ((*it).size() > 1)
		{
			std::cout << "vec: " << (*it)[1] << std::endl;
			if ((*it)[1] == "PRIVMSG" && privmsg_cmd(*it))
				break ;
		}
	}
}

void Server::handle_client_event(int client_fd, uint32_t revents)
{
	int err;
	ssize_t recv_ret;
	char buffer[1024];
	const uint32_t err_mask = EPOLLERR | EPOLLHUP;

	// uint32_t index = this->client_map[client_fd] - 1u;
	
	if (revents & err_mask)
	{
		std::cerr << "revents & err_mask Server has closed its connection" << std::endl;
		// std::cerr << "Client " << this->clients[index].src_ip << ":" << this->clients[index].src_port << " has closed its connection." << std::endl;
		if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL) < 0)
			std::cerr << "epoll_ctl(EPOLL_CTL_DEL) failed." << std::endl;
		close(client_fd);
		Server::~Server();
	}
	if (revents & EPOLLIN)
	{
		recv_ret = recv(client_fd, buffer, sizeof(buffer), 0);
		if (recv_ret == 0)
		{
			std::cerr << "recv() == 0 Server has closed its connection" << std::endl;
			// std::cerr << "Client " << this->clients[index].src_ip << ":" << this->clients[index].src_port << " has closed its connection." << std::endl;
			if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL) < 0)
				std::cerr << "epoll_ctl(EPOLL_CTL_DEL) failed." << std::endl;
			close(client_fd);
			Server::~Server();
		}
		if (recv_ret < 0)
		{
			err = errno;
			if (err == EAGAIN)
				return ;
			std::cerr << "recv() failed." << std::endl;
			std::cerr << "recv() < 0 Server has closed its connection" << std::endl;
			// std::cerr << "Client " << this->clients[index].src_ip << ":" << this->clients[index].src_port << " has closed its connection." << std::endl;
			if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL) < 0)
				std::cerr << "epoll_ctl(EPOLL_CTL_DEL) failed." << std::endl;
			close(client_fd);
			Server::~Server();
		}
		buffer[recv_ret] = '\0';
		std::string buf = buffer;
		if (buffer[recv_ret - 2] == '\r')
			buf = buf.substr(0, recv_ret - 2);
		debug(buf);
		parse_cmd(buf);
        epoll_mod(this->_epoll_fd, client_fd, EPOLLIN | EPOLLOUT);
	}
	if (revents & EPOLLOUT)
	{
		if (connection)
		{
			std::string msg = "PASS ";
			msg.append(this->_password + "\r\n");
			this->sendTo(msg);
			this->sendTo("NICK bot\r\n");
			this->sendTo("USER bot bot localhost :Botname\r\n");
			// this->sendTo("JOIN #bot\r\n");
			connection = false;
		}
		for (std::vector<std::string>::iterator it = this->sendto.begin();it != this->sendto.end();it++)
		{
			std::cout << "sendto: " << *it << std::endl;
			ssize_t sent = send(this->_sock_fd, (*it).c_str(), (*it).size(), 0);
			if (sent < 0) {
				if (errno == EAGAIN)
					return;
				std::cerr << "Error: send() failed." << std::endl;
				close(client_fd);
				epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
				return;
			}
		}
		this->sendto.clear();
		epoll_mod(this->_epoll_fd, client_fd, EPOLLIN);
    }
}

void Server::run()
{
	memset(&this->_serverAddr, 0, sizeof(this->_serverAddr));
	this->_serverAddr.sin_family = AF_INET;
	this->_serverAddr.sin_addr.s_addr = INADDR_ANY;
	this->_serverAddr.sin_port = htons(this->_port);
	if (connect(this->_sock_fd, (struct sockaddr *)&this->_serverAddr, sizeof(this->_serverAddr)) < 0)
	{
		if(errno != EINPROGRESS) {
			std::cerr << "Error: connect failed." << std::endl;
			close(this->_sock_fd);
			this->_sock_fd = -1;
			Server::stop();
        }
	}
	debug("BOT running.");
	
	epoll_add_new(this->_epoll_fd, this->_sock_fd, EPOLLOUT);

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

			// if (fd == this->_sock_fd)
			// {
			// 	if (accept_new_client(fd) < 0)
			// 		Server::stop();
			// 	continue ;
			// }
			handle_client_event(fd, events[i].events);
		}
	}
}

void Server::stop()
{
	debug("Server stopping");
	Server::~Server();
}
