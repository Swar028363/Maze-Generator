#ifndef STACK_H
#define STACK_H

#include <stdint.h>
#include <stdbool.h>

struct stackNode {
	void *data; // void Pointer to the data
	struct stackNode *next; // Pointer to the next node
};


struct Stack {
	struct stackNode *top; // Pointer to the top node
	uint32_t size;		   // Size of the stack
	int32_t capacity;	   // Maximum capacity of the stack. negative if no limit
};


struct Stack *createStack(int32_t capacity); // Create a stack with a given capacity

void freeStack(struct Stack *stack, void (*freeData)(void*)); // Destroy the stack and free memory

bool push(struct Stack *stack, void *data); // Push data onto the stack

void *pop(struct Stack *stack); // Pop data from the stack

void *peek(struct Stack *stack); // Peek at the top data without removing it

bool isEmpty(struct Stack *stack); // Check if the stack is empty

#endif // !STACK_H