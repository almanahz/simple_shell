#ifndef _STACKNODEH_
#define _STACKNODEH_

// Definition of stack node
// Holds a string and pointer to next node
typedef struct stackNode
{
	char* str;
	struct stackNode *next;
} node;


#endif
