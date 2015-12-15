#include <linux/vmalloc.h> /* vmalloc()/vfree()*/
#include <asm/string.h> /* memcpy() */
#include "cbuffer.h"

#ifndef NULL
#define NULL 0
#endif

/* Create cbuffer */
cbuffer_t* create_cbuffer_t (unsigned int max_size)
{
	cbuffer_t *cbuffer= (cbuffer_t *)vmalloc(sizeof(cbuffer_t));
	if (cbuffer == NULL)
	{
	    return NULL;
	}
	cbuffer->size=0;
	cbuffer->head=0;
    cbuffer->max_size=max_size;

    	/* Stores pointers to elements */
    	cbuffer->data=vmalloc(max_size*sizeof(void*));
	if ( cbuffer->data == NULL)
	{
		vfree(cbuffer->data);
	    return NULL;
	}
	return cbuffer;
}

/* Release memory from circular buffer  */
void destroy_cbuffer_t ( cbuffer_t* cbuffer )
{
    vfree(cbuffer->data);
    cbuffer->size=0;
    cbuffer->head=0;
    cbuffer->max_size=0;
    cbuffer->data=NULL;
    vfree(cbuffer);
}

/* Returns the number of elements in the buffer */
int size_cbuffer_t ( cbuffer_t* cbuffer )
{
	return cbuffer->size ;
}

/* Return a non-zero value when buffer is full */
int is_full_cbuffer_t ( cbuffer_t* cbuffer )
{
	return ( cbuffer->size == cbuffer->max_size ) ;
}

/* Return a non-zero value when buffer is empty */
int is_empty_cbuffer_t ( cbuffer_t* cbuffer )
{
	return ( cbuffer->size == 0 ) ;
}


/* Inserts an item at the end of the buffer */
void insert_cbuffer_t ( cbuffer_t* cbuffer, void* new_item )
{
	unsigned int pos=0;

/* The buffer is full */
	if ( cbuffer->size == cbuffer->max_size )
	{
		/* Overwriting head position */
		cbuffer->data[cbuffer->head]=new_item;
		/* Now head position must be the next one*/
		if ( cbuffer->size !=0 )
			cbuffer->head= ( cbuffer->head+1 ) % cbuffer->max_size;
		/* Size remains constant*/
	}
	else
	{
		if ( cbuffer->max_size!=0 )
			pos= ( cbuffer->head+cbuffer->size ) % cbuffer->max_size;
		cbuffer->data[pos]=new_item;
		cbuffer->size++;
	}

}

/* Remove first element in the buffer */
void remove_cbuffer_t ( cbuffer_t* cbuffer)
{

	if ( cbuffer->size !=0 )
	{
		cbuffer->head= ( cbuffer->head+1 ) % cbuffer->max_size;
		cbuffer->size--;
	}
}

/* Returns the first element in the buffer */
void* head_cbuffer_t ( cbuffer_t* cbuffer )
{
	if ( cbuffer->size !=0 )
		return cbuffer->data[cbuffer->head];
	else{
		return NULL;
	}
}



