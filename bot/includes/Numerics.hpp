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
#define CHANGING_NICK(client, user, nick) ":" + client + "!" + user + "@localhost" + " NICK " + nick + "" RESET_COLOR "\r\n"
#define PONG(token) "PONG localhost " + token + "" RESET_COLOR "\r\n"
#define SEND_PRIVMSG(to, msg) "PRIVMSG " + to + " " + msg + "" RESET_COLOR "\r\n"
#define JOIN_CHANNEL(client, user, channel) ":" + client + "!" + user + "@localhost" + " JOIN " + channel + "" RESET_COLOR "\r\n"
#define SEND_TOPIC(channel, topic) "TOPIC " + channel + " :" + topic + "" RESET_COLOR "\r\n"
#define QUIT_MSG(client, msg) ":" + client + " QUIT " + msg + "" RESET_COLOR "\r\n"
#define KICK_MSG(client, user, channel, kick, msg) ":" + client + "!" + user + "@localhost" + " KICK " + channel + " " + kick + " " + msg + "" RESET_COLOR "\r\n"
#define SEND_INVITE(client, user, nick, channel) ":" + client + "!" + user + "@localhost" + " INVITE " + nick + " " + channel + "" RESET_COLOR "\r\n"


// RPL
#define RPL_WELCOME(client) ":localhost 001 " + client + " :" RPL_COLOR "Welcome to the KEK Network, " + client + "" RESET_COLOR "\r\n"
#define RPL_YOURHOST(client) ":localhost 002 " + client + " :" RPL_COLOR "Your host is KEKservername, running version 3.0" RESET_COLOR "\r\n"
#define RPL_CREATED(client, datetime) ":localhost 003 " + client + " :" RPL_COLOR "This server was created " + datetime + "" RESET_COLOR "\r\n"
#define RPL_MYINFO(client, version, usermodes, chanmodes)	":localhost 004 " + client + " :" RPL_COLOR "KEKservername " + version + " " + usermodes + " " + chanmodes + "" RESET_COLOR "\r\n"
#define RPL_YOUREOPER(client) ":localhost 381 " + client + " :" RPL_COLOR " You are now an IRC operator" RESET_COLOR "\r\n"
#define RPL_MOTDSTART(client)	":localhost 375 " + client + " :" RPL_COLOR "- KEKservername Message of the day - " RESET_COLOR "\r\n"
#define RPL_MOTD(client)	":localhost 372 " + client + " :" RPL_COLOR "KEK MOTD\n    OIIIIIIII\n    END MOTD" RESET_COLOR "\r\n"
#define RPL_ENDOFMOTD(client)	":localhost 376 " + client + " :" RPL_COLOR "End of /MOTD command." RESET_COLOR "\r\n"

#define RPL_NAMREPLY(client, channel, users) ":localhost 353 " + client + " = " + channel + " :" + users + "\r\n"
#define RPL_ENDOFNAMES(client, channel) ":localhost 366 " + client + " " + channel + " :" RPL_COLOR "End of /NAMES list" RESET_COLOR "\r\n"

#define RPL_TOPIC(client, channel, topic) ":localhost 332 " + client + " " + channel + " :" RPL_COLOR + topic + "" RESET_COLOR "\r\n"
#define RPL_NOTOPIC(client, channel) ":localhost 331 " + client + " " + channel + " :" RPL_COLOR "No topic is set" RESET_COLOR "\r\n"

#define RPL_INVITING(client, nick, channel) ":localhost 341 " + client + " " + nick + " " + channel + "" RESET_COLOR "\r\n"

#define RPL_CHANNELMODEIS(client, channel, mode) ":localhost 324 " + client + " " + channel + " " + mode + "" RESET_COLOR "\r\n"
#define RPL_MODE(client, user, channel, mode) ":" + client + "!" + user + "@localhost MODE " + channel + " " + mode + "" RESET_COLOR "\r\n"

#define attempt1 "\n\
  +---+\n\
  |   |\n\
      |\n\
      |\n\
      |\n\
      |\n\
=========\n"

#define attempt2 "\n\
  +---+\n\
  |   |\n\
  o   |\n\
      |\n\
      |\n\
      |\n\
=========\n"

#define attempt3 "\n\
  +---+\n\
  |   |\n\
  o   |\n\
  |   |\n\
      |\n\
      |\n\
=========\n"

#define attempt4 "\n\
  +---+\n\
  |   |\n\
  o   |\n\
 /|   |\n\
      |\n\
      |\n\
=========\n"

#define attempt5 "\n\
  +---+\n\
  |   |\n\
  o   |\n\
 /|\\ |\n\
      |\n\
      |\n\
=========\n"

#define attempt6 "\n\
  +---+\n\
  |   |\n\
  o   |\n\
 /|\\ |\n\
 /    |\n\
      |\n\
=========\n"

#define attempt7 "\n\
  +---+\n\
  |   |\n\
  o   |\n\
 /|\\ |\n\
 / \\ |\n\
      |\n\
=========\n"

#define gameover "\n\
      __ _  __ _ _ __ ___   ___    _____   _____ _ __ \n\
     / _` |/ _` | '_ ` _ \\ / _ \\  / _ \\ \\ / / _ | '__|\n\
    | (_| | (_| | | | | | |  __/ | (_) \\ V |  __| |   \n\
     \\__, |\\__,_|_| |_| |_|\\___|  \\___/ \\_/ \\___|_|   \n\
      __/ |                                           \n\
     |___/ \n"

