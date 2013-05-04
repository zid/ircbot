#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"

#define INITIAL_BUCKETS 16

struct bucket {
	char *key;
	void *payload;
	struct bucket *next;
};

struct table {
	struct bucket **buckets;
	unsigned int nbuckets;
};

static unsigned long sdbm_hash(const char *key)
{
	unsigned long hash = 0;
	int c;

	while((c = *key++))
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash;
}

struct table *table_new(void)
{
	struct table *t;

	t = malloc(sizeof(struct table *));
	t->buckets = calloc(sizeof(struct bucket *), INITIAL_BUCKETS);
	t->nbuckets = INITIAL_BUCKETS;
	return t;
}

void bucket_free(struct bucket *b)
{
	free(b->key);
	free(b);
}

void table_del(struct table *t, const char *key)
{
	unsigned long hash;
	unsigned int nbucket;
	struct bucket **pb;

	hash = sdbm_hash(key);
	nbucket = hash % t->nbuckets;

	pb = &t->buckets[nbucket];

	while(1)
	{
		if(strcmp((*pb)->key, key) == 0)
		{
			struct bucket *b;
			b = *pb;
			*pb = (*pb)->next;
			free(b);
			break;
		}
		pb = &(*pb)->next;
	}
}

void table_add(struct table *t, const char *key, void *payload)
{
	unsigned long hash;
	unsigned int nbucket;
	struct bucket *b;

	hash = sdbm_hash(key);
	nbucket = hash % t->nbuckets;
	b = malloc(sizeof(struct bucket));
	b->payload = payload;
	b->key = strdup(key);
	b->next = t->buckets[nbucket];
	t->buckets[nbucket] = b;
}

void *table_get(struct table *t, const char *key)
{
	unsigned long hash;
	unsigned int bucket;
	struct bucket *b;

	if(!t)
		return NULL;

	hash = sdbm_hash(key);
	bucket = hash % t->nbuckets;

	for(b = t->buckets[bucket]; b; b = b->next)
	{
		if(strcmp(b->key, key) == 0)
			return b->payload;
	}

	return NULL;
}

