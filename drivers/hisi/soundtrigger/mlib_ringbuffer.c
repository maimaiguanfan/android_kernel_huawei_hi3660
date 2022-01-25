

#include <linux/slab.h>
#include "mlib_ringbuffer.h"
#include "soundtrigger_log.h"




// Ring Buffer Structure
struct _RingBuffer {
   unsigned int size;
   unsigned int start;
   unsigned int count;
   unsigned int element_size;
   int			transFlag;
   char buffer[0];
};

#undef NULL
#define NULL ((void *)0)

#define   ELEMENT_SIZE	   320
#define   ELEMENT_CNT	   200
#define   HEAD_SIZE   20
#define   RINGBUFFER_SIZE	 ((ELEMENT_SIZE)*(ELEMENT_CNT))



RingBuffer *RingBuffer_Init(void * ringbufferAddr, int element_size, int element_cnt)
{
	RingBuffer *rb;
	int ringbuffer_size;
	rb = ringbufferAddr;
	ringbuffer_size = element_size * element_cnt;
	if (NULL == rb)
	{
		return NULL;
	}

	memset(rb,0X00,HEAD_SIZE+ringbuffer_size);

	rb->element_size = element_size;
	rb->size = element_cnt;
	rb->start = 0;
	rb->count = 0;
	rb->transFlag = 0;

	return rb;
}


unsigned int RingBuffer_GetCount(RingBuffer *rb)
{
	return rb->count;
}


int RingBuffer_IsEmpty(RingBuffer *rb)
{
	return (rb->count == 0);
}


int RingBuffer_IsFull(RingBuffer *rb)
{
	return (rb->count == rb->size);
}

int RingBuffer_GetTransFlag(RingBuffer *rb)
{
	return rb->transFlag;
}

int RingBuffer_SetTransFlag(RingBuffer *rb, int transFlag)
{
	rb->transFlag = transFlag;
	return 0;
}



int RingBuffer_Get(RingBuffer *rb, void *element)
{
	int retval;

	if (!RingBuffer_IsEmpty(rb))
	{
		memcpy(element, rb->buffer + rb->start * rb->element_size, rb->element_size);/*lint !e679*/
		rb->start = (rb->start + 1) % rb->size;
		--rb->count;
		retval = rb->element_size;
	}
	else
	{
		retval = 0;
	}

	return retval;
}


int RingBuffer_Put(RingBuffer *rb, const void *element)
{
	int end, retval;

	end = (rb->start + rb->count) % rb->size;
	memcpy(rb->buffer + end * rb->element_size, element, rb->element_size);/*lint !e679*/
	if (RingBuffer_IsFull(rb))
	{
		rb->start = (rb->start + 1) % rb->size; /* full, overwrite */
		retval = RING_BUFFER_OVERWRITE;
	}
	else
	{
		++rb->count;
		retval = RING_BUFFER_NORMAL;
	}

	return retval;
}


int RingBuffer_Compare(RingBuffer *rb, const void *element,int compareCount)
{
	int i = 0;
	int ret = 0;

	for(i = 0 ; i < compareCount ; i++)
	{
		if (!RingBuffer_IsEmpty(rb))
		{
			ret = memcmp((const char *)element + rb->element_size*i, rb->buffer + rb->start * rb->element_size, rb->element_size);/*lint !e679*/

			if(0 == ret)
			{
				rb->start = (rb->start + 1) % rb->size;
				--rb->count;
			}else{
				ret = 0;
			}

		}
		else
		{
			ret = -1;
		}
	}

	return 0;
}


int RingBuffer_Skip(RingBuffer *rb, unsigned int skip_cnt)
{
	int retval;

	if (RingBuffer_GetCount(rb) < skip_cnt)
	{
		retval = RING_BUFFER_NO_SKIP;
	}
	else
	{
		rb->start = (rb->start + skip_cnt) % rb->size;
		rb->count -= skip_cnt;
		retval = RING_BUFFER_NORMAL;
	}

	return retval;
}


int RingBuffer_Backspace(RingBuffer *rb, unsigned int skip_cnt)
{
	int retval;

	if (RingBuffer_GetCount(rb) < skip_cnt)
	{
		retval = RING_BUFFER_NO_SKIP;
	}
	else
	{
		rb->start = (rb->start + rb->count - skip_cnt) % rb->size;
		rb->count = skip_cnt;
		retval =  RING_BUFFER_NORMAL;
	}

	return retval;
}


void RingBuffer_DeInit(RingBuffer *rb)
{
	unsigned long st_ringbuff_size = 0;
	unsigned long buffer_size = 0;

	if (rb)
	{
		buffer_size = (unsigned long)rb->element_size * rb->size;
		st_ringbuff_size = (unsigned long)HEAD_SIZE + buffer_size;
		memset(rb, 0, st_ringbuff_size);
		rb = NULL;
	}
}

