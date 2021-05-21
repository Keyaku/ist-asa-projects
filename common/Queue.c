#include <stdlib.h>
#include <stdbool.h>


int intcmp(const void *a, const void *b) {
	return ( *(const int*)b - *(const int*)a );
}

/*
** A very simplified structure of a Queue of integers in C.
*/
typedef struct queue {
	int front, back;
	int *data;    /* data[idx] = int */
	bool *in_queue; /* in_queue[int] = boolean */
} Queue;

void Queue_New(Queue *q, size_t size, bool refcount) {
	q->data     = malloc((size+1) * sizeof(*q->data));
	q->in_queue = refcount ? calloc((size+1), sizeof(*q->in_queue)) : NULL;
	q->front = q->back = 0;
}
void Queue_Destroy(Queue *q) {
	free(q->data);     q->data = NULL;
	free(q->in_queue); q->in_queue = NULL;
}

bool Queue_InQueue(Queue *q, int u) { return q->in_queue ? q->in_queue[u] : false; }
void Queue_Push(Queue *q, int u) {
	if (Queue_InQueue(q, u)) { return; }
	q->data[q->back++] = u;
	if (q->in_queue) { q->in_queue[u] = true; }
}
int Queue_Pop(Queue *q) {
	int u = q->data[q->front++];
	if (q->in_queue) { q->in_queue[u] = false; }
	return u;
}

int Queue_Size(Queue *q) { return q->back - q->front; }
bool Queue_IsEmpty(Queue *q) { return q->front == q->back; }
void Queue_Reset(Queue *q) {
	if (q->in_queue) { memset(q->in_queue, false, (q->back) * sizeof(*q->in_queue)); }
	q->front = q->back = 0;
}
void Queue_Sort(Queue *q) {
	int size = Queue_size(q);
	qsort(q->data, size, sizeof(*q->data), intcmp);
}
