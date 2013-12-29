#ifndef MOD_H
#define MOD_H

typedef struct module Module;

enum
{
	BAD_SONAME = 1,
	MODULE_LOADED,
	MODULE_AFK,
	MODULE_WTF,
	MODULE_OK

};

int module_load(const char *);

#endif
