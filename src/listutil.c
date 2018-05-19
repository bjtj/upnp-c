#include "listutil.h"


list_t * list_create_node(void)
{
	list_t * lst = (list_t*)malloc(sizeof(list_t));
	memset(lst, 0, sizeof(list_t));
	return lst;
}

void list_free_node(list_t * node, _free_cb cb)
{
	if (cb) {
		cb(node->data);
	}
	free(node);
}

size_t list_size(list_t * lst)
{
	size_t size = 0;
	for (; lst; lst = lst->next) {
		size++;
	}
	return size;
}

list_t * list_find(list_t * lst, void * target, _cmp_cb cb)
{
	for (; lst; lst = lst->next) {
		if (cb(lst->data, target) == 0) {
			return lst;
		}
	}
	return NULL;
}

list_t * list_tail(list_t * lst)
{
	for (; lst && lst->next; lst = lst->next);
	return lst;
}

list_t * list_add(list_t * lst, void * data)
{
	list_t * tail;
	list_t * node = list_create_node();
	node->data = data;
	tail = list_tail(lst);
	if (tail == NULL) {
		return node;
	}
	tail->next = node;
	return lst;
}

list_t * list_remove(list_t * lst, void * data, _free_cb cb)
{
	list_t * ptr = lst;
	if (ptr == NULL) {
		return NULL;
	}
	if (ptr->data == data) {
		list_t * next = ptr->next;
		list_free_node(ptr, cb);
		return next;
	}
	for (; ptr && ptr->next; ptr = ptr->next) {
		if (ptr->next->data == data) {
			list_t * node = ptr->next;
			ptr->next = ptr->next->next;
			list_free_node(node, cb);
			break;
		}
	}
	return lst;
}

void list_iter(list_t * lst, void * arg, _iter_cb cb)
{
	if (cb == NULL) {
		return;
	}
	for (; lst; lst = lst->next) {
		cb(lst, arg);
	}
}

list_t * list_clear(list_t * lst, _free_cb cb)
{
	while (lst) {
		list_t * next = lst->next;
		list_free_node(lst, cb);
		lst = next;
	}
	return NULL;
}
