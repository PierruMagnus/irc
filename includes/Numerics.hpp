#pragma once

#include <iostream>

#define ERR_NEEDMOREPARAMS(client, cmd) ":localhost 461 " + client + " " + cmd + " :Not enough parameters\r\n"
#define ERR_PASSWDMISMATCH(client) ":localhost 464 " + client + " :Password incorrect\r\n"
#define ERR_ALREADYREGISTERED(client) ":localhost 462 " + client + " :You may not reregister\r\n"

#define ERR_NONICKNAMEGIVEN() ":localhost 431 * :No nickname given\r\n"
#define ERR_ERRONEUSNICKNAME() ":localhost 432 * :Erroneus nickname\r\n"
#define ERR_NICKNAMEINUSE(nick) ":localhost 433 * " + nick + " :Nickname is already in use\r\n"
#define CHANGING_NICK(client, nick) ":" + client + " NICK " + nick + "\r\n"

#define ERR_NOTREGISTERED(client) ":localhost 451 " + client + " :You have not registered\r\n"

#define RPL_WELCOME(client) ":localhost 001 " + client + " :Welcome to the KEK Network, " + client + "\r\n"
#define RPL_YOURHOST(client) ":localhost 002 " + client + " :Your host is KEKservername, running version 3.0\r\n"
#define RPL_CREATED(client, datetime) ":localhost 003 " + client + " :This server was created " + datetime + "\r\n"
#define RPL_YOUREOPER(client) ":localhost 381 " + client + " :You are now an IRC operator\r\n"

#define PONG(token) "PONG localhost " + token + "\r\n"

#define ERR_NOSUCHNICK(client, nick) ":localhost 401 "+ client + " " + nick + " :No such nick/channel\r\n"
#define ERR_CANNOTSENDTOCHAN(client, channel) ":localhost 404 "+ client + " " + channel + " :Cannot send to channel\r\n"
#define SEND_PRIVMSG(client, to, msg) ":" + client + " PRIVMSG " + to + " " + msg + "\r\n"

#define ERR_CHANNELISFULL(client, channel) ":localhost 471 "+ client + " " + channel + " :Cannot join channel (+l)\r\n"
#define ERR_BADCHANNELKEY(client, channel) ":localhost 475 "+ client + " " + channel + " :Cannot join channel (+k)\r\n"
#define JOIN_CHANNEL(client, channel) ":" + client + " JOIN " + channel + "\r\n"


#define SEND_TOPIC(channel, topic) "TOPIC " + channel + " :" + topic + "\r\n"
#define RPL_TOPIC(client, channel, topic) ":localhost 332 " + client + " " + channel + " :" + topic + "\r\n"
#define RPL_NOTOPIC(client, channel) ":localhost 331 " + client + " " + channel + " :No topic is set\r\n"
#define ERR_NOSUCHCHANNEL(client, channel) ":localhost 403 "+ client + " " + channel + " :No such channel\r\n"
#define ERR_NOTONCHANNEL(client, channel) ":localhost 442 "+ client + " " + channel + " :You're not on that channel\r\n"
#define ERR_CHANOPRIVSNEEDED(client, channel) ":localhost 482 "+ client + " " + channel + " :You're not channel operator\r\n"
