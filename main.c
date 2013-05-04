#include <stdlib.h>
#include <stdio.h>
#include "irc.h"

int main(int argc, char *argv[])
{
	char *server = "irc.speedrunslive.com";
	char *nick = "zidbot";
	char *pass = NULL;
	int port = 6667;
	IRC *s;

	s = irc_connect(server, nick, pass, port);
	if(!s)
	{
		fprintf(stderr, "Unable to connect.\n");
		return 0;
	}
	while(irc_do(s) != READY)
		;

	irc_join(s, "#rubicon");

	while(1)
		irc_do(s);

	return 0;
}
