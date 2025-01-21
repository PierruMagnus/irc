#include "../includes/Channel.hpp"

Channel::Channel(): topic("kektopic"), name("none"), mode(0), key(""), limit(3)
{}

Channel::Channel(const std::string &str): topic("kektopic"), name(str), mode(0), key(""), limit(3)
{}

Channel::~Channel()
{
}
