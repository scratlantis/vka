#ifndef MIN_HEAP_H
#define MIN_HEAP_H

#ifndef MIN_HEAP_SIZE
#define MIN_HEAP_SIZE 4
#endif

#extension GL_EXT_control_flow_attributes : enable

struct MinHeap
{
	int id[MIN_HEAP_SIZE];
	float val[MIN_HEAP_SIZE];
	uint end;
};

void minHeapInit(inout MinHeap q)
{
	q.end = 0;
	[[unroll]]
	for (int i = 0; i < MIN_HEAP_SIZE; i++)
	{
		q.id[i] = -1;
	}
}

void minHeapInsert(inout MinHeap q, int idx, int id, float val)
{
	[[unroll]]
	for (int i = MIN_HEAP_SIZE - 2; i >= idx; i--)
	{
		q.id[i + 1] = q.id[i];
		q.val[i + 1] = q.val[i];
	}
	q.id[idx] = id;
	q.val[idx] = val;
	q.end++;
}

void minHeapAdd(inout MinHeap q, int id, float val)
{
	[[unroll]]
	for (int i = 0; i < MIN_HEAP_SIZE; i++)
	{
		if (i == q.end)
		{
			q.id[i] = id;
			q.val[i] = val;
			q.end++;
			break;
		}
		else if (val < q.val[i])
		{
			minHeapInsert(q, i, id, val);
		}
	}
}

#endif