#include "../includes/Channel.hpp"

Channel::Channel(): topic(""), name("none"), mode(0), key(""), limit(2)
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
		{
			this->users.push_back(*it);
		}
		for (std::vector<Client *>::const_iterator it = src.operators.begin();it != src.operators.end();it++)
		{
			this->operators.push_back(*it);
		}
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

bool Channel::user_exist(Client *client)
{
	for (std::vector<Client *>::iterator it = this->users.begin();it != this->users.end();it++)
	{
		if ((*it)->nick == client->nick)
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

Channel::Channel(const std::string &str): topic(""), name(str), mode(0), key(""), limit(2)
{}

Channel::~Channel()
{
}
