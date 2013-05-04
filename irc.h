#ifndef IRC_H
#define IRC_H

typedef struct irc IRC;
typedef struct irc_line line;

enum
{
	READY,
	CONNECTED,
	SENT_LOGIN,
	BUSY
};

IRC *irc_connect(const char *, const char *, const char *, int);
int irc_do(IRC *);
int irc_join(IRC *, const char *);
void irc_send(IRC *, const char *, ...);

struct irc_line
{
	char *who, *what, *at, *said;
};

#endif
