#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdarg.h>
#include <regex.h>
#include "irc.h"
#include "mod.h"

enum
{
	END_OF_MOTD = 376
};

struct irc
{
	int s, state;
	const char *nick, *pass;
};

static struct irc *irc_new(int s, const char *nick, const char *pass)
{
	struct irc *i;

	i = malloc(sizeof(struct irc));
	i->s = s;
	i->state = CONNECTED;
	i->nick = nick;
	i->pass = pass;

	return i;
}

IRC *irc_connect(const char *server, const char *nick, const char *pass,
	 int port)
{
	struct addrinfo hints, *si, *p;
	int r, s;
	char buf[32];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	sprintf(buf, "%d", port);

	r = getaddrinfo(server, buf, &hints, &si);
	if(r != 0)
		return NULL;

	for(p = si; p; p = p->ai_next)
	{
		s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(s == -1)
			continue;

		r = connect(s, p->ai_addr, p->ai_addrlen);
		if(r == -1)
		{
			close(s);
			continue;
		}
		break;
	}

	if(!p)
		return NULL;

	freeaddrinfo(si);

	return irc_new(s, nick, pass);
}

void irc_send(struct irc *i, const char *fmt, ...)
{
	char buf[512];
	va_list args;
	int n;

	va_start(args, fmt);
	n = vsnprintf(buf, 510, fmt, args);
	va_end(args);

	buf[n] = '\r';
	buf[n+1] = '\n';
	buf[n+2] = '\0';
	send(i->s, buf, n+2, 0);
}

void irc_get(struct irc *i, char *buf)
{
	int r;
	char *s = buf;
	int len = 0;
	*s = 0;
	while(1)
	{
		recv(i->s, s, 1, 0);
		len++;
		if(*s == '\n')
		{
			*s = 0;
			break;
		}

		if(*s != '\r')
			s++;
	}

	if(memcmp(buf, "PING :", 6) == 0)
	{
		buf[1] = 'O';
		irc_send(i, buf);
	}
}

static regex_t line_reg;
static int init = 0;

void irc_line_free(struct irc_line *il)
{
	free(il->who);
	free(il->what);
	free(il->at);
	free(il->said);
	free(il);
}

struct irc_line *parse(IRC *i, char *buf)
{
	int r;
	size_t nmatch = 5;
	regmatch_t pmatch[5];
	struct irc_line *il;

	if(!init)
	{
		r = regcomp(&line_reg, "^:([^ ]+) ([^ ]+) ([^ ]+) :(.*)$", REG_EXTENDED);
		if(r != 0)
			exit(0);
		init = 1;
	}

	r = regexec(&line_reg, buf, nmatch, pmatch, 0);
	if(r != 0)
	{
		printf("%s\n", buf);
		return NULL;
	}

	il = malloc(sizeof(struct irc_line));
	il->who  = malloc(pmatch[1].rm_eo - pmatch[1].rm_so + 1);
	il->what = malloc(pmatch[2].rm_eo - pmatch[2].rm_so + 1);
	il->at   = malloc(pmatch[3].rm_eo - pmatch[3].rm_so + 1);
	il->said = malloc(pmatch[4].rm_eo - pmatch[4].rm_so + 1);

	sprintf(il->who,  "%.*s", pmatch[1].rm_eo - pmatch[1].rm_so, &buf[pmatch[1].rm_so]);
	sprintf(il->what, "%.*s", pmatch[2].rm_eo - pmatch[2].rm_so, &buf[pmatch[2].rm_so]);
	sprintf(il->at,   "%.*s", pmatch[3].rm_eo - pmatch[3].rm_so, &buf[pmatch[3].rm_so]);
	sprintf(il->said, "%.*s", pmatch[4].rm_eo - pmatch[4].rm_so, &buf[pmatch[4].rm_so]);

	if(memcmp(il->said, "!load ", 6) == 0)
	{
		int r;
		r = module_load(il->said+6);
		switch(r)
		{
			case BAD_SONAME:
				irc_send(i, "PRIVMSG %s :Invalid characters in module name", il->at);
			break;
			case MODULE_LOADED:
				irc_send(i, "PRIVMSG %s :Module already loaded", il->at);
			break;
			case MODULE_AFK:
				irc_send(i, "PRIVMSG %s :Module not found", il->at);
			break;
			case MODULE_WTF:
				irc_send(i, "PRIVMSG %s :Module malformed", il->at);
			break;
		}
	}
	else if(memcmp(il->said, "!unload ", 8) == 0)
	{
		module_unload(il->said+8);
	}

	return il;
}

int irc_do(struct irc *i)
{
	char buf[512];
	struct irc_line *il;

	switch(i->state)
	{
		case CONNECTED:
		irc_send(i, "USER %s * * %s", i->nick, i->nick);
		if(i->pass)
			irc_send(i, "PASS %s", i->pass);
		irc_send(i, "NICK %s", i->nick);
		i->state = SENT_LOGIN;
		break;
		case SENT_LOGIN:
		irc_get(i, buf);
		il = parse(i, buf);
		if(!il)
			break;
		if(strcmp(il->what, "376") == 0)
			i->state = READY;
		irc_line_free(il);
		break;
		case READY:
		irc_get(i, buf);
		il = parse(i, buf);
		modules_handle(i, il);
		return READY;
		break;
	}
	return BUSY;
}

int irc_join(struct irc *i, const char *channel)
{
	irc_send(i, "JOIN %s", channel);

	return 0;
}
