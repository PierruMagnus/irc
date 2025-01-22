#include "../includes/Channel.hpp"

Channel::Channel(): topic("kektopic"), name("none"), mode(0), key(""), limit(2)
{}

Channel &Channel::operator=(const Channel& src)
{
    if(this != &src)
	{
		this->key = src.key;
		this->limit = src.limit;
		this->mode = src.mode;
		this->name = src.name;
		// this->name = src.name;
		// this->name += "_copy";
		this->topic = src.topic;
		// std::vector<Client *> v(src.users);
		// this->users = v;
		for (std::vector<Client *>::const_iterator it = src.users.begin();it != src.users.end();it++)
		{
			this->users.push_back(*it);
		}
		// std::copy ( src.users.begin(), src.users.end(), this->users.begin() );

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

Channel::Channel(const std::string &str): topic("kektopic"), name(str), mode(0), key(""), limit(2)
{}

Channel::~Channel()
{
}
