#include "plugin.h"

void say(IRC *i, line *l)
{
	irc_send(i, "PRIVMSG %s :%s\n", l->at, l->said+5);
}

Command commands[2] =
{
	{"say", say},
	{0, 0}
};


