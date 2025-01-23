#include "../includes/Server.hpp"


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
	std::string msg = "";
	const std::string pre = "*";
	if (params.size() == 1 || params[1].empty())
		return (msg = ERR_NEEDMOREPARAMS(pre, "PASS"), client->sendto.insert(std::make_pair(client, msg)), false);
	if (params[1] != this->_password)
		return (msg = ERR_PASSWDMISMATCH(pre), client->sendto.insert(std::make_pair(client, msg)), false);
	if (client->registered)
		return (msg = ERR_ALREADYREGISTERED(pre), client->sendto.insert(std::make_pair(client, msg)), false);
	client->authenticated = true;
	return (true);
}

bool Server::nick_cmd(std::vector<std::string> params, Client *client)
{
	std::string msg;
	if (params.size() == 1 || params[1].empty())
		return (msg = ERR_NONICKNAMEGIVEN(), client->sendto.insert(std::make_pair(client, msg)), false);
	if (!is_valid_nick(params[1]))
		return (msg = ERR_ERRONEUSNICKNAME(), client->sendto.insert(std::make_pair(client, msg)), false);
	if (nick_exist(params[1]))
		return (msg = ERR_NICKNAMEINUSE(params[1]), client->sendto.insert(std::make_pair(client, msg)), false);
	if (client->registered)
	{
		debug("Changing nickname.", client, 0);
		msg = CHANGING_NICK(client->nick, params[1]);
	}
	client->nick = params[1];
	// client->sendto.insert(std::make_pair(client, msg));
	return (true);
}

bool Server::user_cmd(std::vector<std::string> params, Client *client)
{
	std::string msg = "";
	if (!client->authenticated)
		return (msg = ERR_NOTREGISTERED(client->nick), client->sendto.insert(std::make_pair(client, msg)), false);
	if (params.size() == 1 || params[1].empty())
		return (msg = ERR_NEEDMOREPARAMS(client->nick, "USER"), client->sendto.insert(std::make_pair(client, msg)), false);
	if (client->registered)
		return (msg = ERR_ALREADYREGISTERED(client->nick), client->sendto.insert(std::make_pair(client, msg)), false);
	client->user = params[1];
	client->realname = params[4];
	if (params.size() == 6)
		client->realname += params[5];
	msg = RPL_WELCOME(client->nick);
	msg += RPL_YOURHOST(client->nick);
	msg += RPL_CREATED(client->nick, getTime());
	client->registered = true;
	client->sendto.insert(std::make_pair(client, msg));
	std::cout << "USER: " << msg << std::endl;
	return (true);
}

bool Server::oper_cmd(std::vector<std::string> params, Client *client)
{
	std::string msg;
	if (params.size() < 3 || params[1].empty() || params[2].empty())
		return (msg = ERR_NEEDMOREPARAMS(client->nick, "OPER"), client->sendto.insert(std::make_pair(client, msg)), false);
	std::map<std::string, std::string>::iterator it = this->operators.find(params[1]);
	if (it == this->operators.end() || params[2] != it->second)
		return (msg = ERR_PASSWDMISMATCH(client->nick), client->sendto.insert(std::make_pair(client, msg)), false);
	client->is_operator = true;
	msg = RPL_YOUREOPER(client->nick);
	client->sendto.insert(std::make_pair(client, msg));
	return (true);
}

bool Server::ping_cmd(std::vector<std::string> params, Client *client)
{
	std::string msg;
	msg = PONG(params[1]);
	client->sendto.insert(std::make_pair(client, msg));
	std::cout << "PONG: " << msg << std::endl;
	// client->send_buffer += PONG(params[1]);
	return (true);
}

