#include <stdio.h>
#include <stdlib.h>

#include "queue_p.h"

static void insert_node(queue_p_t * queue, queue_p_node_t * p_node, int index);

queue_p_t * queue_p_init(uint32_t capacity, FREE_F customfree)
{
	queue_p_t * new_queue = calloc(1, sizeof(queue_p_t));

	if ((0 == capacity) ||
		(NULL == new_queue))
	{
		goto END;
	}

	new_queue->capacity = capacity;
	new_queue->currentsz = 0;
	new_queue->customfree = customfree;
	new_queue->arr = calloc(capacity, sizeof(* new_queue->arr));
	
	if (NULL == new_queue->arr)
	{
		free(new_queue);
		new_queue = NULL;
		goto END;
	}

END:
	return new_queue;
}

int queue_p_fullcheck(queue_p_t * queue)
{
	int result = 1;

	if (NULL == queue)
	{
		goto END;
	}

	result = (queue->currentsz >= queue->capacity) ? 0 : 1;

END:
	return result;
}

int queue_p_emptycheck(queue_p_t * queue)
{
	int result = 1;

	if (NULL == queue)
	{
		goto END;
	}

	result = (0 == queue->currentsz) ? 0 : 1;

END:
	return result;
}

int queue_p_enqueue(queue_p_t * queue, void * data, int priority)
{
	int SUCCESS = 1;
	int index = 0;
	int FIRST_ELEMENT = 0;

	if ((NULL == queue) ||
		(NULL == data) ||
		(0 == queue_p_fullcheck(queue)))
	{
		goto END;
	}

	queue_p_node_t * new_node = calloc(1, sizeof(queue_p_node_t));

	if (NULL == new_node)
	{
		goto END;
	}

	new_node->data = data;
	new_node->priority = priority;

	while (index < queue->currentsz)
	{
		if (queue->arr[index]->priority <= priority)
		{
			break;
		}

		index ++;
	}

	insert_node(queue, new_node, index);

	queue->currentsz++;
	SUCCESS = 0;
				
END:
	return SUCCESS;
}

queue_p_node_t * queue_p_dequeue(queue_p_t * queue)
{
	queue_p_node_t * popped_node = NULL;
	int index = 0;
	int FIRST_ELEMENT = 0;

	if ((NULL == queue) ||
		(0 == queue->currentsz))
	{
		goto END;
	}

	popped_node = queue->arr[FIRST_ELEMENT];

	for (; index + 1 < queue->currentsz; index++)
	{
		queue->arr[index] = queue->arr[index+1];
	}

	queue->currentsz--;

END:
	return popped_node;
}

queue_p_node_t * queue_p_peek(queue_p_t * queue)
{
	int FIRST_ELEMENT	= 0;
	queue_p_node_t * p_node = NULL;

	if ((NULL == queue) ||
		(0 == queue->currentsz))
	{
		goto END;
	}

	p_node = queue->arr[FIRST_ELEMENT];

END:
	return p_node;
}

int queue_p_clear(queue_p_t * queue)
{
	int SUCCESS = 1;
	int index = 0;

	if ((NULL == queue) ||
		(0 == queue->currentsz))
	{
		goto END;
	}

	for (; index < queue->currentsz; index++)
	{
		if (NULL != queue->customfree)
		{
			queue->customfree(queue->arr[index]->data);
		}

		free(queue->arr[index]);
		queue->arr[index] = NULL;
	}

	queue->currentsz = 0;
	SUCCESS = 0;

END:
	return SUCCESS;
}

int queue_p_destroy(queue_p_t ** queue)
{
	int SUCCESS = 1;

	if ((NULL == queue) ||
		(NULL == (* queue)))
	{
		goto END;
	}

	if (0 != (* queue)->currentsz)
	{
		queue_p_clear((* queue));
	}

	(* queue) = NULL;
	free(* queue);

	SUCCESS = 0;

END:
	return SUCCESS;
}

static void insert_node(queue_p_t * queue, queue_p_node_t * p_node, int index)
{
	if ((NULL == queue) ||
		(NULL == p_node))
	{
		goto END;
	}

	int track = queue->currentsz;

	while (track > index)
	{
		queue->arr[track] = queue->arr[track - 1];
		track--;
	}

	queue->arr[index] = p_node;

END:
	return;
}
