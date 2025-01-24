#include "../includes/Channel.hpp"

Channel::Channel(): topic(""), name("none"), mode(1), key(""), limit(2)
{}

Channel &Channel::operator=(const Channel& src)
{
    if(this != &src)
	{
		this->key = src.key;
		this->limit = src.limit;
		this->mode = src.mode;
		this->name = src.name;
		this->topic = src.topic;
		for (std::vector<Client *>::const_iterator it = src.users.begin();it != src.users.end();it++)
			this->users.push_back(*it);
		for (std::vector<Client *>::const_iterator it = src.invite.begin();it != src.invite.end();it++)
			this->invite.push_back(*it);
		for (std::vector<Client *>::const_iterator it = src.operators.begin();it != src.operators.end();it++)
			this->operators.push_back(*it);
		this->operators = src.operators;
	}
    return *this;
}

bool Channel::is_user(Client *c)
{
	for (std::vector<Client *>::iterator it = this->users.begin();it != this->users.end();it++)
	{
		if ((*it)->nick == c->nick)
			return (true);
	}
	return (false);
}

bool Channel::user_exist(const std::string &client)
{
	for (std::vector<Client *>::iterator it = this->users.begin();it != this->users.end();it++)
	{
		if ((*it)->nick == client)
			return (true);
	}
	return (false);
}

bool Channel::is_operator(Client *client)
{
	for (std::vector<Client *>::iterator it = this->operators.begin();it != this->operators.end();it++)
	{
		if ((*it)->nick == client->nick)
			return (true);
	}
	return (false);
}

bool Channel::is_invited(Client *client)
{
	for (std::vector<Client *>::iterator it = this->invite.begin();it != this->invite.end();it++)
	{
		if ((*it) == client)
			return (true);
	}
	return (false);
}

void Channel::rm_invite(Client *client)
{
	std::vector<Client *>::iterator itt = std::find(this->invite.begin(), this->invite.end(), client);
	if (itt != this->invite.end())
		this->invite.erase(itt);
}

Client *Channel::kick_client(const std::string &user)
{
	Client *cl = NULL;
	for (std::vector<Client *>::iterator itt = this->users.begin(); itt != this->users.end();)
	{
		if ((*itt)->nick == user)
		{
			cl = *itt;
			this->users.erase(itt);
			break ;
		}
		else
			itt++;
	}
	for (std::vector<Client *>::iterator itt = this->operators.begin(); itt != this->operators.end();)
	{
		if ((*itt)->nick == user)
		{
			this->operators.erase(itt);
			break ;
		}
		else
			itt++;
	}
	return (cl);
}

Channel::Channel(const std::string &str): topic(""), name(str), mode(1), key(""), limit(2)
{}

Channel::~Channel()
{
	this->users.clear();
	this->invite.clear();
	this->operators.clear();
}
