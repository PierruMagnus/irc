#include "../includes/Server.hpp"

void Server::sendTo(Client *client, Client *to, const std::string &msg)
{
	client->sendto.push_back(std::make_pair(to, msg));
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
		if (this->clients[i].is_used && this->clients[i].registered && this->clients[i].nick == nick)
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

bool Server::pass_cmd(std::vector<std::string> params, Client *client)
{
	const std::string pre = "*";
	if (params.size() == 1 || params[1].empty())
		return (this->sendTo(client, client, ERR_NEEDMOREPARAMS(pre, "PASS")), false);
	if (params[1] != this->_password)
		return (this->sendTo(client, client, ERR_PASSWDMISMATCH(pre)), false);
	if (client->registered)
		return (this->sendTo(client, client, ERR_ALREADYREGISTERED(pre)), false);
	client->authenticated = true;
	return (true);
}

bool Server::nick_cmd(std::vector<std::string> params, Client *client)
{
	if (params.size() == 1 || params[1].empty())
		return (this->sendTo(client, client, ERR_NONICKNAMEGIVEN()), false);
	if (!is_valid_nick(params[1]))
		return (this->sendTo(client, client, ERR_ERRONEUSNICKNAME()), false);
	if (nick_exist(params[1]))
		return (this->sendTo(client, client, ERR_NICKNAMEINUSE(params[1])), false);
	if (client->registered)
	{
		debug("Changing nickname.", client, 0);
		this->sendTo(client, client, CHANGING_NICK(client->nick, client->user, params[1]));
	}
	client->nick = params[1];
	return (true);
}

bool Server::user_cmd(std::vector<std::string> params, Client *client)
{
	std::string msg = "";
	if (!client->authenticated)
		return (this->sendTo(client, client, ERR_NOTREGISTERED(client->nick)), false);
	if (params.size() < 5 || params[1].empty() || params[2].empty() || params[3].empty() || params[4].empty())
		return (this->sendTo(client, client, ERR_NEEDMOREPARAMS(client->nick, "USER")), false);
	if (client->registered)
		return (this->sendTo(client, client, ERR_ALREADYREGISTERED(client->nick)), false);
	client->user = params[1];
	client->realname = params[4];
	if (params.size() > 5)
		client->realname += params[5];
	msg = RPL_WELCOME(client->nick);
	msg += RPL_YOURHOST(client->nick);
	msg += RPL_CREATED(client->nick, getTime());
	client->registered = true;
	this->sendTo(client, client, msg);
	return (true);
}

bool Server::oper_cmd(std::vector<std::string> params, Client *client)
{
	if (params.size() < 3 || params[1].empty() || params[2].empty())
		return (this->sendTo(client, client, ERR_NEEDMOREPARAMS(client->nick, "OPER")), false);
	std::map<std::string, std::string>::iterator it = this->operators.find(params[1]);
	if (it == this->operators.end() || params[2] != it->second)
		return (this->sendTo(client, client, ERR_PASSWDMISMATCH(client->nick)), false);
	client->is_operator = true;
	this->sendTo(client, client, RPL_YOUREOPER(client->nick));
	return (true);
}

bool Server::ping_cmd(std::vector<std::string> params, Client *client)
{
	this->sendTo(client, client, PONG(params[1]));
	return (true);
}

bool Server::privmsg_cmd(std::vector<std::string> params, Client *client)
{
	std::string token = "";
	if (params.size() < 3 || params[1].empty() || params[2].empty())
		return (this->sendTo(client, client, ERR_NEEDMOREPARAMS(client->nick, "PRIVMSG")), false);
	(params[1]).erase((params[1]).find_last_not_of(' ') + 1);
	(params[1]).erase(0, (params[1]).find_first_not_of(' '));
	for (std::vector<std::string>::iterator it = params.begin() + 2;it != params.end();it++)
	{
		token += (*it).c_str();
		if (it < params.end() - 1)
			token += " ";
	}
	if ((params[1])[0] != '#')
	{
		Client *c = nick_exist((params[1]));
		if (!c)
			return (this->sendTo(client, client, ERR_NOSUCHNICK(client->nick, params[1])), false);
		return (this->sendTo(client, c, SEND_PRIVMSG(client->nick, client->user, params[1], token)), true);
	}
	else
	{
		Channel *cc = channel_exist(params[1]);
		if (!cc)
			return (this->sendTo(client, client, ERR_NOSUCHNICK(client->nick, params[1])), false);
		if (!cc->is_user(client))
			return (this->sendTo(client, client, ERR_CANNOTSENDTOCHAN(client->nick, params[1])), false);

		for (std::vector<Client *>::iterator it = cc->users.begin();it != cc->users.end();it++)
		{
			if (client->nick != (*it)->nick)
				this->sendTo(client, *it, SEND_PRIVMSG(client->nick, client->user, params[1], token));
		}
		return (true);
	}
}

bool Server::join_cmd(std::vector<std::string> params, Client *client)
{
	bool new_channel = false;
	std::string key = "";
	if (params.size() < 2 || params[1].empty())
		return (this->sendTo(client, client, ERR_NEEDMOREPARAMS(client->nick, "JOIN")), false);
	if (params.size() == 3 && !params[2].empty())
		key =  params[2];
	(params[1]).erase((params[1]).find_last_not_of(' ') + 1);
	(params[1]).erase(0, (params[1]).find_first_not_of(' '));
	Channel *c = channel_exist((params[1]));
	if (!c)
	{
		Channel *cc = new Channel(params[1]);
		c = cc;
		c->operators.push_back(client);
		this->channels.insert(cc);
		new_channel = true;
		c->key = key;
	}
	if (c->users.size() == c->limit)
		return (this->sendTo(client, client, ERR_CHANNELISFULL(client->nick, c->name)), false);
	if (key != c->key)
		return (this->sendTo(client, client, ERR_BADCHANNELKEY(client->nick, c->name)), false);
	if (!new_channel && c->mode && !c->is_invited(client))
		return (this->sendTo(client, client, ERR_INVITEONLYCHAN(client->nick, c->name)), false);
	c->rm_invite(client);
	c->users.push_back(client);
	std::string user_list;
	for (std::vector<Client *>::iterator it = c->users.begin();it != c->users.end();it++)
	{
		user_list.append((*it)->nick + " ");
		// if (client->nick != (*it)->nick)
		this->sendTo(client, *it, JOIN_CHANNEL(client->nick, client->user, c->name));
	}
	std::cout << "user_list: " << user_list << std::endl;
	if (new_channel)
		this->sendTo(client, client, RPL_NOTOPIC(client->nick, c->name));
	else
		this->sendTo(client, client, RPL_TOPIC(client->nick, c->name, c->topic));
	this->sendTo(client, client, RPL_NAMREPLY(client->nick, c->name, user_list));
	this->sendTo(client, client, RPL_ENDOFNAMES(client->nick, c->name));
	return (true);
}

bool Server::topic_cmd(std::vector<std::string> params, Client *client)
{
	std::string token = "";
	if (params.size() < 2 || params[1].empty())
		return (this->sendTo(client, client, ERR_NEEDMOREPARAMS(client->nick, "TOPIC")), false);
	Channel *c = channel_exist(params[1]);
	if (!c)
		return (this->sendTo(client, client, ERR_NOSUCHCHANNEL(client->nick, params[1])), false);
	if (!c->user_exist(client->nick))
		return (this->sendTo(client, client, ERR_NOTONCHANNEL(client->nick, params[1])), false);
	if (c->topic_mode && !c->is_operator(client))
		return (this->sendTo(client, client, ERR_CHANOPRIVSNEEDED(client->nick, client->user, params[1])), false);
	if (params.size() == 2 && !c->topic.empty())
		return (this->sendTo(client, client, RPL_TOPIC(client->nick, params[1], c->topic)), true);
	else if (params.size() == 2 && c->topic.empty())
		return (this->sendTo(client, client, RPL_NOTOPIC(client->nick, params[1])), true);
	if (params[2][0] == ':')
		params[2].erase(0, 1);
	for (std::vector<std::string>::iterator it = params.begin() + 2;it != params.end();it++)
	{
		token += (*it).c_str();
		if (it < params.end() - 1)
			token += " ";
	}
	c->topic = token;
	for (std::vector<Client *>::iterator it = c->users.begin();it != c->users.end();it++)
	{
		this->sendTo(client, *it, SEND_TOPIC(params[1], token));
	}
	return (true);
}

bool Server::kick_cmd(std::vector<std::string> params, Client *client)
{
	std::string token = "Default kick message";
	if (params.size() < 3 || params[1].empty() || params[2].empty())
		return (this->sendTo(client, client, ERR_NEEDMOREPARAMS(client->nick, "KICK")), false);
	Channel *c = channel_exist(params[1]);
	if (!c)
		return (this->sendTo(client, client, ERR_NOSUCHCHANNEL(client->nick, params[1])),false);
	if (!c->user_exist(client->nick))
		return (this->sendTo(client, client, ERR_NOTONCHANNEL(client->nick, params[1])),false);
	if (!c->user_exist(params[2]))
		return (this->sendTo(client, client, ERR_USERNOTINCHANNEL(client->nick, params[2], params[1])),false);
	if (!c->is_operator(client))
		return (this->sendTo(client, client, ERR_CHANOPRIVSNEEDED(client->nick, client->user, params[1])),false);
	if (params.size() >= 3 && params[3][0] == ':')
	{
		token.clear();
		params[3].erase(0, 1);
		for (std::vector<std::string>::iterator it = params.begin() + 3;it != params.end();it++)
		{
			token += (*it).c_str();
			if (it < params.end() - 1)
				token += " ";
		}
	}
	Client *cl = c->kick_client(params[2]);

	for (std::vector<Client *>::iterator it = c->users.begin();it != c->users.end();it++)
			this->sendTo(client, *it, KICK_MSG(client->nick, client->user, params[1], params[2], token));
	this->sendTo(client, cl, KICK_MSG(client->nick, client->user, params[1], params[2], token));
	return (true);
}

bool Server::invite_cmd(std::vector<std::string> params, Client *client)
{
	if (params.size() < 3 || params[1].empty() || params[2].empty())
		return (this->sendTo(client, client, ERR_NEEDMOREPARAMS(client->nick, "INVITE")), false);
	Channel *c = channel_exist(params[2]);
	if (!c)
		return (this->sendTo(client, client, ERR_NOSUCHCHANNEL(client->nick, params[2])),false);
	if (!c->user_exist(client->nick))
		return (this->sendTo(client, client, ERR_NOTONCHANNEL(client->nick, params[2])),false);
	if (c->user_exist(params[1]))
		return (this->sendTo(client, client, ERR_USERONCHANNEL(client->nick, params[1], params[2])),false);
	if (c->mode && !c->is_operator(client))
		return (this->sendTo(client, client, ERR_CHANOPRIVSNEEDED(client->nick, client->user, params[1])),false);
	Client *cl = nick_exist(params[1]);
	if (!cl)
		return (this->sendTo(client, client, ERR_NOSUCHNICK(client->nick, params[1])),false);
	c->invite.push_back(cl);
	this->sendTo(client, client, RPL_INVITING(client->nick, params[1], params[2]));
	this->sendTo(client, cl, SEND_INVITE(client->nick, client->user, params[1], params[2]));
	return (true);
}

bool Server::mode_cmd(std::vector<std::string> params, Client *client)
{
	if (params.size() < 2)
		return (this->sendTo(client, client, ERR_NEEDMOREPARAMS(client->nick, "MODE")), false);
	if (params[1][0] != '#')
		return (true);
	Channel *c = channel_exist(params[1]);
	if (!c)
		return (this->sendTo(client, client, ERR_NOSUCHCHANNEL(client->nick, params[1])), false);
	if (!c->user_exist(client->nick))
		return (this->sendTo(client, client, ERR_NOTONCHANNEL(client->nick, params[1])), false);
	if (c->mode && !c->is_operator(client))
		return (this->sendTo(client, client, ERR_CHANOPRIVSNEEDED(client->nick, client->user, params[1])), false);
	if (params.size() == 2)
	{
		std::string modes = "+";
		if (c->mode)
			modes.append("i");
		if (c->topic_mode)
			modes.append("t");
		if (!c->key.empty())
			modes.append("k");
		if (c->limit > 0)
			modes.append("l");
		return (this->sendTo(client, client, RPL_CHANNELMODEIS(client->nick, params[1], modes)), false);
	}
	bool minus = true;
	if (params[2][0] == '+')
		minus = false;
	for (std::string::iterator it = params[2].begin();it != params[2].end();it++)
	{
		if (*it != '-' && *it != '+' && *it != 'i' && *it != 't'
			&& *it != 'k' && *it != 'o' && *it != 'l')
			return (this->sendTo(client, client, ERR_UMODEUNKNOWNFLAG(client->nick, client->user)), false);
		switch (*it) {
			case 'i':
			{
				c->mode = minus ? 0 : 1;
				for (std::vector<Client *>::iterator it = c->users.begin();it != c->users.end();it++)
					this->sendTo(client, *it, RPL_MODE(client->nick, client->user, params[1], (minus ? "-i" : "+i")));
				break ;
			}
			case 't':
			{
				c->topic_mode = minus ? false : true;
				for (std::vector<Client *>::iterator it = c->users.begin();it != c->users.end();it++)
					this->sendTo(client, *it, RPL_MODE(client->nick, client->user, params[1], (minus ? "-t" : "+t")));
				break ;
			}
			case 'l':
			{
				c->limit = minus ? 0 : (params.size() > 3 ? std::atoi(params[3].c_str()) : 0);
				for (std::vector<Client *>::iterator it = c->users.begin();it != c->users.end();it++)
					this->sendTo(client, *it, RPL_MODE(client->nick, client->user, params[1], (minus ? "-l " + c->limit : "+l " + c->limit)));
				break ;
			}
			case 'k':
			{
				c->key = minus ? "" : (params.size() > 3 ? params[3] : "");
				for (std::vector<Client *>::iterator it = c->users.begin();it != c->users.end();it++)
					this->sendTo(client, *it, RPL_MODE(client->nick, client->user, params[1], (minus ? "-k " + c->key : "+k " + c->key)));
				break ;
			}
			case 'o':
			{
				// c->key = minus ? "" : (params.size() > 3 ? params[3] : "");
				// for (std::vector<Client *>::iterator it = c->users.begin();it != c->users.end();it++)
				// 	this->sendTo(client, *it, RPL_MODE(client->nick, client->user, params[1], (minus ? "-t" : "+t")));
				break ;
			}
			default:
				break;
		}
	}
	return (true);
}

bool Server::quit_cmd(std::vector<std::string> params, Client *client)
{
	client->is_used = false;
	client->registered = false;
	client->authenticated = false;
	for (std::set<Channel *>::iterator it = this->channels.begin();it != this->channels.end();)
	{
		std::vector<Client *>::iterator itt = std::find((*it)->users.begin(), (*it)->users.end(), client);
		if (itt != (*it)->users.end())
			(*it)->users.erase(itt);
		for (itt = (*it)->users.begin();itt != (*it)->users.end();itt++)
				this->sendTo(client, *itt, QUIT_MSG(client->nick, params[1]));
		itt = std::find((*it)->operators.begin(), (*it)->operators.end(), client);
		if (itt != (*it)->operators.end())
			(*it)->operators.erase(itt);
		if ((*it)->users.size() == 0)
		{
			delete *(it);
			this->channels.erase(it++);
		}
		else
			++it;
	}
	return (true);
}
