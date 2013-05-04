#ifndef COMMAND_H
#define COMMAND_H
#include "irc.h"
typedef void (Callback)(IRC *, line *);
typedef struct command Command;
struct command
{
	const char *name;
	Callback *c;
};
#endif
