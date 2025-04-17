#pragma once

#include <iostream>

#define ERR_COLOR "\033[31m"
#define RPL_COLOR "\033[32m"
#define MSG_COLOR "\033[36m"
#define RESET_COLOR "\033[0m"

// ERROR
#define ERR_NEEDMOREPARAMS(client, cmd) ":localhost 461 " + client + " " + cmd + " :" ERR_COLOR "Not enough parameters" RESET_COLOR "\r\n"
#define ERR_PASSWDMISMATCH(client) ":localhost 464 " + client + " :" ERR_COLOR "Password incorrect" RESET_COLOR "\r\n"
#define ERR_ALREADYREGISTERED(client) ":localhost 462 " + client + " :" ERR_COLOR "You may not reregister" RESET_COLOR "\r\n"

#define ERR_NONICKNAMEGIVEN() ":localhost 431 * :" ERR_COLOR "No nickname given" RESET_COLOR "\r\n"
#define ERR_ERRONEUSNICKNAME() ":localhost 432 * :" ERR_COLOR "Erroneus nickname" RESET_COLOR "\r\n"
#define ERR_NICKNAMEINUSE(nick) ":localhost 433 * " + nick + " :" ERR_COLOR "Nickname is already in use" RESET_COLOR "\r\n"


#define ERR_NOTREGISTERED(client) ":localhost 451 " + client + " :" ERR_COLOR "You have not registered" RESET_COLOR "\r\n"

#define ERR_NOSUCHNICK(client, nick) ":localhost 401 "+ client + " " + nick + " :" ERR_COLOR "No such nick/channel" RESET_COLOR "\r\n"
#define ERR_CANNOTSENDTOCHAN(client, channel) ":localhost 404 "+ client + " " + channel + " :" ERR_COLOR "Cannot send to channel" RESET_COLOR "\r\n"

#define ERR_CHANNELISFULL(client, channel) ":localhost 471 "+ client + " " + channel + " :" ERR_COLOR "Cannot join channel (+l)" RESET_COLOR "\r\n"
#define ERR_BADCHANNELKEY(client, channel) ":localhost 475 "+ client + " " + channel + " :" ERR_COLOR "Cannot join channel (+k)" RESET_COLOR "\r\n"
#define ERR_INVITEONLYCHAN(client, channel) ":localhost 473 "+ client + " " + channel + " :" ERR_COLOR "Cannot join channel (+i)" RESET_COLOR "\r\n"

#define ERR_NOSUCHCHANNEL(client, channel) ":localhost 403 "+ client + " " + channel + " :" ERR_COLOR "No such channel" RESET_COLOR "\r\n"
#define ERR_NOTONCHANNEL(client, channel) ":localhost 442 "+ client + " " + channel + " :" ERR_COLOR "You're not on that channel" RESET_COLOR "\r\n"
#define ERR_USERONCHANNEL(client, nick, channel) ":localhost 443 "+ client + " " + nick + " " + channel + " :" ERR_COLOR "is already on channel" RESET_COLOR "\r\n"
#define ERR_CHANOPRIVSNEEDED(client, user, channel) ":localhost 482 "+ client + "!" + user + "@localhost " + channel + " :" ERR_COLOR "You're not channel operator" RESET_COLOR "\r\n"

#define ERR_USERNOTINCHANNEL(client, nick, channel) ":localhost 441 "+ client + " " + nick + " " + channel + " :" ERR_COLOR "They aren't on that channel" RESET_COLOR "\r\n"

#define ERR_UNKNOWNMODE(client, user, modechar) ":localhost 472 "+ client + "!" + user + "@localhost " + modechar + " :" ERR_COLOR "is unknown mode char to me" RESET_COLOR "\r\n"

#define ERR_BADCHANMASK(channel) ":localhost 476 "+ channel + " :" ERR_COLOR "Bad Channel Mask" RESET_COLOR "\r\n"

