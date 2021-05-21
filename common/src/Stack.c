#include <stdlib.h>
#include <stdbool.h>


int intcmp(const void *a, const void *b) {
	return ( *(const int*)b - *(const int*)a );
}

/*
** A very simplified structure of a Stack of integers in C.
*/
typedef struct stack {
	size_t idx;
	int *data;   /* data[idx] = idx */
	bool *in_stack; /* in_stack[idx] = boolean */
} Stack;

void Stack_New(Stack *st, size_t size)
{
	if (st == NULL) { return; }
	st->idx = 0;
	st->data     = malloc(size* sizeof(*st->data));
	st->in_stack = calloc(size, sizeof(*st->in_stack));
}

void Stack_Push(Stack *st, int u)
{
	if (st == NULL) { return; }
	if (st->in_stack[u]) { return; }
	st->data[++st->idx] = u;
	st->in_stack[u] = true;
}

int Stack_Pop(Stack *st)
{
	int u;
	if (st == NULL) { return 0; }
	u = st->data[st->idx--];
	st->in_stack[u] = 0;
	return u;
}

void Stack_Sort(Stack *st) {
	if (st == NULL) { return; }
	qsort(st->data+1, st->idx, sizeof(int), intcmp);
}
int Stack_Size(Stack *st) { return st->idx; }
bool Stack_Contains(Stack *st, int u) { return st != NULL ? st->in_stack[u] : false; }
bool Stack_IsEmpty(Stack *st) { return st->idx == 0; }

void Stack_Reset(Stack *st) {
	memset(st->in_stack, 0, st->idx * sizeof(*st->in_stack));
	st->idx = 0;
}

void Stack_Destroy(Stack *st)
{
	if (st == NULL) { return; }
	free(st->data);     st->data = NULL;
	free(st->in_stack); st->in_stack = NULL;
}
