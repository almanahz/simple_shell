#ifndef _STACKC_
#define _STACKC_

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stack.h"

// Function definitions for Stack

/*
Function: newStack()
Description:
Creates a new stack and returns it
*/
struct stack *newStack(void)
{
        myStack *stack = malloc(sizeof *stack); //allocate new stack
        if(stack)
        {
                stack->head = NULL; // if allocation successful set head to null
        }
        return stack; // return the stack
}

/*
Function: getString()
Description:
Used to set str attribute for node
*/
char* getString(char *str)
{
        char *tmp = malloc(strlen(str)+1);
        if(tmp)
        {
                strcpy(tmp,str); //copy str into tmp
        }
        return tmp;
}

/*
Function: push()
Description:
Pushes new node onto stack, and sets str attribute
*/
void push(myStack *stack, char* val)
{
        node *node = malloc(sizeof *node);

        if(node)
        {
                node->str = getString(val);
                node->next = stack->head;
                stack->head = node;
        }
        else
        {
                fprintf(stderr, "Error allocating to stack");
        }
}

/*
Function: pop()
Description:
Pops node off the stack and returns its str attribute
*/
char* pop(myStack *stack)
{
	char *str = malloc(sizeof(stack->head->str));
	
        if(stack && stack->head)
        {	
		strcpy(str, stack->head->str);
		node *tmp = stack->head;
		stack->head = stack->head->next;
		free(tmp->str);
        }
	return str;

}

#endif
