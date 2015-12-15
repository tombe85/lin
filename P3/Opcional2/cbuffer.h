#ifndef CBUFFER_H
#define CBUFFER_H


typedef struct
{
    void** data;			/* Vector of pointers to items in the buffer */
	unsigned int head;		/* Index of the first element // head in [0 .. max_size-1] */
	unsigned int size;		/* Current Buffer size // size in [0 .. max_size] */
	unsigned int max_size;  /* Buffer max capacity */
}
cbuffer_t;

/* Operations supported by cbuffer_t */
/* Creates a new cbuffer (takes care of allocating memory) */
cbuffer_t* create_cbuffer_t (unsigned int max_size);

/* Release memory from circular buffer  */
void destroy_cbuffer_t ( cbuffer_t* cbuffer );

/* Returns the number of elements in the buffer */
int size_cbuffer_t ( cbuffer_t* cbuffer );

/* Returns a non-zero value when buffer is full */
int is_full_cbuffer_t ( cbuffer_t* cbuffer );

/* Returns a non-zero value when buffer is empty */
int is_empty_cbuffer_t ( cbuffer_t* cbuffer );

/* Inserts an item at the end of the buffer */
void insert_cbuffer_t ( cbuffer_t* cbuffer, void* new_item );

/* Removes the first element in the buffer */
void remove_cbuffer_t ( cbuffer_t* cbuffer);

/* Returns the first element in the buffer */
void* head_cbuffer_t ( cbuffer_t* cbuffer );

#endif
