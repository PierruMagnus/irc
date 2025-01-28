#include "../includes/Client.hpp"

Client::Client(): is_used(false), registered(false), authenticated(false),
				  is_operator(false), quit(false), type(NC), nick(""), user("")
{}

Client &Client::operator=(const Client& src)
{
    if(this != &src)
	{
		this->type = src.type;
		this->authenticated = src.authenticated;
		this->client_fd = src.client_fd;
		this->is_operator = src.is_operator;
		this->is_used = src.is_used;
		this->nick = src.nick;
		this->quit = src.quit;
		this->realname = src.realname;
		this->registered = src.registered;
		this->send_buffer = src.send_buffer;
		this->sendto = src.sendto;
		this->src_ip = src.src_ip;
		this->src_port = src.src_port;
		this->user = src.user;
		std::cout << "Copy operator called" << std::endl;
	}
    return *this;
}

Client::~Client()
{
	this->send_buffer.clear();
}
