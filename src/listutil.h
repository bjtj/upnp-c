#ifndef __LISTUTIL_H__
#define __LISTUTIL_H__

#include "public.h"

typedef int (*_cmp_cb)(void *, void *);
typedef void (*_free_cb)(void *);
typedef void (*_iter_cb)(void *, void *);

typedef struct _list_t
{
    void * data;
	struct _list_t * next;
} list_t;

extern list_t * list_create_node(void);
extern void list_free_node(list_t * node, _free_cb cb);
extern size_t list_size(list_t * lst);
extern list_t * list_find(list_t * lst, void * target, _cmp_cb cb);
extern list_t * list_tail(list_t * lst);
extern list_t * list_add(list_t * lst, void * data);
extern list_t * list_remove(list_t * lst, void * data, _free_cb cb);
extern void list_iter(list_t * lst, void * arg, _iter_cb cb);
extern list_t * list_clear(list_t * lst, _free_cb cb);

#endif