bool Server::privmsg_cmd(std::vector<std::string> params, Client *client)
{
	std::string msg = "";
	std::string token = "";
	if (params.size() < 3 || params[1].empty() || params[2].empty())
		return (msg = ERR_NEEDMOREPARAMS(client->nick, "PRIVMSG"), client->sendto.insert(std::make_pair(client, msg)), false);
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
			return (msg = ERR_NOSUCHNICK(client->nick, params[1]),
				client->sendto.insert(std::make_pair(client, msg)),
				false);
		msg = SEND_PRIVMSG(client->nick, params[1], token);
		client->sendto.insert(std::make_pair(c, msg));
		return (true);
	}
	else
	{
		Channel *cc = channel_exist(params[1]);
		if (!cc)
			return (msg = ERR_NOSUCHNICK(client->nick, params[1]),
				client->sendto.insert(std::make_pair(client, msg)),
				false);
		if (!cc->is_user(client))
			return (msg = ERR_CANNOTSENDTOCHAN(client->nick, params[1]),
				client->sendto.insert(std::make_pair(client, msg)),
				false);

		for (std::vector<Client *>::iterator it = cc->users.begin();it != cc->users.end();it++)
		{
			msg = SEND_PRIVMSG(client->nick, params[1], token);
			if (client->nick != (*it)->nick)
				client->sendto.insert(std::make_pair(*it, msg));
		}
		return (true);
	}
}

bool Server::join_cmd(std::vector<std::string> params, Client *client)
{
	std::string msg = "";
	bool new_user = false;
	std::string key = "";
	if (params.size() < 2 || params[1].empty())
		return (msg = ERR_NEEDMOREPARAMS(client->nick, "JOIN"), client->sendto.insert(std::make_pair(client, msg)), false);
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
		new_user = true;
	}
	if (c->users.size() == c->limit)
		return (msg = ERR_CHANNELISFULL(client->nick, c->name),
			client->sendto.insert(std::make_pair(client, msg)),
			false);
	if (key != c->key)
		return (msg = ERR_BADCHANNELKEY(client->nick, c->name),
			client->sendto.insert(std::make_pair(client, msg)),
			false);
	c->users.push_back(client);
	for (std::vector<Client *>::iterator it = c->users.begin();it != c->users.end();it++)
	{
		msg = JOIN_CHANNEL(client->nick, c->name);
		if (client->nick != (*it)->nick)
			client->sendto.insert(std::make_pair(*it, msg));
	}
	if (new_user)
		msg = RPL_NOTOPIC(client->nick, c->name);
	else
		msg = RPL_TOPIC(client->nick, c->name, c->topic);
	client->sendto.insert(std::make_pair(client, msg));
	return (true);
}

bool Server::topic_cmd(std::vector<std::string> params, Client *client)
{
	std::string msg = "";
	std::string token = "";
	if (params.size() < 2 || params[1].empty())
		return (msg = ERR_NEEDMOREPARAMS(client->nick, "TOPIC"), client->sendto.insert(std::make_pair(client, msg)), false);
	Channel *c = channel_exist(params[1]);
	if (!c)
		return (msg = ERR_NOSUCHCHANNEL(client->nick, params[1]), client->sendto.insert(std::make_pair(client, msg)),false);
	if (!c->user_exist(client))
		return (msg = ERR_NOTONCHANNEL(client->nick, params[1]), client->sendto.insert(std::make_pair(client, msg)),false);
	if (!c->is_operator(client))
		return (msg = ERR_CHANOPRIVSNEEDED(client->nick, params[1]), client->sendto.insert(std::make_pair(client, msg)),false);
	if (params.size() == 2 && !c->topic.empty())
		return (msg = RPL_TOPIC(client->nick, params[1], c->topic), client->sendto.insert(std::make_pair(client, msg)),true);
	else if (params.size() == 2 && c->topic.empty())
		return (msg = RPL_NOTOPIC(client->nick, params[1]), client->sendto.insert(std::make_pair(client, msg)),true);
	if (params[2][0] == ':')
		params[2].erase(0, 1);
	for (std::vector<std::string>::iterator it = params.begin() + 2;it != params.end();it++)
	{
		token += (*it).c_str();
		if (it < params.end() - 1)
			token += " ";
	}
	std::cout << "KEK: " << params[2] << std::endl;
	c->topic = msg;
	for (std::vector<Client *>::iterator it = c->users.begin();it != c->users.end();it++)
	{
		msg = SEND_TOPIC(params[1], token);
		client->sendto.insert(std::make_pair((*it), msg));
	}
	return (true);
}

bool Server::quit_cmd(std::vector<std::string> params, Client *client)
{
	(void)params;
	std::cout << "QUIIIIIIIT" << std::endl;
	client->is_used = false;
	client->registered = false;
	client->authenticated = false;
	return (true);
}
