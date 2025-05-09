#include "stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Stack Stack;

Stack *createStack(int32_t capacity) {
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    if (!stack) {
        return NULL;
    }

    stack->top = NULL;
    stack->size = 0;
    stack->capacity = capacity;

    return stack;
}

void freeStack(Stack *stack, void (*freeData)(void*)) {
    if (!stack) {
        return;
    }

    struct stackNode *current = stack->top;
    struct stackNode *next;

    while (current) {
        next = current->next;
        if (freeData) {
            freeData(current->data);
        }
        free(current);
        current = next;
    }

    free(stack);
}

bool push(Stack *stack, void *data) {
    if (stack->capacity != -1 && stack->size >= stack->capacity) {
        return false;
    }

    struct stackNode *newNode = (struct stackNode *)malloc(sizeof(struct stackNode));
    if (!newNode) {
        return false;
    }

    newNode->data = data;
    newNode->next = stack->top;
    stack->top = newNode;
    stack->size++;

    return true;
}

void *pop(Stack *stack) {
    if (stack->size == 0) {
        return NULL;
    }

    struct stackNode *topNode = stack->top;
    void *data = topNode->data;

    stack->top = topNode->next;
    free(topNode);
    stack->size--;

    return data;
}

void *peek(Stack *stack) {
    if (stack->size == 0) {
        return NULL;
    }
    return stack->top->data;
}

bool isEmpty(Stack *stack) {
    return stack->size == 0;
}
