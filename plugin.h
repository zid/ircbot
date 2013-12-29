#ifndef PLUGIN_H
#define PLUGIN_H
#include "irc.h"
typedef void (Callback)(IRC *, line *);
typedef struct command Command;
struct command
{
	const char *name;
	Callback *c;
};
#endif
