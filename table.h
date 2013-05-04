#ifndef HASH_H
#define HASH_H
typedef struct table Table;

Table *table_new(void);
void table_add(Table *, const char *, void *);
void *table_get(Table *, const char *);
void table_del(Table *, const char *);
#endif