// MESSAGE REPLY
#define CHANGING_NICK(client, user, nick) ":" + client + "!" + user + "@localhost" + " NICK " + nick + "\r\n"
#define PONG(token) "PONG localhost " + token + "\r\n"
#define SEND_PRIVMSG(client, user, to, msg) ":" + client + "!" + user + "@localhost" + " PRIVMSG " + to + " " + msg + "\r\n"
#define JOIN_CHANNEL(client, user, channel) ":" + client + "!" + user + "@localhost" + " JOIN " + channel + "\r\n"
#define SEND_TOPIC(client, user, channel, topic) ":" + client + "!" + user + "@localhost" + " TOPIC " + channel + " :" + topic + "\r\n"
#define QUIT_MSG(client, user, msg) ":" + client + "!" + user + "@localhost" + " QUIT " + msg + "\r\n"
#define KICK_MSG(client, user, channel, kick, msg) ":" + client + "!" + user + "@localhost" + " KICK " + channel + " " + kick + " " + msg + "\r\n"
#define SEND_INVITE(client, user, nick, channel) ":" + client + "!" + user + "@localhost" + " INVITE " + nick + " " + channel + "\r\n"


// RPL
#define RPL_WELCOME(client, user) ":" + client + "!" + user + "@localhost 001 " + client + " :" RPL_COLOR "Welcome to the KEK Network, " + client + "" RESET_COLOR "\r\n"
#define RPL_YOURHOST(client, user) ":" + client + "!" + user + "@localhost 002 " + client + " :" RPL_COLOR "Your host is KEKservername, running version 3.0" RESET_COLOR "\r\n"
#define RPL_CREATED(client, user, datetime) ":" + client + "!" + user + "@localhost 003 " + client + " :" RPL_COLOR "This server was created " + datetime + "" RESET_COLOR "\r\n"
#define RPL_MYINFO(client, user, version, usermodes, chanmodes)	":" + client + "!" + user + "@localhost 004 " + client + " :" RPL_COLOR "KEKservername " + version + " " + usermodes + " " + chanmodes + "" RESET_COLOR "\r\n"
#define RPL_YOUREOPER(client, user) ":" + client + "!" + user + "@localhost 381 " + client + " :" RPL_COLOR " You are now an IRC operator" RESET_COLOR "\r\n"
#define RPL_MOTDSTART(client, user)	":" + client + "!" + user + "@localhost 375 " + client + " :" RPL_COLOR "- KEKservername Message of the day - " RESET_COLOR "\r\n"
#define RPL_MOTD(client, user)	":" + client + "!" + user + "@localhost 372 " + client + " :" RPL_COLOR "KEK MOTD\n    OIIIIIIII\n    END MOTD" RESET_COLOR "\r\n"
#define RPL_ENDOFMOTD(client, user)	":" + client + "!" + user + "@localhost 376 " + client + " :" RPL_COLOR "End of /MOTD command." RESET_COLOR "\r\n"

#define RPL_NAMREPLY(nick, user, channel, users) ":" + nick + "!" + user + "@localhost 353 " + nick + " = " + channel + " :" + users + "\r\n"
#define RPL_ENDOFNAMES(nick, user, channel) ":" + nick + "!" + user + "@localhost 366 " + nick + " " + channel + " :" RPL_COLOR "End of /NAMES list" RESET_COLOR "\r\n"

#define RPL_TOPIC(nick, user, channel, topic) ":" + nick + "!" + user + "@localhost 332 " + nick + " " + channel + " :" RPL_COLOR + topic + "" RESET_COLOR "\r\n"
#define RPL_NOTOPIC(nick, user, channel) ":" + nick + "!" + user + "@localhost 331 " + nick + " " + channel + " :" RPL_COLOR "No topic is set" RESET_COLOR "\r\n"

#define RPL_INVITING(client, user, nick, channel) ":" + client + "!" + user + "@localhost 341 " + client + " " + nick + " " + channel + "\r\n"

#define RPL_CHANNELMODEIS(client, user, channel, mode) ":" + client + "!" + user + "@localhost 324 " + client + " " + channel + " " + mode + "\r\n"
#define RPL_MODE(client, user, channel, mode) ":" + client + "!" + user + "@localhost MODE " + channel + " " + mode + "\r\n"
