#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "irc.h"
#include "mod.h"
#include "table.h"

typedef void (Callback)(IRC *, line *);
typedef struct command Command;
typedef struct module Module;

struct command
{
	const char *name;
	Callback *c;
};

struct module
{
	void *dl;
	Command *command_list;
};

static Table *module_table;
static Table *command_table;

void module_unload(const char *soname)
{
	struct module *m;
	Command *c;

	m = table_get(module_table, soname);
	if(!m)
		return;

	for(c = m->command_list; c->name; c++)
	{
		table_del(command_table, c->name);
	}

	table_del(module_table, soname);

	dlclose(m->dl);
	free(m);
}

int module_load(const char *soname)
{
	char buf[128];
	struct module *t;
	void *dl;
	Command *commands;
	const char *s;

	for(s = soname; *s; s++)
		if(!isalnum(*s))
			return BAD_SONAME;

	if(module_table)
		if(table_get(module_table, soname))
			return MODULE_LOADED;

	snprintf(buf, 128, "./%s.so", soname);

	dl = dlopen(buf, RTLD_LAZY | RTLD_GLOBAL);
	if(!dl)
		return MODULE_AFK;
	commands = dlsym(dl, "commands");
	if(!commands)
		return MODULE_WTF;

	t = malloc(sizeof(struct module));

	t->dl   = dl;
	t->command_list = commands;

	/* Register all the commands */
	for(commands = t->command_list; commands->name; commands++)
	{
		if(!command_table)
			command_table = table_new();
		table_add(command_table, commands->name, commands->c);
	}

	/* Register the module */
	if(!module_table)
		module_table = table_new();

	table_add(module_table, soname, t);

	return 0;
}

void modules_handle(IRC *s, line *l)
{
	char com[32], *str;
	Callback *c;
	int i;

	if(!l || *l->said != '!')
		return;

	str = l->said+1;

	for(i = 0; i<32; i++)
	{
		int ch = *str++;
		if(!isalnum(ch))
			break;
		com[i] = ch;
	}
	if(i == 32)
		return;
	com[i] = 0;

	printf("Testing command '%s'\n", com);
	c = table_get(command_table, com);
	if(c)
		c(s, l);
}

