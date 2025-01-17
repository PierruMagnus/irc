#include "../includes/Client.hpp"

Client::Client(): is_used(false), registered(false), authenticated(false), send_to_all(false),
				  is_operator(false), nick(""), user("")
{}

Client::~Client()
{
	this->send_buffer.clear();
}
